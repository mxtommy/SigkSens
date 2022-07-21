#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "Adafruit_INA219_custom.h"

#include "../../../config.h"

#include "ina219.h"

INA219SensorInfo::INA219SensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = String("electrical.batteries.") + String(myHostname) + String(".current");
  signalKPath[1] = String("electrical.batteries.") + String(myHostname) + String(".voltage");
  attrName[0] = "Current(A)";
  attrName[1] = "Voltage(V)";
  type = SensorType::ina219;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = 0;
  offset[1] = 0;
  scale[0] = 1;
  scale[1] = 1;

  isUpdated = false;
}

INA219SensorInfo::INA219SensorInfo(
    String addr, String path1, String path2, float offset0, float offset1, float scale0, float scale1) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "Current(A)";
  attrName[1] = "Voltage(V)";
  type = SensorType::ina219;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = offset0;
  offset[1] = offset1;
  scale[0] = scale0;
  scale[1] = scale1;

  isUpdated = false;
}

INA219SensorInfo *INA219SensorInfo::fromJson(JsonObject &jsonSens) {
  return new INA219SensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][0]["offset"],
    jsonSens["attrs"][1]["offset"],
    jsonSens["attrs"][0]["scale"],
    jsonSens["attrs"][1]["scale"]
  );
}

void INA219SensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::ina219;
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

// sensor object
Adafruit_INA219_custom ina219;

//Running values. (running value to reduce noise with exponential filter)
float valueCurrent = 0;
float valueVoltage = 0;

// forward declarations
void readINA219();
void updateINA219();

void setupINA219() {
  Serial.println(F("Starting INA219 sensor"));
  if (!ina219.begin()) {
    Serial.println(F("Failed to connect to INA219"));
    return;
  }
  if (!ina219.setCalibration(SHUNT_AMP, SHUNT_MV, MAX_VOLT, MAX_AMP)) {
    Serial.println(F("Failed to configure INA219"));
    return;
  }
  app.onRepeat(SLOW_LOOP_DELAY, readINA219);
  Serial.println(F("INA219 ready"));
}

void readINA219() {
  if (!ina219.conversionReady()) {
    // if no new measurements are ready, trigger new and return
    ina219.triggerMeasurement();
    return;
  }

  float current;
  float voltage;
  float power;
  float v_shunt;
  
  current = ina219.getCurrent_A();
  voltage = ina219.getBusVoltage_V();
  power = ina219.getPower_W();
  v_shunt = ina219.getShuntVoltage_mV();
  ina219.triggerMeasurement();

  const float kf = 0.5;//0.95;
  valueCurrent = ((1-kf)*current) + (kf*valueCurrent);
  valueVoltage = ((1-kf)*voltage) + (kf*valueVoltage);

  updateINA219();
}

void updateINA219() {
  sensorStorage[(int)SensorType::ina219].forEach([&](SensorInfo* si) {
    si->valueJson[0] = (valueCurrent * si->scale[0] ) + si->offset[0];
    si->valueJson[1] = (valueVoltage * si->scale[1] ) + si->offset[1];
    si->isUpdated = true;
  });
}