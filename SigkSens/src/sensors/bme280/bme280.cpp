#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "../../../config.h"

#include "bme280.h"

BME280SensorInfo::BME280SensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  signalKPath[2] = "";
  signalKPath[3] = "";
  attrName[0] = "tempK";
  attrName[1] = "Pa";
  attrName[2] = "humidity";
  attrName[3] = "altitude";
  type = SensorType::bme280;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  valueJson[3] = "null";
  offset[0] = 0;
  offset[1] = 0;
  offset[2] = 0;
  offset[3] = 0;
  scale[0] = 1;
  scale[1] = 1;
  scale[2] = 1;
  scale[3] = 1;

  isUpdated = false;
}

BME280SensorInfo::BME280SensorInfo(String addr, String path1, String path2, String path3, String path4, 
                                   float offset0, float offset1, float offset2, float offset3,
                                   float scale0, float scale1, float scale2, float scale3) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  signalKPath[2] = path3;
  signalKPath[3] = path4;
  attrName[0] = "tempK";
  attrName[1] = "Pa";
  attrName[2] = "humidity";
  attrName[3] = "altitude";
  type = SensorType::bme280;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  valueJson[3] = "null";
  offset[0] = offset0;
  offset[1] = offset1;
  offset[2] = offset2;
  offset[3] = offset3;
  scale[0] = scale0;
  scale[1] = scale1;
  scale[2] = scale2;
  scale[3] = scale3;

  isUpdated = false;
}

BME280SensorInfo *BME280SensorInfo::fromJson(JsonObject &jsonSens) {
  return new BME280SensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][2]["signalKPath"],
    jsonSens["attrs"][3]["signalKPath"],
    jsonSens["attrs"][0]["offset"],
    jsonSens["attrs"][1]["offset"],
    jsonSens["attrs"][2]["offset"],
    jsonSens["attrs"][3]["offset"],
    jsonSens["attrs"][0]["scale"],
    jsonSens["attrs"][1]["scale"],
    jsonSens["attrs"][2]["scale"],
    jsonSens["attrs"][3]["scale"]
  );
}

void BME280SensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::bme280;
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
Adafruit_BME280 bme;

//Running values. (running value to reduce noise with exponential filter)
float valuTempK = 0;
float valuPa = 0;
float valueHum = 0;
float valueAlt = 0;

uint16_t readBMEDelay = 25;

// forward declarations
void readBME280();
void updateBME280();


void setupBME280() {
   // this calls Wire.begin() again, but that shouldn't
  // do any harm. Hopefully.
  bme.begin(0x76);
  app.onRepeat(SLOW_LOOP_DELAY, readBME280);
}

void readBME280() {
  float TempK;
  float Pa;
  float Hum;
  float Alt;
  
  Pa = bme.readPressure();
  TempK = bme.readTemperature() + 273.15;
  valueHum = bme.readHumidity();
  valueAlt = bme.readAltitude(SEALEVELPRESSURE_HPA);  
  valuPa = ((0.05*Pa) + (0.95*valuPa));
  valuTempK = ((0.2*TempK) + (0.8*valuTempK));

  updateBME280();

}

void updateBME280() {
  sensorStorage[(int)SensorType::bme280].forEach([&](SensorInfo* si) {
    si->valueJson[0] = (valuTempK * si->scale[0] ) + si->offset[0];
    si->valueJson[1] = (valuPa * si->scale[1] ) + si->offset[1];
    si->valueJson[2] = (valueHum * si->scale[2] ) + si->offset[2];
    si->valueJson[3] = (valueAlt * si->scale[3] ) + si->offset[3];
    si->isUpdated = true;
  });
}
