

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
  server.on("/getSensors", HTTP_GET, htmlGetSensors);
  server.on("/set1wSensorPath", HTTP_GET, htmlSet1WPath);
  server.on("/description.xml", HTTP_GET, [](){  SSDP.schema(server.client()); });
  server.begin();
}


void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void htmlGetSensors() {
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  char response[1024];
  JsonObject& json = jsonBuffer.createObject();
  char strAddress[32];
  //sensors
  JsonArray& oneWSensors = json.createNestedArray("1wSensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = oneWSensors.createNestedObject();
    sprintf(strAddress, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
        tmpSensorInfo->sensorAddress[0], 
        tmpSensorInfo->sensorAddress[1], 
        tmpSensorInfo->sensorAddress[2],
        tmpSensorInfo->sensorAddress[3], 
        tmpSensorInfo->sensorAddress[4], 
        tmpSensorInfo->sensorAddress[5], 
        tmpSensorInfo->sensorAddress[6], 
        tmpSensorInfo->sensorAddress[7]  );
    tmpSens.set<String>("address", strAddress);
    tmpSens.set<String>("signalKPath", tmpSensorInfo->signalKPath );
    tmpSens.set<float>("tempK", tmpSensorInfo->tempK);
  }
  json.printTo(response);
  server.send ( 200, "application/json", response);
}


void htmlSet1WPath() {
  
  DeviceAddress address;
  SensorInfo *tmpSensorInfo;
  char pathStr[MAX_SIGNALK_PATH_LEN];
  bool found = false;

  Serial.print("Setting path for 1W Sensor: ");
  if(!server.hasArg("address")) {server.send(500, "text/plain", "missing arg 'address'"); return;}
  if(!server.hasArg("path")) {server.send(500, "text/plain", "missing arg 'path'"); return;}
  
  String addressStr = server.arg("address");
  server.arg("path").toCharArray(pathStr, MAX_SIGNALK_PATH_LEN);

  parseBytes(addressStr.c_str(), ':', address,  8, 16);

  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (memcmp(tmpSensorInfo->sensorAddress, address, sizeof(address)) == 0) {
      memcpy(tmpSensorInfo->signalKPath, pathStr, MAX_SIGNALK_PATH_LEN);
      found = true;
      break; //no need to check others if we found it
    }
  }

  if (found) {
    saveConfig();
    server.send(200, "application/json", "{ \"success\": true }");
  } else {
    server.send(500, "application/json", "{ \"success\": false }");
  }
  
}

