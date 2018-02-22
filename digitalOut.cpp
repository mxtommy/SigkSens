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

int   digitalOutPins[NUMBER_DIGITAL_OUTPUT] = DIGITAL_INPUT_PINS;
char  digitalOutPinNames[NUMBER_DIGITAL_OUTPUT][10] = DIGITAL_INPUT_NAME;


void setupDigitalOut(bool &need_save) {
  for (int index=0;index<(sizeof(digitalOutPins)/sizeof(digitalOutPins[0])); index++) {
    initializeDigitalOutPin(index, need_save); 
  }
}


void handleDigitalOut(bool &sendDelta) {


}



void initializeDigitalOutPin(uint8_t index, bool &need_save) {
  SensorInfo *si;

  pinMode(digitalOutPins[index], OUTPUT);
  digitalWrite(digitalOutPins[index], LOW);

  // Setup "sensors" if not already existing
  bool known = sensorStorage[(int)SensorType::digitalOut].find(
    digitalOutPinNames[index]) != nullptr;
  if (!known) {
    Serial.print("Setting up Digital Input on pin: ");
    Serial.println(digitalOutPinNames[index]);
    DigitalOutSensorInfo *newSensor = new DigitalOutSensorInfo(
      digitalOutPinNames[index]
    );
    sensorStorage[(int)newSensor->type].add(newSensor);         
    need_save = true;
  }      

}






