#include <FS.h> //this needs to be first, or it all crashes and burns...
#ifdef ESP32
#include "SPIFFS.h"
#endif

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson

#ifdef ESP8266
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <AsyncJson.h>

#include "../../config.h"
#include "httpd.h"

#include "src/services/configStore.h"
#include "webSocket.h"
#include "../../sigksens.h"
#include "../sensors/sensorStorage.h"
#include "discovery.h"
#include "../services/configReset.h"



// Simple web page to view deltas
const char INDEX_PAGE[] PROGMEM = R"foo(
<html>
<head>
  <title>Deltas</title>
  <meta charset="utf-8">
  <script type="text/javascript">
    var WebSocket = WebSocket || MozWebSocket;
    var lastDelta = Date.now();
    var serverUrl = "ws://" + window.location.hostname + ":81";

    connection = new WebSocket(serverUrl);

    connection.onopen = function(evt) {
      console.log("Connected!");
      document.getElementById("box").innerHTML = "Connected!";
      document.getElementById("last").innerHTML = "Last: N/A";
    };

    connection.onmessage = function(evt) {
      var msg = JSON.parse(evt.data);
      document.getElementById("box").innerHTML = JSON.stringify(msg, null, 2);
      document.getElementById("last").innerHTML = "Last: " + ((Date.now() - lastDelta)/1000).toFixed(2) + " seconds";
      lastDelta = Date.now();
    };

    setInterval(function(){
      document.getElementById("age").innerHTML = "Age: " + ((Date.now() - lastDelta)/1000).toFixed(1) + " seconds";
    }, 50);
  </script>
</head>
<body>
  <h3>Last Delta</h3>
  <pre width="100%" height="50%" id="box">Not Connected yet</pre>
  <div id="last"></div>
  <div id="age"></div>
</body>
</html>
)foo";

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
HTTP
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

AsyncWebServer server(HTTP_PORT);

void createStaticFiles() {
  if (!SPIFFS.exists("/web/index.html")) {
    File f = SPIFFS.open("/web/index.html", "w");
    f.println("Hello, world!");
    f.close();
  }
}


