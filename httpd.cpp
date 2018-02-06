extern "C" {
#include "user_interface.h"
}

#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <ESP8266SSDP.h>

#include "i2c.h"
#include "mpu.h"
#include "sht30.h"
#include "oneWire.h"
#ifdef ENABLE_DIGITALIN
  #include "digitalIn.h"
#endif
#include "httpd.h"
#include "FSConfig.h"
#include "webSocket.h"
#include "sigksens.h"
#include "config.h"


/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
HTTP
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

ESP8266WebServer httpServer(80);


void createStaticFiles() {
  if (!SPIFFS.exists("/web/index.html")) {
    File f = SPIFFS.open("/web/index.html", "w");
    f.println("Hello, world!");
    f.close();
  }
}


void handleNotFound() {
  httpServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void htmlNewHostname() {
  if(!httpServer.hasArg("hostname")) {httpServer.send(500, "text/plain", "missing arg 'hostname'"); return;}
  httpServer.arg("hostname").toCharArray(myHostname, 16);
  saveConfig();
  httpServer.send(200, "application/json", "{ \"success\": true }");
  delay(1000);
  ESP.reset();
}


void htmlSetSignalKHost() {
  if(!httpServer.hasArg("host")) {httpServer.send(500, "text/plain", "missing arg 'host'"); return;}
  signalKClientInfo.host = httpServer.arg("host");
  saveConfig();
  httpServer.send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}


void htmlSetSignalKPort() {
  if(!httpServer.hasArg("port")) {httpServer.send(500, "text/plain", "missing arg 'port'"); return;}
  signalKClientInfo.port = httpServer.arg("port").toInt();
  saveConfig();
  httpServer.send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}


void htmlSetSignalKPath() {
  if(!httpServer.hasArg("path")) {httpServer.send(500, "text/plain", "missing arg 'path'"); return;}
  signalKClientInfo.path = httpServer.arg("path");
  saveConfig();
  httpServer.send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}

#ifdef ENABLE_DIGITALIN
void htmlSetDigitalMode() {
  if(!httpServer.hasArg("input")) {httpServer.send(500, "text/plain", "missing arg 'input'"); return;}
  if(!httpServer.hasArg("mode")) {httpServer.send(500, "text/plain", "missing arg 'mode'"); return;}

  if (setDigitalMode(httpServer.arg("input").c_str(), httpServer.arg("mode").toInt())) {
    saveConfig();
    httpServer.send(200, "application/json", "{ \"success\": true }");    
  } else {
    httpServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
  }
}
#endif


void htmlGetSensorInfo() {
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  char response[2048];
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
  json["sensorOneWire"] = getSensorOneWirePresent();
  json["sensorSHT30"] = getSensorSHT30Present();
  json["sensorMPU925X"] = getSensorMPU925XPresent();

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

  timers["oneWire"] = getOneWireReadDelay();
  timers["sht30"] = getSensorSHTReadDelay();
  timers["mpu925x"] = getUpdateMPUDelay();
  #ifdef ENABLE_DIGITALIN
  timers["digitalIn"] = getUpdateDigitalInDelay();
  #endif

  //Sensors
  JsonArray& sensorArr = json.createNestedArray("sensors");
  
  
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = sensorArr.createNestedObject();
    
    tmpSens.set("address", tmpSensorInfo->address);
    tmpSens.set("type", (int)tmpSensorInfo->type);

    JsonArray& jsonAttr = tmpSens.createNestedArray("attr");
    for (int x=0;x<MAX_SENSOR_ATTRIBUTES; x++) {
      if (strcmp(tmpSensorInfo->attrName[x].c_str(), "") == 0) {
        break; // no more attrs
      }
      JsonObject& tmpAttr = jsonAttr.createNestedObject();
      tmpAttr.set("attrName", tmpSensorInfo->attrName[x]);
      tmpAttr.set("signalKPath", tmpSensorInfo->signalKPath[x] );
      tmpAttr.set("scale", tmpSensorInfo->scale[x] );
      tmpAttr.set("offset", tmpSensorInfo->offset[x] );
      tmpAttr["value"] = RawJson(tmpSensorInfo->valueJson[x].c_str());
    }
    //tmpSensorInfo->toJson(tmpSens);
  }

  json.prettyPrintTo(response);
  httpServer.send(200, "application/json", response);
}


