extern "C" {
#include "user_interface.h"
}

#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266SSDP.h>
#include <StreamString.h>

#include "config.h"

#ifdef ENABLE_I2C
  #include "i2c.h"
#endif
#ifdef ENABLE_MPU
  #include "mpu9250.h"
#endif
#ifdef ENABLE_SHT30
  #include "sht30.h"
#endif
#ifdef ENABLE_ADS1115
  #include "ads1115.h"
#endif
#ifdef ENABLE_ONEWIRE
  #include "oneWire.h"
#endif
#ifdef ENABLE_DIGITALIN
  #include "digitalIn.h"
#endif
#include "httpd.h"
#include "FSConfig.h"
#include "webSocket.h"
#include "sigksens.h"

// SSDP related stuff

//SSDP properties
const char * modelName = "WifiSensorNode";
const char * modelNumber = "12345";

static const char* ssdpTemplate =
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u/</URLBase>"
    "<device>"
      "<deviceType>upnp:rootdevice</deviceType>"
      "<friendlyName>%s</friendlyName>"
      "<presentationURL>index.html</presentationURL>"
      "<serialNumber>%u</serialNumber>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>http://www.espressif.com</modelURL>"
      "<manufacturer>Espressif Systems</manufacturer>"
      "<manufacturerURL>http://www.espressif.com</manufacturerURL>"
      "<UDN>uuid:38323636-4558-4dda-9188-cda0e6%02x%02x%02x</UDN>"
    "</device>"
  "</root>\r\n"
  "\r\n";

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
HTTP
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

AsyncWebServer server(80);


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


void httpNewHostname(AsyncWebServerRequest *request) {
  if(!request->hasArg("hostname")) {
    request->send(400, "text/plain", "missing arg 'hostname'");
    return;
  }
  request->arg("hostname").toCharArray(myHostname, 16);
  saveConfig();
  request->send(200, "application/json", "{ \"success\": true }");
  delay(1000);
  ESP.reset();
}


void httpSetSignalKHost(AsyncWebServerRequest *request) {
  if(!request->hasArg("host")) {
    request->send(400, "text/plain", "missing arg 'host'"); 
    return;
  }
  signalKClientInfo.host = request->arg("host");
  saveConfig();
  request->send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}


void httpSetSignalKPort(AsyncWebServerRequest *request) {
  if(!request->hasArg("port")) {
    request->send(500, "text/plain", "missing arg 'port'");
    return;
  }
  signalKClientInfo.port = request->arg("port").toInt();
  saveConfig();
  request->send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}


void httpSetSignalKPath(AsyncWebServerRequest *request) {
  if(!request->hasArg("path")) {request->send(500, "text/plain", "missing arg 'path'"); return;}
  signalKClientInfo.path = request->arg("path");
  saveConfig();
  request->send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}

#ifdef ENABLE_DIGITALIN
void httpSetDigitalMode(AsyncWebServerRequest *request) {
  if(!request->hasArg("input")) {request->send(500, "text/plain", "missing arg 'input'"); return;}
  if(!request->hasArg("mode")) {request->send(500, "text/plain", "missing arg 'mode'"); return;}

  if (setDigitalMode(request->arg("input").c_str(), request->arg("mode").toInt())) {
    saveConfig();
    request->send(200, "application/json", "{ \"success\": true }");    
  } else {
    request->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
  }
}
#endif

#ifdef ENABLE_MPU
void httpMpuCalAccelGyro(AsyncWebServerRequest *request) {
  runAccelGyroCal();
  request->send(200, "application/json", "{ \"success\": true }");
}

void httpMpuCalMagStart(AsyncWebServerRequest *request) {
  runMagCalStart();
  request->send(200, "application/json", "{ \"success\": true }");
}

void httpMpuCalMagStop(AsyncWebServerRequest *request) {
  runMagCalStop();
  request->send(200, "application/json", "{ \"success\": true }");
}

#endif

