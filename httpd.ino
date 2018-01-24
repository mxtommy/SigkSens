

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
HTTP
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

void setupHTTP() {
  Serial.println("starting webserver");
  server.onNotFound(handleNotFound);
  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.serveStatic("/index.html", SPIFFS, "/web/index.html");
  server.on("/getSensorInfo", HTTP_GET, htmlGetSensorInfo);


  //server.on("/getMPUCalibration", HTTP_GET, htmlGetMPUCalibration);
  server.on("/setSensorPath", HTTP_GET, htmlSetSensorPath);
  server.on("/setTimerDelay", HTTP_GET, htmlSetTimerDelay);
  server.on("/setNewHostname", HTTP_GET, htmlNewHostname);

  server.on("/setDigitalMode", HTTP_GET, htmlSetDigitalMode);
  
  server.on("/setSignalKHost", HTTP_GET, htmlSetSignalKHost);
  server.on("/setSignalKPort", HTTP_GET, htmlSetSignalKPort);
  server.on("/setSignalKPath", HTTP_GET, htmlSetSignalKPath);

  
  server.on("/description.xml", HTTP_GET, [](){  SSDP.schema(server.client()); });

  server.on("/signalk", HTTP_GET, htmlSignalKEndpoints);
  server.on("/signalk/", HTTP_GET, htmlSignalKEndpoints);
  
  server.begin();
}


void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void htmlNewHostname() {
  if(!server.hasArg("hostname")) {server.send(500, "text/plain", "missing arg 'hostname'"); return;}
  server.arg("hostname").toCharArray(myHostname, 16);
  saveConfig();
  server.send(200, "application/json", "{ \"success\": true }");
  delay(1000);
  ESP.reset();
}

void htmlSetSignalKHost() {
  if(!server.hasArg("host")) {server.send(500, "text/plain", "missing arg 'host'"); return;}
  signalKHost = server.arg("host");
  saveConfig();
  server.send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}

void htmlSetSignalKPort() {
  if(!server.hasArg("port")) {server.send(500, "text/plain", "missing arg 'port'"); return;}
  signalKPort = server.arg("port").toInt();
  saveConfig();
  server.send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}

void htmlSetSignalKPath() {
  if(!server.hasArg("path")) {server.send(500, "text/plain", "missing arg 'path'"); return;}
  signalKPath = server.arg("path");
  saveConfig();
  server.send(200, "application/json", "{ \"success\": true }");
  restartWebSocketClient();
}

void htmlSetDigitalMode() {
  if(!server.hasArg("input")) {server.send(500, "text/plain", "missing arg 'input'"); return;}
  if(!server.hasArg("mode")) {server.send(500, "text/plain", "missing arg 'mode'"); return;}
  
  if (server.arg("input").toInt() == 1) {
    d1Mode = server.arg("mode").toInt();
  }
  if (server.arg("input").toInt() == 2) {
    d2Mode = server.arg("mode").toInt();
  }
    
  saveConfig();
  server.send(200, "application/json", "{ \"success\": true }");

  
}


void htmlGetSensorInfo() {
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  char response[2048];
  JsonObject& json = jsonBuffer.createObject();
  char strAddress[32];
  uint8_t numAttr;

  //Info
  json["hostname"] = myHostname;

  //sigk
  json["signalKHost"] = signalKHost;
  json["signalKPort"] = signalKPort;
  json["signalKPath"] = signalKPath;

  //Sensor types present
  json["sensorOneWire"] = sensorOneWirePresent;
  json["sensorSHT30"] = sensorSHT30Present;
  json["sensorMPU925X"] = sensorMPU925XPresent;

  //Digital
  json["d1Mode"] = d1Mode;
  json["d2Mode"] = d2Mode;
  
  //Timers
  JsonObject& timers = json.createNestedObject("timers");

  timers["oneWire"] = getOneWireReadDelay();
  timers["sht30"] = sensorSHTReadDelay;
  timers["mpu925x"] = updateMPUDelay;
  timers["digitalIn"] = updateDigitalInDelay;

  //Sensors
  JsonArray& sensorArr = json.createNestedArray("sensors");
  
  
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = sensorArr.createNestedObject();
    tmpSens.set<String>("address", tmpSensorInfo->address);
    tmpSens.set<String>("type", tmpSensorInfo->type);

    JsonArray& jsonAttr = tmpSens.createNestedArray("attr");
    for (int x=0;x<MAX_SENSOR_ATTRIBUTES; x++) {
      if (strcmp(tmpSensorInfo->attrName[x].c_str(), "") == 0) {
        break; // no more attrs
      }
      JsonObject& tmpAttr = jsonAttr.createNestedObject();
      tmpAttr.set<String>("attrName", tmpSensorInfo->attrName[x]);
      tmpAttr.set<String>("signalKPath", tmpSensorInfo->signalKPath[x] );
      tmpAttr["value"] = RawJson(tmpSensorInfo->valueJson[x].c_str());
    }
  }

  json.printTo(response);
  server.send ( 200, "application/json", response);
}


void htmlSetSensorPath() {
  
  SensorInfo *tmpSensorInfo;
  char pathStr[MAX_SIGNALK_PATH_LEN];
  char address[32];
  char attrName[32];
  int attrIndex;
  bool found = false;

  Serial.print("Setting path for Sensor");
  if(!server.hasArg("address")) {server.send(500, "text/plain", "missing arg 'address'"); return;}
  if(!server.hasArg("attrName")) {server.send(500, "text/plain", "missing arg 'attrName'"); return;}
  if(!server.hasArg("path")) {server.send(500, "text/plain", "missing arg 'path'"); return;}
  
  server.arg("address").toCharArray(address, 32);
  server.arg("path").toCharArray(pathStr, MAX_SIGNALK_PATH_LEN);
  server.arg("attrName").toCharArray(attrName, 32);


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
    server.send(200, "application/json", "{ \"success\": true }");
  } else {
    server.send(500, "application/json", "{ \"success\": false }");
  }
  
}


void htmlSetTimerDelay() {
  uint32_t newDelay = 0;
  char timer[10];
  bool ok = false;

  Serial.print("Setting Timer delay");
  if(!server.hasArg("timer")) {server.send(500, "text/plain", "missing arg 'timer'"); return;}
  if(!server.hasArg("delay")) {server.send(500, "text/plain", "missing arg 'delay'"); return;}

  server.arg("timer").toCharArray(timer, 10);
  newDelay = server.arg("delay").toInt();

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
    } else if (strcmp(timer, "digitalIn") == 0) {
      ok = true;
      setDigitalInUpdateDelay(newDelay);
    }
    
  }

  if (ok) {
    saveConfig();
    server.send(200, "application/json", "{ \"success\": true }");
  } else {
    server.send(500, "application/json", "{ \"success\": false }");
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
  server.send ( 200, "application/json", response);
  
}


void htmlReturnSignalKREST(){
  DynamicJsonBuffer jsonBuffer; 
  SensorInfo *thisSensorInfo;




  
}





