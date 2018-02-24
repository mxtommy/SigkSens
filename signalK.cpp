extern "C" {
#include "user_interface.h"
}

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson

#include "sigksens.h"
#include "webSocket.h"
#include "signalK.h"

void setupSignalK() {
  
}


void handleSignalK() {
  bool needToSend = false;
  
  sensorStorageForEach([&](SensorInfo* si) {
    if (si->isUpdated) {
      needToSend = true;
    }
  });
  if (needToSend) {
    sendDelta();
  }
}


void receiveDelta(uint8_t * payload) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  root.prettyPrintTo(Serial);

}


void sendDelta() {

  String deltaText;
  SensorInfo *thisSensorInfo;

  DynamicJsonBuffer jsonBuffer; 

  //  build delta message
  JsonObject& delta = jsonBuffer.createObject();

  //updated array
  JsonArray& updatesArr = delta.createNestedArray("updates");
  
  sensorStorageForEach([&](SensorInfo* si) {
    if (si->isUpdated) {
      JsonObject& thisUpdate = updatesArr.createNestedObject();

      JsonObject& source = thisUpdate.createNestedObject("source");
      source["label"] = myHostname;
      source["src"] = (int)si->type;
      // values array
     
      JsonArray& values = thisUpdate.createNestedArray("values");

      for (int x=0;x<MAX_SENSOR_ATTRIBUTES; x++) {
        if (strcmp(si->attrName[x].c_str(), "") == 0) {
          break; // if attr is empty, no more attr's for this sensor
        } 
        if (strcmp(si->signalKPath[x].c_str(),  "") == 0) {
          continue; // no path set for this...
        }
        JsonObject& thisValue = values.createNestedObject();
        thisValue["path"] = si->signalKPath[x].c_str();
        thisValue["value"] = RawJson(si->valueJson[x].c_str());
        
      }
      //reset updated
      si->isUpdated = false;
    }
  });

  delta.printTo(deltaText);
  //Serial.println(deltaText);
  #ifdef ENABLE_WEBSOCKET_SERVER
  webSocketServer.broadcastTXT(deltaText);
  #endif
  if (signalKClientInfo.connected) { // client
    signalKClientInfo.client.sendTXT(deltaText);
  }
  
}

