#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#include <ArduinoJson.h> 

#include "sigksens.h"
#include "src/services/configStore.h"
#include "src/services/ledBlinker.h"

#include "signalK.h"

#include "../net/webSocket.h"
#include <map>



SignalK signalK;

void setupSignalK() {
  app.onRepeat(10, handleSignalK); // check for deltas to send every 10ms
}

void handleSignalK() {
  signalK.handle(); // not sure onRepeat will work directly with the singlaK Object instance...
}


void SignalK::addValue(String path, String jsonValue) {
  _mapValues[path] = jsonValue;
}


void SignalK::handle() {
  bool needToSend = false;
  if (!_mapValues.empty) {
    String deltaText;
    DynamicJsonBuffer jsonBuffer; 

    //  build delta message
    JsonObject& delta = jsonBuffer.createObject();

    //updated array
    JsonArray& updatesArr = delta.createNestedArray("updates");
    JsonObject& thisUpdate = updatesArr.createNestedObject();
    JsonObject& source = thisUpdate.createNestedObject("source");
    source["label"] = configStore.getString("myHostname");
    JsonArray& values = thisUpdate.createNestedArray("values");

    for (const auto& kv : _mapValues) {
      JsonObject& thisValue = values.createNestedObject();
      thisValue["path"] = kv.first.c_str();
      thisValue["value"] = RawJson(kv.second.c_str());
    }

    // Send Deltas
    delta.printTo(deltaText);
    #ifdef ENABLE_SERIAL_DELTA
    Serial.println(deltaText);
    #endif
    #ifdef ENABLE_WEBSOCKET_SERVER
    webSocketServer.broadcastTXT(deltaText);
    #endif
    if (signalKClientInfo.connected) { // client
      signalKClientInfo.client.sendTXT(deltaText);
      ledBlinker.flip();
    }
  }
}


void receiveDelta(uint8_t * payload) {
  /*
  DynamicJsonBuffer jsonBuffer;
  char tempStr[255];
  bool tempBool;

  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println(F("parseObject() failed"));
    return;
  }
  //root.prettyPrintTo(Serial);

  if (root.containsKey("put")) {
    for (uint8_t i=0; i < root["put"].size(); i++) {
      strcpy(tempStr, root["put"][i]["path"]);
      sensorStorage[(int)SensorType::digitalOut].forEach([&](SensorInfo* si) {
        if (strcmp(si->signalKPath[0].c_str(), tempStr) == 0) {
           //Serial.println(si->address);
           //Serial.println(si->signalKPath[0]);

          if (root["put"][i]["value"].is<bool>()) {
            tempBool = root["put"][i]["value"];
#ifdef ENABLE_DIGITALOUT
            digitalOutSetBooleanValue(si->address, tempBool);
#endif
          }
        }
      });

      
    }
  }
  */



}


void sendDelta() {
  bool needToSend = false;

  String deltaText;

  DynamicJsonBuffer jsonBuffer; 

  //  build delta message
  JsonObject& delta = jsonBuffer.createObject();

  //updated array
  JsonArray& updatesArr = delta.createNestedArray("updates");
  



  delta.printTo(deltaText);

  #ifdef ENABLE_SERIAL_DELTA
  Serial.println(deltaText);
  #endif
  
  #ifdef ENABLE_WEBSOCKET_SERVER
  webSocketServer.broadcastTXT(deltaText);
  #endif
  
  if (signalKClientInfo.connected) { // client
    signalKClientInfo.client.sendTXT(deltaText);
    ledBlinker.flip();
  }
  
}