void handleNotFound(AsyncWebServerRequest *request) {
  Serial.printf("NOT_FOUND: ");
  if(request->method() == HTTP_GET)
    Serial.printf("GET");
  else if(request->method() == HTTP_POST)
    Serial.printf("POST");
  else if(request->method() == HTTP_DELETE)
    Serial.printf("DELETE");
  else if(request->method() == HTTP_PUT)
    Serial.printf("PUT");
  else if(request->method() == HTTP_PATCH)
    Serial.printf("PATCH");
  else if(request->method() == HTTP_HEAD)
    Serial.printf("HEAD");
  else if(request->method() == HTTP_OPTIONS)
    Serial.printf("OPTIONS");
  else
    Serial.printf("UNKNOWN");
  Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for(i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}


void httpSetKeyValue(AsyncWebServerRequest *request) {
  if(!request->hasArg("key")) {
    request->send(400, "text/plain", "missing arg 'key'");
    return;
  }
  if(!request->hasArg("dataType")) {
    request->send(400, "text/plain", "missing arg 'dataType'");
    return;
  }
  if (request->arg("dataType") == "String") {
    configStore.putString(request->arg("key").c_str(), request->arg("value"));
  }
  
}

void httpReboot(AsyncWebServerRequest *request) {
  #ifdef ESP8266
    ESP.reset();
  #elif defined(ESP32)
    ESP.restart();
  #endif
}

void httpGetSensorInfo(AsyncWebServerRequest *request) {
  AsyncJsonResponse * response = new AsyncJsonResponse();
  JsonObject& json = response->getRoot();

  //Info
  json["hostname"] = configStore.getString("myHostname");

  //sigk
  json["signalKHost"] = configStore.getString("signalKServerHost");
  json["signalKPort"] = configStore.getUInt16("signalKServerPort");
  json["signalKPath"] = configStore.getString("signalKURLPath");
  json["signalKToken"] = configStore.getString("accessToken");

  json["websocketClientConnected"] = getWebsocketClientStatus();
  //Sensor types present
/*
  //Sensors
  JsonArray& sensorArr = json.createNestedArray("sensors");
  
  sensorStorageForEach([&](SensorInfo* si) {
    JsonObject& tmpSens = sensorArr.createNestedObject();
    si->toJson(tmpSens);
  });
*/
  response->setLength();
  request->send(response);
}

/*

void httpSetSensorAttr(AsyncWebServerRequest *request) {
  char pathStr[MAX_SIGNALK_PATH_LEN];
  char address[32];
  char attrName[32];
  bool found = false;

  Serial.println(F("Setting attributes for Sensor"));
  if(!request->hasArg("address")) {request->send(400, "text/plain", "missing arg 'address'"); return;}
  if(!request->hasArg("attrName")) {request->send(400, "text/plain", "missing arg 'attrName'"); return;}
  
  request->arg("address").toCharArray(address, 32);
  request->arg("attrName").toCharArray(attrName, 32);


  //for (int x=0;x<sensorStorage.size() ; x++) 
  sensorStorageForEach([&](SensorInfo* si) {
    if (strcmp(si->address, address) == 0) {
      // found our sensor, now find index
      for (int y=0; y<MAX_SENSOR_ATTRIBUTES; y++) {
        if (strcmp(si->attrName[y].c_str(), attrName) == 0) {
          //  found index!
          if(request->hasArg("path")) {
            request->arg("path").toCharArray(pathStr, MAX_SIGNALK_PATH_LEN);
            si->signalKPath[y] = pathStr;
          }
          
          if(request->hasArg("offset")) {
            si->offset[y] = request->arg("offset").toFloat();
          }
          if(request->hasArg("scale")) {
            si->scale[y] = request->arg("scale").toFloat();
          }          
          
          found = true;          
        }
      }     
    }
  });

  if (found) {
    app.onDelay(0, &saveConfig);
    request->send(200, "application/json", "{ \"success\": true }");
  } else {
    request->send(400, "application/json", "{ \"success\": false }");
  }
  
}
*/
/* no longer used, leaving it here as it may be useful?
void httpSetTimerDelay(AsyncWebServerRequest *request) {
  uint32_t newDelay = 0;
  char timer[15];
  bool ok = false;

  Serial.print(F("Setting Timer delay"));
  if(!request->hasArg("timer")) {request->send(400, "text/plain", "missing arg 'timer'"); return;}
  if(!request->hasArg("delay")) {request->send(400, "text/plain", "missing arg 'delay'"); return;}

  request->arg("timer").toCharArray(timer, 15);
  newDelay = request->arg("delay").toInt();

  if (newDelay > 5) { //ostimer min delay is 5ms
    if (strcmp(timer, "deltaDelay") == 0) {
      ok = true;
    }
  }

  if (ok) {
    app.onDelay(0, &saveConfig);
    request->send(200, "application/json", "{ \"success\": true }");
  } else {
    request->send(400, "application/json", "{ \"success\": false }");
  }
}
*/

void httpSignalKEndpoints(AsyncWebServerRequest *request) {
  IPAddress ip;  
  DynamicJsonBuffer jsonBuffer;
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  String wsURL;
  ip = WiFi.localIP();
 
  JsonObject& json = jsonBuffer.createObject();
  String ipString = String(ip[0]);
  for (uint8_t octet = 1; octet < 4; ++octet) {
    ipString += '.' + String(ip[octet]);
  }

  wsURL = "ws://" + ipString + ":81/";

  JsonObject& endpoints = json.createNestedObject("endpoints");
  JsonObject& v1 = endpoints.createNestedObject("v1");
  v1["version"] = "1.alpha1";
  v1["signalk-ws"] = wsURL;
  JsonObject& serverInfo = json.createNestedObject("server");
  serverInfo["id"] = "ESP-SigKSen";
  json.printTo(*response);
  request->send(response);
  
}

void httpResetConfig(AsyncWebServerRequest *request) {
  request->send(200, "application/json", "{ \"success\": true }");
  app.onDelay(0, &resetConfig);
}


void setupHTTP() {
  Serial.println(F("starting webserver"));

  createStaticFiles();

  server.onNotFound(handleNotFound);

  server.on("/",[](AsyncWebServerRequest *request ) {
      request->send_P(200, "text/html", INDEX_PAGE);
    });
  server.on("/index.html",[](AsyncWebServerRequest *request ) {
      request->send_P(200, "text/html", INDEX_PAGE);
    });


  server.on("/getSensorInfo", HTTP_GET, httpGetSensorInfo);

  //server.on("/getMPUCalibration", HTTP_GET, httpGetMPUCalibration);
  
  server.on("/set", HTTP_GET, httpSetKeyValue);
  server.on("/signalk", HTTP_GET, httpSignalKEndpoints);
  server.on("/signalk/", HTTP_GET, httpSignalKEndpoints);
  
  server.on("/resetConfig", HTTP_GET, httpResetConfig);
  server.on("/restart", HTTP_GET, httpReboot);

  //setup sensor callbacks
  sensorStorageForEach([&](SensorInfo* si) {
    si->setupWebServerHooks(server);
  });
  setupSSDPHttpCallback(server);

  server.begin();
}
