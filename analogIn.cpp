extern "C" {
#include "user_interface.h"
}


#include "sigksens.h"
#include "analogIn.h"



AinSensorInfo::AinSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  attrName[0] = "a0";
  type = SensorType::analogIn;
  valueJson[0] = "null";
  offset[0] = 0;
  scale[0] = 1;

  isUpdated = false;
}

AinSensorInfo::AinSensorInfo( String addr, 
                              String path0, 
                              float offset0,
                              float scale0) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path0;
  attrName[0] = "a0";
  type = SensorType::analogIn;
  valueJson[0] = "null";
  offset[0] = offset0;
  scale[0] = scale0;
  
  isUpdated = false;
}

AinSensorInfo *AinSensorInfo::fromJson(JsonObject &jsonSens) {
  return new AinSensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0],
    jsonSens["offsets"][0],
    jsonSens["scales"][0]
  );
}

void AinSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::analogIn;
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  JsonArray& jsonOffsets = jsonSens.createNestedArray("offsets");
  JsonArray& jsonScales = jsonSens.createNestedArray("scales");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonPaths.add(signalKPath[x]);
    jsonOffsets.add(offset[x]);
    jsonScales.add(scale[x]);
  }
}

void setupAnalogIn(bool &need_save) {
    bool known = sensorStorage[(int)SensorType::analogIn].find(
      "A0") != nullptr;
    if (!known) {
      Serial.print("Adding A0 input ");
      SensorInfo *newSensor = new AinSensorInfo("A0");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    

}


void handleAnalogIn(bool &sendDelta) {
    //Serial.println(analogRead(A0));
};

