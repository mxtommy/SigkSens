

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
  char response[2048];
  JsonObject& json = jsonBuffer.createObject();
  char strAddress[32];

  //Sensors
  JsonArray& sensorArr = json.createNestedArray("sensors");
  
  
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = sensorArr.createNestedObject();
    tmpSens.set<String>("address", tmpSensorInfo->address);
    tmpSens.set<String>("signalKPath", tmpSensorInfo->signalKPath );
    tmpSens["value"] = RawJson(tmpSensorInfo->valueJson);
    tmpSens.set<String>("type", tmpSensorInfo->type);
  }

  json.printTo(response);
  server.send ( 200, "application/json", response);
}


void htmlSet1WPath() {
  
  SensorInfo *tmpSensorInfo;
  char pathStr[MAX_SIGNALK_PATH_LEN];
  char address[32];
  bool found = false;

  Serial.print("Setting path for 1W Sensor: ");
  if(!server.hasArg("address")) {server.send(500, "text/plain", "missing arg 'address'"); return;}
  if(!server.hasArg("path")) {server.send(500, "text/plain", "missing arg 'path'"); return;}
  
  server.arg("address").toCharArray(address, 32);
  server.arg("path").toCharArray(pathStr, MAX_SIGNALK_PATH_LEN);


  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (strcmp(tmpSensorInfo->address, address) == 0) {
      strcpy(tmpSensorInfo->signalKPath, pathStr);
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

