
extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"
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
    jsonSens["signalKPaths"][0],
    jsonSens["signalKPaths"][1]
  );
}

void SystemHzSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::local;
  JsonArray& jsonAttrNames = jsonSens.createNestedArray("attrNames");
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  JsonArray& jsonValues = jsonSens.createNestedArray("values");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonAttrNames.add(attrName[x]);
    jsonPaths.add(signalKPath[x]);
    jsonValues.add(valueJson[x]);
  }
}



uint32_t systemHzCount = 0, systemHzMs = 0;

float systemHz = 0;

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
}


void updateSystemHz() {
  uint32_t elapsed = millis() - systemHzMs;
  
  if (elapsed == 0) { return; } // getting sporadic devide by 0 exceptions, no harm in skipping a loop.
  
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


void handleSystemHz(bool &sendDelta) {
  if (sendDelta) {
    updateSystemHz();
  }
  systemHzCount++;
}
