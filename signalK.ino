

void setupSignalK() {
  
}


void handleSignalK() {
  String deltaText;
  SensorInfo *thisSensorInfo;
  bool needToSend = false;
  DynamicJsonBuffer jsonBuffer; 

  //  build delta message
  JsonObject& delta = jsonBuffer.createObject();

  //updated array
  JsonArray& updatesArr = delta.createNestedArray("updates");

  //TODO initialize json only if updated needed.
  // source


  
  for (uint8_t i=0; i < sensorList.size(); i++) {

    thisSensorInfo = sensorList.get(i);
    if (thisSensorInfo->isUpdated) {
      needToSend = true;
      JsonObject& thisUpdate = updatesArr.createNestedObject();

      JsonObject& source = thisUpdate.createNestedObject("source");
      source["label"] = myHostname;
      source["src"] = thisSensorInfo->type;
      // values array
     
      JsonArray& values = thisUpdate.createNestedArray("values");

      for (int x=0;x<MAX_SENSOR_ATTRIBUTES; x++) {
        if (strcmp(thisSensorInfo->attrName[x].c_str(), "") == 0) {
          break; // if attr is empty, no more attr's for this sensor
        } 
        if (strcmp(thisSensorInfo->signalKPath[x].c_str(),  "") == 0) {
          continue; // no path set for this...
        }
        JsonObject& thisValue = values.createNestedObject();
        thisValue["path"] = thisSensorInfo->signalKPath[x].c_str();
        thisValue["value"] = RawJson(thisSensorInfo->valueJson[x].c_str());
        
      }
    

      
      //reset updated
      thisSensorInfo->isUpdated = false;
    }
  }

  if (needToSend) {
    delta.printTo(deltaText);
    //Serial.println(deltaText);
    webSocketServer.broadcastTXT(deltaText);
    if (websocketConnected) { // client
      webSocketClient.sendTXT(deltaText);
    }
  }

  
}