void httpGetSensorInfo(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  JsonObject& json = jsonBuffer.createObject();
  char strAddress[32];
  char tmpPinStr[10];
  uint8_t numAttr;

  //Info
  json["hostname"] = myHostname;

  //sigk
  json["signalKHost"] = signalKClientInfo.host;
  json["signalKPort"] = signalKClientInfo.port;
  json["signalKPath"] = signalKClientInfo.path;

  //Sensor types present
  #ifdef ENABLE_ONEWIRE
    json["sensorOneWire"] = getSensorOneWirePresent();
  #endif
  #ifdef ENABLE_SHT30
  json["sensorSHT30"] = getSensorSHT30Present();
  #endif
  #ifdef ENABLE_MPU
  json["sensorMPU925X"] = getSensorMPU925XPresent();
  #endif

  #ifdef ENABLE_DIGITALIN
  //Digital
  JsonObject& digitalPins = json.createNestedObject("digitalPins");
  for (uint8_t x=0; x < NUMBER_DIGITAL_INPUT; x++) {
    getDigitalPinName(x, tmpPinStr); // sets tmpPinStr to the name of pin (array of char)
    digitalPins[tmpPinStr] = getDigitalMode(x);
  }
  #endif
  
  //Timers
  JsonObject& timers = json.createNestedObject("timers");

  #ifdef ENABLE_ONEWIRE
  timers["oneWire"] = getOneWireReadDelay();
  #endif
  #ifdef ENABLE_SHT30
  timers["sht30"] = getSensorSHTReadDelay();
  #endif
  #ifdef ENABLE_MPU
  timers["mpu925x"] = getUpdateMPUDelay();
  #endif
  #ifdef ENABLE_ADS1115
  timers["ads1115Read"] = getReadADSDelay();
  timers["ads1115Update"] = getUpdateADSDelay();
  #endif
  #ifdef ENABLE_DIGITALIN
  timers["digitalIn"] = getUpdateDigitalInDelay();
  #endif

  //Sensors
  JsonArray& sensorArr = json.createNestedArray("sensors");
  
  
  //for (uint8_t i=0; i < sensorStorage.size(); i++) {
  sensorStorageForEach([&](SensorInfo* si) {
    JsonObject& tmpSens = sensorArr.createNestedObject();
    
    tmpSens.set("address", si->address);
    tmpSens.set("type", (int)si->type);

    JsonArray& jsonAttr = tmpSens.createNestedArray("attr");
    for (int x=0;x<MAX_SENSOR_ATTRIBUTES; x++) {
      if (strcmp(si->attrName[x].c_str(), "") == 0) {
        break; // no more attrs
      }
      JsonObject& tmpAttr = jsonAttr.createNestedObject();
      tmpAttr.set("attrName", si->attrName[x]);
      tmpAttr.set("signalKPath", si->signalKPath[x] );
      tmpAttr.set("scale", si->scale[x] );
      tmpAttr.set("offset", si->offset[x] );
      tmpAttr["value"] = RawJson(si->valueJson[x].c_str());
    }
    //tmpSensorInfo->toJson(tmpSens);
  });

  json.prettyPrintTo(*response);
  request->send(response);
}


void httpSetSensorAttr(AsyncWebServerRequest *request) {
  
  SensorInfo *tmpSensorInfo;
  char pathStr[MAX_SIGNALK_PATH_LEN];
  char address[32];
  char attrName[32];
  bool found = false;

  Serial.print("Setting attributes for Sensor");
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
    saveConfig();
    request->send(200, "application/json", "{ \"success\": true }");
  } else {
    request->send(400, "application/json", "{ \"success\": false }");
  }
  
}


