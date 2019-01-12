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
#include "../../sigksens.h"

#include "httpd.h"

#include "../services/configStore.h"
#include "../services/configReset.h"
#include "../components/componentSensor.h"

#include "webSocket.h"
#include "discovery.h"


// Simple web page to view deltas
const char INDEX_PAGE[] PROGMEM = R"foo(
<html><head><title>Deltas</title><meta charset="utf-8"><script type="text/javascript">var WebSocket=WebSocket||MozWebSocket;var lastDelta=Date.now();var serverWsUrl="ws://"+window.location.hostname+":81";var serverUrl="http://"+window.location.hostname;var paths={};var pathAge={};connection=new WebSocket(serverWsUrl);connection.onopen=function(evt){console.log("Connected!");document.getElementById("box").innerHTML="Status: Connected!"};connection.onmessage=function(evt){var msg=JSON.parse(evt.data);msg.updates[0].values.forEach(function(pathData){paths[pathData.path]=pathData.value;pathAge[pathData.path]=Date.now()});updateValues()
lastDelta=Date.now()};function updateValues(){var newTbody="";for(var path in paths){newTbody=newTbody+"<tr><td>"+path+"</td><td>"+paths[path]+"</td><td>"+((Date.now()-pathAge[path])/1000).toFixed(1)+"</td></tr>"}
document.getElementById("deltaValues").innerHTML=newTbody}
setInterval(function(){document.getElementById("age").innerHTML="Time since last delta: "+((Date.now()-lastDelta)/1000).toFixed(1)+" seconds";updateValues()},50);function getConfig(){fetch(serverUrl+"/getConfig").then((resp)=>resp.json()).then(function(data){var newTbody="";Object.keys(data).sort().forEach(function(key,idx){newTbody=newTbody+"<tr><td>"+key+"</td><td>";switch(data[key].dataType){case "string":newTbody=newTbody+"<input id=\""+key+"\" type='text' value=\""+data[key].value+"\">";break;case "boolean":newTbody=newTbody+"<select id=\""+key+"\"><option value='true'";if(data[key].value){newTbody=newTbody+" SELECTED"}
newTbody=newTbody+">True</option><option value='false'";if(!data[key].value){newTbody=newTbody+" SELECTED"}
newTbody=newTbody+">False</Option></select>";break;case "int8":newTbody=newTbody+"<input id=\""+key+"\" type='number' min=-127 max=127 value=\""+data[key].value+"\">";break;case "uint8":newTbody=newTbody+"<input id=\""+key+"\" type='number' min=0 max=255 value=\""+data[key].value+"\">";break;case "int16":newTbody=newTbody+"<input id=\""+key+"\" type='number' min=-32767 max=32767 value=\""+data[key].value+"\">";break;case "uint16":newTbody=newTbody+"<input id=\""+key+"\" type='number' min=0 max=65535 value=\""+data[key].value+"\">";break;case "int32":case "uint32":newTbody=newTbody+"<input id=\""+key+"\" type='number'value=\""+data[key].value+"\">";break}
newTbody=newTbody+"</td><td><button id=\"button"+key+"\" onclick=\"postConfig('"+key+"', '"+data[key].dataType+"')\">Set</button></td></tr>"});document.getElementById("formTable").innerHTML=newTbody}).catch(function(error){console.log(error);alert(error)})}
function postConfig(configKey,configDataType){document.getElementById("button"+configKey).innerHTML="Saving";let newValue=document.getElementById(configKey).value;let data="key="+encodeURIComponent(configKey)+"&dataType="+encodeURIComponent(configDataType)+"&value="+encodeURIComponent(newValue);let fetchData={method:'POST',body:data,headers:{'Content-type':'application/x-www-form-urlencoded;charset=UTF-8'}};fetch(serverUrl+"/set",fetchData).then((resp)=>resp.json()).then(function(data){document.getElementById("button"+configKey).innerHTML="Saved!";console.log(data)}).catch(function(error){document.getElementById("button"+configKey).innerHTML="Error!";console.log(error);alert(error)})}
function getURL(url,buttonId,reloadConfig=!1){fetch(serverUrl+url).then((resp)=>resp.json()).then(function(data){document.getElementById(buttonId).innerHTML="Done!";if(reloadConfig){setTimeout(function(){getConfig()},1000)}
console.log(data)}).catch(function(error){document.getElementById(buttonId).innerHTML="Error!";console.log(error);alert(error)})}
getConfig();</script><style>html{font-family:Helvetica;display:inline-block;margin:0px auto;text-align:center}
h1{color:#0F3376;padding:2vh}
p{font-size:1.5rem}
table{margin-left:auto;margin-right:auto}
th{background-color:rgb(158,233,247)}
th,td{border-bottom:1px solid #ddd}
form{margin-block-end:0px}</style></head><body><h1>SigKSens</h1><h2>Current Values</h2><table><thead><tr><th>Path</th><th>Value</th><th>Age</th></tr></thead><tbody id="deltaValues"></tbody></table><div id="box">Status:Not Connected yet</div><div id="age"></div><h2>Actions</h2><div><button id="buttonReboot" onclick="getURL('/restart', 'buttonReboot')">Reboot</button><button id="buttonReset" onclick="getURL('/resetConfig', 'buttonReset')">Reset all Configuration</button><button id="buttonDiscover" onclick="getURL('/rediscover', 'buttonDiscover', true)">Rediscover server</button></div><h2>Configuration</h2><table><thead><tr><th>Config</th><th>Value</th><th></th></tr></thead><tbody id="formTable"></tbody></table></body></html>
)foo";

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
HTTP
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

AsyncWebServer server(HTTP_PORT);

/* No static files for the moment
void createStaticFiles() {
  if (!SPIFFS.exists("/web/index.html")) {
    File f = SPIFFS.open("/web/index.html", "w");
    f.println(F("Hello, world!"));
    f.close();
  }
} */


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
  else if(request->method() == HTTP_OPTIONS) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{ \"success\": true }");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response); // when posting needed for CORS pre-flight
  }
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

