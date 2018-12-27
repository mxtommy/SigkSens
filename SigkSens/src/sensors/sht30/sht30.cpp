#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <Wire.h>

#include "../../../config.h"
#include "sht30.h"


SHT30SensorInfo::SHT30SensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  attrName[0] = "tempK";
  attrName[1] = "humidity";
  type = SensorType::sht30;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = 0;
  offset[1] = 0;
  scale[0] = 1;
  scale[1] = 1;
  isUpdated = false;
}

SHT30SensorInfo::SHT30SensorInfo( String addr, 
                                  String path1, float offset1, float scale1,
                                  String path2, float offset2, float scale2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "tempK";
  attrName[1] = "humidity";
  type = SensorType::sht30;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = offset1;
  offset[1] = offset2;
  scale[0] = scale1;
  scale[1] = scale2;

  isUpdated = false;
}

SHT30SensorInfo *SHT30SensorInfo::fromJson(JsonObject &jsonSens) {
  return new SHT30SensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][0]["offset"],
    jsonSens["attrs"][0]["scale"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][1]["offset"],
    jsonSens["attrs"][1]["scale"]

  );
}

void SHT30SensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::sht30;
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


/* SensorType info

type="sht30"
[0] = tempK
[1] = humidity

*/

// forward declarations
void pollSHT();
void readSHT();


void setupSHT30() {
  // prepare SHT Timers 
  app.onRepeat(1000, pollSHT);
}


void pollSHT() {
  sensorStorage[(int)SensorType::sht30].forEach([&](SensorInfo* si) {
    uint8_t address;
    uint8_t errorCode;
    //convert address string to int
    parseBytes(si->address,':',&address,1,16);
    Wire.beginTransmission(address);
    Wire.write(0x2C);
    Wire.write(0x06);
    // Stop I2C transmission
    errorCode = Wire.endTransmission();
    if (errorCode != 0) {
      Serial.print(F("Error pollling SHT at address: "));
      Serial.print(address, HEX);
      Serial.print(F(" ErrorCode: "));
      Serial.println(errorCode);
    }
  });

  app.onDelay(150, readSHT);
}


void readSHT() {
  sensorStorage[(int)SensorType::sht30].forEach([&](SensorInfo* si) {
    uint8_t errorCode;
    uint8_t data[6] = { 0 };
    uint8_t address;
    float tempK;
    float humidity;
  
    //convert address string to int
    parseBytes(si->address,':',&address,1,16);

    Wire.beginTransmission(address);
    Wire.requestFrom(address, (uint8_t)6); // either both int or both uint8_t...
    // Read 6 bytes of data
    // cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc
    if (Wire.available() == 6)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
      data[2] = Wire.read();
      data[3] = Wire.read();
      data[4] = Wire.read();
      data[5] = Wire.read();
    }
    errorCode = Wire.endTransmission();
    if (errorCode != 0) {
      Serial.print(F("Error reading from SHT30 at address: "));
      Serial.print(address, HEX);
      Serial.print(F(" ErrorCode: "));
      Serial.println(errorCode);
      si->valueJson[0] = "null";
      si->valueJson[1] = "null";
      return;
    }

    // Convert the data
    tempK = (((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45) + 273.15;
    humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

    si->valueJson[0] = (tempK * si->scale[0]) + si->offset[0];
    si->valueJson[1] = (humidity * si->scale[1]) + si->offset[1];
    si->isUpdated = true;
  });
}
