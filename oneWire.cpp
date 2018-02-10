/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
One Wire
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

#include <OneWire.h>
#include <DallasTemperature.h>

extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"
#include "oneWire.h"

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
    jsonSens["signalKPaths"][0]
  );
}

void OneWireSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::oneWire;
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonPaths.add(signalKPath[x]);
  }
}

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

bool sensorOneWirePresent = false;

bool getSensorOneWirePresent() { return sensorOneWirePresent; }

// some timers
uint32_t oneWireReadDelay = 5000; //ms between reading
uint32_t oneWireScanDelay = 30000; //ms between scan

os_timer_t  oneWireRequestTimer; // repeating timer that fires ever X/time to start temp request cycle
os_timer_t  oneWireReadyTimer; // once request cycle starts, this timer set so we can send when ready
os_timer_t  oneWireScanTimer; // timer to scan for new devices
bool readyToRequest1Wire = false;
bool readyToRead1Wire = false;
bool readyToScan1Wire = false;

// forward declarations
void oneWireScanBus(bool&);
void interruptRequest1WSensors(void *pArg);
void interruptReady1WSensors(void *pArg);
void interruptScan1WSensors(void *pArg);
void request1WSensors();
void read1WSensors();

uint32_t getOneWireReadDelay() { return oneWireReadDelay; }

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

  Serial.print("Starting oneWire polling timer at: ");
  Serial.print(oneWireReadDelay);
  Serial.println("ms");

  Serial.print("Starting oneWire scanning timer at: ");
  Serial.print(oneWireScanDelay);
  Serial.println("ms");

  Serial.println("Scanning OneWire Bus");

  oneWireScanBus(need_save);

  os_timer_setfn(&oneWireRequestTimer, interruptRequest1WSensors, NULL);
  os_timer_setfn(&oneWireReadyTimer, interruptReady1WSensors, NULL);
  os_timer_setfn(&oneWireScanTimer, interruptScan1WSensors, NULL);

  os_timer_arm(&oneWireRequestTimer, oneWireReadDelay, true);
  os_timer_arm(&oneWireScanTimer, oneWireScanDelay, true);
}

//called once every loop()
void handle1Wire(bool &need_save) {

  // If it's time to request temps, well request it...
  if (readyToRequest1Wire) {
    request1WSensors();
  }

  //ready to send temps! 
  if (readyToRead1Wire) {
    read1WSensors();
  }

  if (readyToScan1Wire) {
    oneWireScanBus(need_save);
  }

}

void setOneWireReadDelay(uint32_t newDelay) {
  os_timer_disarm(&oneWireRequestTimer);
  Serial.print("Restarting oneWire polling timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");
  oneWireReadDelay = newDelay;
  os_timer_arm(&oneWireRequestTimer, oneWireReadDelay, true);
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

void interruptRequest1WSensors(void *pArg) {
  readyToRequest1Wire = true;
}

void interruptReady1WSensors(void *pArg) {
  readyToRead1Wire = true;
} 

void interruptScan1WSensors(void *pArg) {
  readyToScan1Wire = true;
} 

void request1WSensors() {
  readyToRequest1Wire = false; // reset interupt

  if (sensorOneWirePresent) {
      sensors.requestTemperatures();

    // start ready timer
    os_timer_arm(&oneWireReadyTimer, ONEWIRE_READ_DELAY, false); // false = no loop
  }
  
}

void read1WSensors() {
  readyToRead1Wire = false; // reset interupt
  SensorInfo *thisSensorInfo;
  DeviceAddress address;
  float tempK;
  float tempC;

  for (uint8_t i=0; i < sensorStorage[(int)SensorType::oneWire].size(); i++) {
    thisSensorInfo = sensorStorage[(int)SensorType::oneWire].get(i);
    if (thisSensorInfo->type==SensorType::oneWire) {
      parseBytes(thisSensorInfo->address, ':', address,  8, 16); // convert string address to uint_8 array
    
      tempC = sensors.getTempC(address);
      if (tempC == DEVICE_DISCONNECTED) {
        thisSensorInfo->valueJson[0] = "null";
      } else {
        tempK = tempC + 273.15;
        thisSensorInfo->valueJson[0] = tempK;
      } 
      thisSensorInfo->isUpdated = true;
    }

  }
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
  readyToScan1Wire = false; // reset Interrupt

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


