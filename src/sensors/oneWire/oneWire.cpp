extern "C" {
#include "user_interface.h"
}

#include <Reactduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>



#include "../../../config.h"
#include "../../../FSConfig.h"

#include "oneWire.h"

#ifndef DEVICE_DISCONNECTED_C
# define DEVICE_DISCONNECTED_C DEVICE_DISCONNECTED
#endif

OneWireSensorInfo::OneWireSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  attrName[0] = "tempK";
  type = SensorType::oneWire;
  valueJson[0] = "null";

  isUpdated = false;
}

OneWireSensorInfo::OneWireSensorInfo(String addr, String path) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path;
  attrName[0] = "tempK";
  type = SensorType::oneWire;
  valueJson[0] = "null";

  isUpdated = false;
}

OneWireSensorInfo *OneWireSensorInfo::fromJson(JsonObject &jsonSens) {
  return new OneWireSensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"]
  );
}

void OneWireSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::oneWire;
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

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

bool sensorOneWirePresent = false;

// some timers
uint32_t oneWireScanDelay = 30000; //ms between scan

// forward declarations
void oneWireScanBus(bool&);
void request1WSensors();
void read1WSensors();
void onewire_loop();
void onewire_scan_loop();

void setup1Wire(bool &need_save) {
  sensors.begin();
  
  sensors.setWaitForConversion(false);

  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  Serial.print("1Wire Device precision currently: ");
  Serial.print(sensors.getResolution());
  Serial.print(" setting to ");
  Serial.print(TEMPERATURE_PRECISION);
  sensors.setResolution(TEMPERATURE_PRECISION);
  Serial.println(" Done!");

  Serial.print("Starting oneWire scanning timer at: ");
  Serial.print(oneWireScanDelay);
  Serial.println("ms");

  Serial.println("Scanning OneWire Bus");

  oneWireScanBus(need_save);

  app.repeat(1000, &request1WSensors);
  app.repeat(oneWireScanDelay, &onewire_scan_loop);
}

void onewire_scan_loop() {
  bool need_save = false;
  oneWireScanBus(need_save);
  if (need_save) {
    saveConfig();
  }
}

// debug function to print a device address
void printAddress(DeviceAddress deviceAddress) {
 for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) {
      Serial.print(":");
    }
  }
}

void request1WSensors() {
  if (sensorOneWirePresent) {
    sensors.requestTemperatures();

    app.delay(ONEWIRE_READ_DELAY, &read1WSensors);  
  }
}

void read1WSensors() {
  sensorStorage[(int)SensorType::oneWire].forEach([&](SensorInfo* si) {
    DeviceAddress address;
    float tempK;
    float tempC;

    parseBytes(si->address, ':', address,  8, 16); // convert string address to uint_8 array
  
    tempC = sensors.getTempC(address);
    if (tempC == DEVICE_DISCONNECTED_C) {
      si->valueJson[0] = "null";
    } else {
      tempK = tempC + 273.15;
      si->valueJson[0] = tempK;
    } 
    if (strcmp(si->signalKPath[0].c_str(), "") != 0) {
      si->isUpdated = true;    
    }
  });
}


void addrToString(char *strAddress, uint8_t *deviceAddress) {
  // convert to string
  sprintf(strAddress, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
    deviceAddress[0],
    deviceAddress[1],
    deviceAddress[2],
    deviceAddress[3],
    deviceAddress[4],
    deviceAddress[5],
    deviceAddress[6],
    deviceAddress[7] );  
}


void oneWireScanBus(bool &need_save) {
  uint8_t tempDeviceAddress[8];
  char strAddress[32];

  int numberOfDevices = 0;

  sensors.begin(); //needed so the library searches for new sensors that came up since boot

  numberOfDevices = sensors.getDeviceCount();
  if (numberOfDevices > 0) {
    sensorOneWirePresent = true;
  }

  SensorInfo *tmpSensorInfo;

  for(int i=0;i<numberOfDevices; i++) {
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      addrToString(strAddress, tempDeviceAddress);

      //see if it's in sensorInfo
      bool known = sensorStorage[(int)SensorType::oneWire].find(
        strAddress) != nullptr;

      if (!known) {
        Serial.print("New Sensor found: ");
        Serial.print(strAddress);
        Serial.println("");
        OneWireSensorInfo *newSensor = new OneWireSensorInfo(strAddress);
        sensorStorage[(int)newSensor->type].add(newSensor);
        need_save = true;
      }
    }
  }
}


