extern "C" {
#include "user_interface.h"
}

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include "../../../config.h"

#include "bmp280.h"

BMP280SensorInfo::BMP280SensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  attrName[0] = "tempK";
  attrName[1] = "Pa";
  type = SensorType::bmp280;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = 0;
  offset[1] = 0;
  scale[0] = 1;
  scale[1] = 1;

  isUpdated = false;
}

BMP280SensorInfo::BMP280SensorInfo(
    String addr, String path1, String path2, float offset0, float offset1, float scale0, float scale1) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "tempK";
  attrName[1] = "Pa";
  type = SensorType::bmp280;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = offset0;
  offset[1] = offset1;
  scale[0] = scale0;
  scale[1] = scale1;

  isUpdated = false;
}

BMP280SensorInfo *BMP280SensorInfo::fromJson(JsonObject &jsonSens) {
  return new BMP280SensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][0]["offset"],
    jsonSens["attrs"][1]["offset"],
    jsonSens["attrs"][0]["scale"],
    jsonSens["attrs"][1]["scale"]
  );
}

void BMP280SensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::bmp280;
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
Adafruit_BMP280 bmp;

//Running values. (running value to reduce noise with exponential filter)
float valueTempK = 0;
float valuePa = 0;

uint16_t readBMPDelay = 25;

// forward declarations
void readBMP280();
void updateBMP280();


void setupBMP280() {
   // this calls Wire.begin() again, but that shouldn't
  // do any harm. Hopefully.
  bmp.begin();
  app.onRepeat(SLOW_LOOP_DELAY, readBMP280);
}

void readBMP280() {
  float Pa;
  float tempK;
  
  Pa = bmp.readPressure();
  tempK = bmp.readTemperature() + 273.15;

  valueTempK = ((0.2*tempK) + (0.8*valueTempK));
  valuePa = ((0.05*Pa) + (0.95*valuePa));

  updateBMP280();
}

void updateBMP280() {
  sensorStorage[(int)SensorType::bmp280].forEach([&](SensorInfo* si) {
    si->valueJson[0] = (valueTempK * si->scale[0] ) + si->offset[0];
    si->valueJson[1] = (valuePa * si->scale[1] ) + si->offset[1];
    si->isUpdated = true;
  });
}