

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
  server.on("/setSensorPath", HTTP_GET, htmlSetSensorPath);
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
  uint8_t numAttr;

  //Sensors
  JsonArray& sensorArr = json.createNestedArray("sensors");
  
  
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = sensorArr.createNestedObject();
    tmpSens.set<String>("address", tmpSensorInfo->address);
    tmpSens.set<String>("type", tmpSensorInfo->type);

    // set number of attributes by sensor type
    if (strcmp(tmpSensorInfo->type, "oneWire") == 0) {
      numAttr = 1;
    } else if (strcmp(tmpSensorInfo->type, "sht30") == 0) {
      numAttr = 2;
    } else {
      //default to all...
      numAttr = MAX_SENSOR_ATTRIBUTES;
    }
    JsonArray& jsonAttr = tmpSens.createNestedArray("attr");
    for (int x=0;x<numAttr; x++) {
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

