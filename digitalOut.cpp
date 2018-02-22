extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"
#include "digitalOut.h"


DigitalOutSensorInfo::DigitalOutSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  attrName[0] = "state";
  type = SensorType::digitalOut;
  valueJson[0] = "false";

  isUpdated = false;
}

DigitalOutSensorInfo::DigitalOutSensorInfo(String addr, String path) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path;
  attrName[0] = "state";
  type = SensorType::digitalOut;
  valueJson[0] = "false";

  isUpdated = false;
}

DigitalOutSensorInfo *DigitalOutSensorInfo::fromJson(JsonObject &jsonSens) {
  return new DigitalOutSensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0]
  );
}

void DigitalOutSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::digitalIn;
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonPaths.add(signalKPath[x]);
  }
}


void setupDigitalOut(bool &need_save) {

}


void handleDigitalOut(bool &sendDelta) {


}