void htmlSetSensorPath() {
  
  SensorInfo *tmpSensorInfo;
  char pathStr[MAX_SIGNALK_PATH_LEN];
  char address[32];
  char attrName[32];
  int attrIndex;
  bool found = false;

  Serial.print("Setting path for Sensor");
  if(!httpServer.hasArg("address")) {httpServer.send(500, "text/plain", "missing arg 'address'"); return;}
  if(!httpServer.hasArg("attrName")) {httpServer.send(500, "text/plain", "missing arg 'attrName'"); return;}
  if(!httpServer.hasArg("path")) {httpServer.send(500, "text/plain", "missing arg 'path'"); return;}
  
  httpServer.arg("address").toCharArray(address, 32);
  httpServer.arg("path").toCharArray(pathStr, MAX_SIGNALK_PATH_LEN);
  httpServer.arg("attrName").toCharArray(attrName, 32);


  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (strcmp(tmpSensorInfo->address, address) == 0) {
      // found our sensor, now find index
      for (int y=0; y<MAX_SENSOR_ATTRIBUTES; y++) {
        if (strcmp(tmpSensorInfo->attrName[y].c_str(), attrName) == 0) {
          //  found index!
          tmpSensorInfo->signalKPath[y] = pathStr;
          found = true;          
          break; //no need to check others if we found it
        }
      }
      
      if (found) { break; } // break out of outer for loop too if we found it
     
    }
  }

  if (found) {
    saveConfig();
    httpServer.send(200, "application/json", "{ \"success\": true }");
  } else {
    httpServer.send(500, "application/json", "{ \"success\": false }");
  }
  
}


void htmlSetTimerDelay() {
  uint32_t newDelay = 0;
  char timer[10];
  bool ok = false;

  Serial.print("Setting Timer delay");
  if(!httpServer.hasArg("timer")) {httpServer.send(500, "text/plain", "missing arg 'timer'"); return;}
  if(!httpServer.hasArg("delay")) {httpServer.send(500, "text/plain", "missing arg 'delay'"); return;}

  httpServer.arg("timer").toCharArray(timer, 10);
  newDelay = httpServer.arg("delay").toInt();

  if (newDelay > 5) { //ostimer min delay is 5ms
    if (strcmp(timer, "oneWire") == 0) {
      ok = true;
      setOneWireReadDelay(newDelay);
    } else if (strcmp(timer, "sht30") == 0) {
      ok = true;
      setSHTReadDelay(newDelay);
    } else if (strcmp(timer, "mpu925x") == 0) {
      ok = true;
      setMPUUpdateDelay(newDelay);
    } 
    #ifdef ENABLE_DIGITALIN
    else if (strcmp(timer, "digitalIn") == 0) {
      ok = true;
      setDigitalInUpdateDelay(newDelay);
    }
    #endif
    
  }

  if (ok) {
    saveConfig();
    httpServer.send(200, "application/json", "{ \"success\": true }");
  } else {
    httpServer.send(500, "application/json", "{ \"success\": false }");
  }
  
}


void htmlSignalKEndpoints() {
  IPAddress ip;  
  DynamicJsonBuffer jsonBuffer;
  char response[2048];
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
  json.printTo(response);
  httpServer.send ( 200, "application/json", response);
  
}


void htmlReturnSignalKREST() {
  DynamicJsonBuffer jsonBuffer; 
  SensorInfo *thisSensorInfo;  
}


void setupHTTP() {
  Serial.println("starting webserver");

  createStaticFiles();

  httpServer.onNotFound(handleNotFound);

  httpServer.serveStatic("/", SPIFFS, "/web/index.html");
  httpServer.serveStatic("/index.html", SPIFFS, "/web/index.html");
  httpServer.on("/getSensorInfo", HTTP_GET, htmlGetSensorInfo);

  //httpServer.on("/getMPUCalibration", HTTP_GET, htmlGetMPUCalibration);
  httpServer.on("/setSensorPath", HTTP_GET, htmlSetSensorPath);
  httpServer.on("/setTimerDelay", HTTP_GET, htmlSetTimerDelay);
  httpServer.on("/setNewHostname", HTTP_GET, htmlNewHostname);

  #ifdef ENABLE_DIGITALIN
  httpServer.on("/setDigitalMode", HTTP_GET, htmlSetDigitalMode);
  #endif
  
  httpServer.on("/setSignalKHost", HTTP_GET, htmlSetSignalKHost);
  httpServer.on("/setSignalKPort", HTTP_GET, htmlSetSignalKPort);
  httpServer.on("/setSignalKPath", HTTP_GET, htmlSetSignalKPath);

  
  httpServer.on("/description.xml", HTTP_GET, [](){  SSDP.schema(httpServer.client()); });

  httpServer.on("/signalk", HTTP_GET, htmlSignalKEndpoints);
  httpServer.on("/signalk/", HTTP_GET, htmlSignalKEndpoints);
  
  httpServer.begin();
}
