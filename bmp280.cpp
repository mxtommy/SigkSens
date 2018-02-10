extern "C" {
#include "user_interface.h"
}

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include "sigksens.h"
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

  isUpdated = false;
}

BMP280SensorInfo::BMP280SensorInfo(
    String addr, String path1, String path2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "tempK";
  attrName[1] = "Pa";
  type = SensorType::bmp280;
  valueJson[0] = "null";
  valueJson[1] = "null";

  isUpdated = false;
}

BMP280SensorInfo *BMP280SensorInfo::fromJson(JsonObject &jsonSens) {
  return new BMP280SensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0],
    jsonSens["signalKPaths"][1]
  );
}

void BMP280SensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::bmp280;
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonPaths.add(signalKPath[x]);
  }
}


// timer delay
uint32_t sensorBMP280ReadDelay = 5000; //ms between reading

uint32_t getSensorBMP280ReadDelay() { return sensorBMP280ReadDelay; }

os_timer_t  sensorBMP280PollTimer; // repeating timer that fires ever X/time to start poll cycle

bool readytoPollBMP280 = false;

void interruptBMP280Poll(void *pArg) {
  readytoPollBMP280 = true;
}

// sensor object
Adafruit_BMP280 bmp;

void setupBMP280() {
  // prepare BMP280 Timers 
  os_timer_setfn(&sensorBMP280PollTimer, interruptBMP280Poll, NULL);

  Serial.print("Starting BMP280 polling timer");
  Serial.print(sensorBMP280ReadDelay);  
  Serial.println("ms");
  os_timer_arm(&sensorBMP280PollTimer, sensorBMP280ReadDelay, true); // TODO should probably confirm we actually have an BMP280 connected
  
  // this calls Wire.begin() again, but that shouldn't
  // do any harm. Hopefully.
  bmp.begin();
}


void readBMP280() {
  uint8_t errorCode;
  uint8_t data[6];
  uint8_t address;
  float Pa;
  float tempK;
  
  readytoPollBMP280 = false; //reset interupt
  
  Pa = bmp.readPressure();
  tempK = bmp.readTemperature() + 273.15;

  sensorStorage[(int)SensorType::bmp280].forEach([&](SensorInfo* si) {
    si->valueJson[0] = tempK;
    si->valueJson[1] = Pa;
    si->isUpdated = true;
  });
}


void handleBMP280() {
  if (readytoPollBMP280){
    digitalWrite(LED_BUILTIN, LOW);
    readBMP280();
    digitalWrite(LED_BUILTIN, HIGH);  
  }
}


void setBMP280ReadDelay(uint32_t newDelay) {
  os_timer_disarm(&sensorBMP280PollTimer);
  sensorBMP280ReadDelay = newDelay;
  Serial.print("Restarting BMP280 polling timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");  
  os_timer_arm(&sensorBMP280PollTimer, sensorBMP280ReadDelay, true);  
}