void httpGetConfig(AsyncWebServerRequest *request) {
  String fileName = "/globalConfig.json";
  if(request->hasArg("component")) {
    fileName = "/" + request->arg("component") + ".json";
  }
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, fileName, "application/json");
  request->send(response);
}

void httpSetKeyValue(AsyncWebServerRequest *request) {
  ConfigStore* componentConfig = &configStore; // default to global configStore
  if(!request->hasArg("key")) {
    request->send(400, "application/json", "{ \"success\": false, \"message\": \"missing arg 'key'\" }");    return;
  }
  if(!request->hasArg("dataType")) {
    request->send(400, "application/json", "{ \"success\": false, \"message\": \"missing arg 'dataType'\" }");
    return;
  }

  if(!request->hasArg("component")) {
    ComponentSensor* component = getComponent(request->arg("component"));
    if (component == nullptr) {
      request->send(400, "application/json", "{ \"success\": false, \"message\": \"did not find component\" }");
      return;
    }
    componentConfig = &(*component).config;
  }

  if (request->arg("dataType") == "string") {
    componentConfig->putString(request->arg("key").c_str(), request->arg("value"));
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "boolean") {
    bool newValue = false;
    if (request->arg("value") == "true") {
      newValue = true;
    }
    componentConfig->putBool(request->arg("key").c_str(), newValue);
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "int8") {
    componentConfig->putInt8(request->arg("key").c_str(), request->arg("value").toInt());
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "uint8") {
    componentConfig->putUInt8(request->arg("key").c_str(), request->arg("value").toInt());
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "int16") {
    componentConfig->putInt16(request->arg("key").c_str(), request->arg("value").toInt());
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "uint16") {
    componentConfig->putUInt16(request->arg("key").c_str(), request->arg("value").toInt());
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "int32") {
    componentConfig->putInt32(request->arg("key").c_str(), request->arg("value").toInt());
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  } else if (request->arg("dataType") == "uint32") {
    componentConfig->putUInt32(request->arg("key").c_str(), request->arg("value").toInt());
    request->send(200, "application/json", "{ \"success\": true }");
    return;
  }


  request->send(400, "application/json", "{ \"success\": false }");
}

void httpReboot(AsyncWebServerRequest *request) {
  request->send(400, "application/json", "{ \"success\": true }");
  delay(1000); //allow time for response to be sent
  #ifdef ESP8266
    ESP.reset();
  #elif defined(ESP32)
    ESP.restart();
  #endif
}

void httpGetSensorInfo(AsyncWebServerRequest *request) {
  AsyncJsonResponse * response = new AsyncJsonResponse();
  JsonObject& json = response->getRoot();

  json["websocketClientConnected"] = getWebsocketClientStatus();

  //Components types present
  JsonArray& componentsArr = json.createNestedArray("components");
  
  forEachComponent([&](ComponentSensor* component) {
    componentsArr.add(component->componentName);
  });

  response->setLength();
  request->send(response);
}


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

void httpRediscover(AsyncWebServerRequest *request) {
  String success = "false";
  if (getMDNSService()) {
    success = "true";
    restartWebSocketClient();
  }
  request->send(200, "application/json", "{ \"success\": " + success + " }");
}


void setupHTTP() {
  Serial.println(F("starting webserver"));

  server.onNotFound(handleNotFound);

  server.on("/",[](AsyncWebServerRequest *request ) {
      request->send_P(200, "text/html", INDEX_PAGE);
    });
  server.on("/index.html",[](AsyncWebServerRequest *request ) {
      request->send_P(200, "text/html", INDEX_PAGE);
    });


  server.on("/getSensorInfo", HTTP_GET, httpGetSensorInfo);

  server.on("/getConfig", HTTP_GET, httpGetConfig);
  server.on("/set", HTTP_POST, httpSetKeyValue);

  
  server.on("/resetConfig", HTTP_GET, httpResetConfig);
  server.on("/restart", HTTP_GET, httpReboot);
  server.on("/rediscover", HTTP_GET, httpRediscover);

  server.on("/signalk", HTTP_GET, httpSignalKEndpoints);
  server.on("/signalk/", HTTP_GET, httpSignalKEndpoints);

  //setup sensor callbacks
  /*
  sensorStorageForEach([&](SensorInfo* si) {
    si->setupWebServerHooks(server);
  });
  */
  setupSSDPHttpCallback(server);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); //CORSS...
  server.begin();
}
