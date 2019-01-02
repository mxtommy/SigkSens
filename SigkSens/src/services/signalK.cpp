#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#include <ArduinoJson.h> 

#include "config.h"
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
  signalK.sendDeltas(); // not sure onRepeat will work directly with the singlaK Object instance...
}


void SignalK::addValue(String path, String value) {
  _mapValues[path] = value;
}

void SignalK::addValue(String path, bool value) {
  if (value) {
    _mapValues[path] = String("true");
  } else {
    _mapValues[path] = String("false");
  }
  
}

void SignalK::addValue(String path, uint8_t value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, uint16_t value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, uint32_t value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, int8_t value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, int16_t value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, int32_t value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, float value) {
  _mapValues[path] = String(value);
}

void SignalK::addValue(String path, double value) {
  _mapValues[path] = String(value);
}

void SignalK::sendDeltas() {
  if (!_mapValues.empty()) {
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
    _mapValues.clear();
  }
}

void SignalK::registerCallbackBool(String key, void (* CallbackFunction)(bool)) {
  _mapCallbackBool[key] = CallbackFunction;
}

void SignalK::requestAuth() {
  String requestText;
  DynamicJsonBuffer jsonBuffer; 
  JsonObject& root = jsonBuffer.createObject();
  root["requestId"] = uuidv4();
  JsonObject& request = root.createNestedObject("accessRequest");
  request["clientId"] = configStore.getString("myUUID", uuidv4());
  request["description"] = configStore.getString("myHostname");
  request["permissions"] = "readwrite";
  root.printTo(requestText);
  #ifdef ENABLE_SERIAL_DELTA
    Serial.println(requestText);
  #endif
  if (signalKClientInfo.connected) { // client
      signalKClientInfo.client.sendTXT(requestText);
      ledBlinker.flip();
  }
}

void SignalK::receiveDelta(uint8_t * payload) {
  Serial.println("receivingDelta");
  DynamicJsonBuffer jsonBuffer;
//  char tempStr[255];
//  bool tempBool;

  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println(F("parseObject() failed"));
    return;
  }
  root.prettyPrintTo(Serial);
  if (root.containsKey("accessRequest")) {
    JsonObject& accessRequest = root["accessRequest"];
    if (accessRequest.containsKey("token")) {
      configStore.putString("accessToken", accessRequest["token"]);
      Serial.println ("Got new token!");
    }
    
  }
/*
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
  } */
}


void sendDelta() {
  /*
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
  */
}

