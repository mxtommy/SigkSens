#include "systemHz.h"

SystemHzSensorInfo::SystemHzSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  attrName[0] = "systemHz";
  attrName[1] = "freeMem";
  type = SensorType::local;
  valueJson[0] = "null";
  valueJson[1] = "null";

  isUpdated = false;
}

SystemHzSensorInfo::SystemHzSensorInfo(String addr, 
                                         String path1, 
                                         String path2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "systemHz";
  attrName[1] = "freeMem";
  type = SensorType::local;
  valueJson[0] = "null";
  valueJson[1] = "null";

  isUpdated = false;
}

SystemHzSensorInfo *SystemHzSensorInfo::fromJson(JsonObject &jsonSens) {
  return new SystemHzSensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"]
  );
}

void SystemHzSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::local;
  JsonArray& jsonAttrs = jsonSens.createNestedArray("attrs");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    JsonObject& attr = jsonAttrs.createNestedObject();
    attr["name"] = attrName[x];
    attr["signalKPath"] = signalKPath[x];
    attr["offset"] = offset[x];
    attr["scale"] = scale[x];
    attr["value"] = valueJson[x];
  }
}


uint32_t systemHzCount = 0, systemHzMs = 0;

float systemHz = 0;


// forward declarations
void countSystemHz();
void updateSystemHz();


void setupSystemHz(bool &need_save) {
  systemHzMs = millis();
  SensorInfo *tmpSensorInfo;

  // Setup "sensor" if not already existing
  bool known = sensorStorage[(int)SensorType::local].find(
    "Local") != nullptr;

  if (!known) {
    Serial.print("Setting up System info ");
    SensorInfo *newSensor = new SystemHzSensorInfo("Local");
    sensorStorage[(int)newSensor->type].add(newSensor);
    need_save = true;
  }
  app.onTick(&countSystemHz);
  app.repeat(SLOW_LOOP_DELAY, &updateSystemHz);
}


void countSystemHz() {
  systemHzCount++;
}


void updateSystemHz() {
  uint32_t elapsed = millis() - systemHzMs;
  
  if (elapsed == 0) { return; } // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  
  systemHz = (systemHzCount*1000) / elapsed;
 // Serial.print ("System Hz :");
 // Serial.println (systemHz);

  sensorStorage[(int)SensorType::local].forEach([&](SensorInfo* si) {
    if (strcmp(si->signalKPath[0].c_str(),  "") != 0) {
      si->valueJson[0] = systemHz;
      si->isUpdated = true;
    }
    if (strcmp(si->signalKPath[1].c_str(),  "") != 0) {
      si->valueJson[1] = ESP.getFreeHeap();
      si->isUpdated = true;
    } 
  });

  systemHzCount = 0;
  systemHzMs = millis();
}