void httpSetTimerDelay(AsyncWebServerRequest *request) {
  uint32_t newDelay = 0;
  char timer[15];
  bool ok = false;

  Serial.print("Setting Timer delay");
  if(!request->hasArg("timer")) {request->send(400, "text/plain", "missing arg 'timer'"); return;}
  if(!request->hasArg("delay")) {request->send(400, "text/plain", "missing arg 'delay'"); return;}

  request->arg("timer").toCharArray(timer, 15);
  newDelay = request->arg("delay").toInt();

  if (newDelay > 5) { //ostimer min delay is 5ms
    if (strcmp(timer, "oneWire") == 0) {
    #ifdef ENABLE_ONEWIRE
      ok = true;
      setOneWireReadDelay(newDelay);
    #endif
    }
    #ifdef ENABLE_SHT30
    else if (strcmp(timer, "sht30") == 0) {
      ok = true;
      setSHTReadDelay(newDelay);
    }
    #endif
    #ifdef ENABLE_MPU
    else if (strcmp(timer, "mpu925x") == 0) {
      ok = true;
      setMPUUpdateDelay(newDelay);
    }
    #endif
    #ifdef ENABLE_ADS1115
    else if (strcmp(timer, "ads1115Read") == 0) {
      ok = true;
      setADSReadDelay(newDelay);
    }
    else if (strcmp(timer, "ads1115Update") == 0) {
      ok = true;
      setADSUpdateDelay(newDelay);
    }        
    #endif
    #ifdef ENABLE_DIGITALIN
    else if (strcmp(timer, "digitalIn") == 0) {
      ok = true;
      setDigitalInUpdateDelay(newDelay);
    }
    #endif
    
  }

  if (ok) {
    saveConfig();
    request->send(200, "application/json", "{ \"success\": true }");
  } else {
    request->send(400, "application/json", "{ \"success\": false }");
  }
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


void setupHTTP() {
  Serial.println("starting webserver");

  createStaticFiles();

  server.onNotFound(handleNotFound);

  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.serveStatic("/index.html", SPIFFS, "/web/index.html");
  server.on("/getSensorInfo", HTTP_GET, httpGetSensorInfo);

  //server.on("/getMPUCalibration", HTTP_GET, httpGetMPUCalibration);
  server.on("/setSensorPath", HTTP_GET, httpSetSensorAttr); //path for legacy
  server.on("/setSensorAttr", HTTP_GET, httpSetSensorAttr);

  server.on("/setTimerDelay", HTTP_GET, httpSetTimerDelay);
  server.on("/setNewHostname", HTTP_GET, httpNewHostname);

  #ifdef ENABLE_DIGITALIN
  server.on("/setDigitalMode", HTTP_GET, httpSetDigitalMode);
  #endif
  
  #ifdef ENABLE_MPU
  server.on("/mpuCalAccelGyro", HTTP_GET, httpMpuCalAccelGyro);
  server.on("/mpuCalMagStart", HTTP_GET, httpMpuCalMagStart);
  server.on("/mpuCalMagStop", HTTP_GET, httpMpuCalMagStop);
  #endif


  server.on("/setSignalKHost", HTTP_GET, httpSetSignalKHost);
  server.on("/setSignalKPort", HTTP_GET, httpSetSignalKPort);
  server.on("/setSignalKPath", HTTP_GET, httpSetSignalKPath);

  server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest *request){
      StreamString output;
      if(output.reserve(1024)){
        IPAddress ip = WiFi.localIP();
        uint32_t chipId = ESP.getChipId();
        output.printf(ssdpTemplate,
          ip[0], ip[1], ip[2], ip[3],
          myHostname,
          chipId,
          modelName,
          modelNumber,
          (uint8_t) ((chipId >> 16) & 0xff),
          (uint8_t) ((chipId >>  8) & 0xff),
          (uint8_t)   chipId        & 0xff
        );
        request->send(200, "text/xml", (String)output);
      } else {
        request->send(500);
      }
  });

  server.on("/signalk", HTTP_GET, httpSignalKEndpoints);
  server.on("/signalk/", HTTP_GET, httpSignalKEndpoints);
  
  server.begin();
}
