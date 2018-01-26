extern "C" {
#include "user_interface.h"
}

#include <Wire.h>

#include "sigksens.h"
#include "sht30.h"
#include "mpu.h"
#include "i2c.h"

bool sensorSHT30Present = false;
bool sensorMPU925XPresent = false;


bool getSensorSHT30Present() { return sensorSHT30Present; }
bool getSensorMPU925XPresent() { return sensorMPU925XPresent; }


bool scanI2CAddress(uint8_t address) {
  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();
  if (errorCode == 0) {
    return true;
  }
  return false;
}


void scanI2C(bool &need_save) {
  SensorInfo *tmpSensorInfo;

  Serial.println("Scanning for i2c Sensors...");

  //SHT30
  if (scanI2CAddress(0x45)) { //Sht on D1 sheild
    sensorSHT30Present = true;
    bool known = false;
    for (int x=0;x<sensorList.size() ; x++) {
      tmpSensorInfo = sensorList.get(x);
      if (strcmp(tmpSensorInfo->address, "45") == 0) {
        known = true;                
      }
    }
    if (!known) {
      Serial.print("New SHT Sensor found at: 0x45 ");
      SensorInfo *newSensor = new SensorInfo();
      strcpy(newSensor->address, "45");
      strcpy(newSensor->type,"sht30");
      newSensor->attrName[0] = "tempK";
      newSensor->attrName[1] = "humidity";
      newSensor->signalKPath[0] = "";
      newSensor->signalKPath[1] = "";
      newSensor->valueJson[0] = "null";
      newSensor->valueJson[1] = "null";
      newSensor->isUpdated = false;
      sensorList.add(newSensor);         
      need_save = true;
    }    
  }

  //MPU925X
  if (scanI2CAddress(0x68)) {
    sensorMPU925XPresent = true;
    bool known = false;
    for (int x=0;x<sensorList.size() ; x++) {
      tmpSensorInfo = sensorList.get(x);
      if (strcmp(tmpSensorInfo->address, "68") == 0) {
        known = true;                
      }
    }    
    if (!known) {
      Serial.print("New MPU925X found at: 0x68 ");
      SensorInfo *newSensor = new SensorInfo();
      strcpy(newSensor->address, "68");
      strcpy(newSensor->type,"mpu925x");
      newSensor->attrName[0] = "tempK";
      newSensor->attrName[1] = "yaw";
      newSensor->attrName[2] = "pitch";
      newSensor->attrName[3] = "roll";
      newSensor->signalKPath[0] = "";
      newSensor->signalKPath[1] = "";
      newSensor->signalKPath[2] = "";
      newSensor->signalKPath[3] = "";
      newSensor->valueJson[0] = "null";
      newSensor->valueJson[1] = "null";
      newSensor->valueJson[2] = "null";
      newSensor->valueJson[3] = "null";
      newSensor->isUpdated = false;
      sensorList.add(newSensor);         
      need_save = true;
    }    
  }
}


void setupI2C(bool &need_save) {
  //Wire.setClock(0000L);

  Wire.begin();
  Wire.setClock(400000); // choose 400 kHz I2C rate
  
  scanI2C(need_save);

  if (sensorSHT30Present) {
    setupSHT30();
  }

  if (sensorMPU925XPresent) {
    setupMPU9250();
  } 
}


void handleI2C() {
  if (sensorMPU925XPresent) {
    handleMPU9250();
  }
}


void handleI2C_slow() {
  if (sensorSHT30Present) {
    handleSHT30();  
  }
}
