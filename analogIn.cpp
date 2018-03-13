extern "C" {
#include "user_interface.h"
}

#include "config.h"

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

// By default we start disabled. Ever time we get ready to send a delta, 
// we see if the signalk path is set. If yes then we enable.
bool analogInEnabled = false;

//Running values. (we need to keep running value to reduce noise with exponential filter)
uint16_t valueA0 = 0;

uint16_t readADCDelay = 25;


void setupAnalogIn(bool &need_save) {
    bool known = sensorStorage[(int)SensorType::analogIn].find(
      "A0") != nullptr;
    if (!known) {
      Serial.println("Adding A0 input ");
      SensorInfo *newSensor = new AinSensorInfo("A0");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }
  
  app.repeat(readADCDelay, &readADC);
  app.repeat(SLOW_LOOP_DELAY, &updateAnalogIn);
}


void readADC() {
  if (!analogInEnabled) {
    return;
  }
  sensorStorage[(int)SensorType::analogIn].forEach([&](SensorInfo* si){
    int16_t rawResult;
  
    if (si->type != SensorType::analogIn) return;
    rawResult = analogRead(A0);
    valueA0 = ((0.2*rawResult) + (0.8*valueA0));
  });
}

void updateAnalogIn() {
  // See if there is a signalk path set.
  sensorStorage[(int)SensorType::analogIn].forEach([&](SensorInfo* si){
    int16_t rawResult;
  
    if (si->type != SensorType::analogIn) return;
    if (strcmp(si->signalKPath[0].c_str(),  "") != 0) {
      analogInEnabled = true;
      si->valueJson[0] = (valueA0 * si->scale[0] ) + si->offset[0];
      si->isUpdated = true;
    } else {
      analogInEnabled = false;
    }
  });
}