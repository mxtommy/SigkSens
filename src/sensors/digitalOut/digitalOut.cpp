extern "C" {
#include "user_interface.h"
}

#include "../../../config.h"

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
    jsonSens["attrs"][0]["signalKPath"]
  );
}

void DigitalOutSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::digitalOut;
  JsonArray& jsonAttrs = jsonSens.createNestedArray("attrs");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    JsonObject& attr = jsonAttrs.createNestedObject();
    attr["name"] = attrName[x];
    attr["signalKPath"] = signalKPath[x];
    attr["value"] = valueJson[x];
  }
}

int   digitalOutPins[NUMBER_DIGITAL_OUTPUT] = DIGITAL_OUTPUT_PINS;
char  digitalOutPinNames[NUMBER_DIGITAL_OUTPUT][10] = DIGITAL_OUTPUT_NAME;


// forward declarations
void updateSensorInfo();


void setupDigitalOut(bool &need_save) {
  for (int index=0;index<(sizeof(digitalOutPins)/sizeof(digitalOutPins[0])); index++) {
    initializeDigitalOutPin(index, need_save); 
  }

  app.repeat(SLOW_LOOP_DELAY, &updateSensorInfo);
}


void updateSensorInfo() {
  sensorStorage[(int)SensorType::digitalOut].forEach([&](SensorInfo* si) {
    if (strcmp(si->signalKPath[0].c_str(),  "") != 0) {
      si->isUpdated = true;
    }
  });
}


void initializeDigitalOutPin(uint8_t index, bool &need_save) {
  SensorInfo *si;

  pinMode(digitalOutPins[index], OUTPUT);
  digitalWrite(digitalOutPins[index], LOW);

  // Setup "sensors" if not already existing
  bool known = sensorStorage[(int)SensorType::digitalOut].find(
    digitalOutPinNames[index]) != nullptr;
  if (!known) {
    Serial.print(F("Setting up Digital Output on pin: "));
    Serial.println(digitalOutPinNames[index]);
    DigitalOutSensorInfo *newSensor = new DigitalOutSensorInfo(
      digitalOutPinNames[index]
    );
    sensorStorage[(int)newSensor->type].add(newSensor);         
    need_save = true;
  }      

}


void digitalOutSetBooleanValue(char * address, bool value) {
  SensorInfo *si;
  uint8_t index;
  si = sensorStorage[(int)SensorType::digitalOut].find(
    address
  );

  if (si != nullptr) {

    index = digitalOutGetIndex(address);
    if (index >= 0) {
      //We have both Sensorinfo and pin index
      //current state
      if (value) {
        si->valueJson[0] = "true";
        digitalWrite(digitalOutPins[index], HIGH);
      } else {
        si->valueJson[0] = "false";
        digitalWrite(digitalOutPins[index], LOW);
      }
      si->isUpdated = true;
    }
  }
}


uint8_t digitalOutGetIndex(char * address) {
  for (int index=0;index<(sizeof(digitalOutPins)/sizeof(digitalOutPins[0])); index++) {
    if (strcmp(address, digitalOutPinNames[index]) == 0) {
      return index;
    }
  }
  //not found 
  return -1;
}




