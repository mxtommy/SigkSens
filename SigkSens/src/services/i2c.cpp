#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <Wire.h>

#include "../../sigksens.h"
#include "../../config.h"

#include "i2c.h"

#ifdef ENABLE_SHT30
#include "../sensors/sht30/sht30.h"
#endif

#ifdef ENABLE_MPU
#include "../sensors/mpu9250/mpu9250.h"
#endif

#ifdef ENABLE_BMP280
#include "../sensors/bmp280/bmp280.h"
#endif

#ifdef ENABLE_BME280
#include "../sensors/bme280/bme280.h"
#endif

#ifdef ENABLE_ADS1115
#include "../sensors/ads1115/ads1115.h"
#endif


bool scanI2CAddress(uint8_t address) {
  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();
  if (errorCode == 0) {
    return true;
  }
  return false;
}

void scanAllI2C() {
  Serial.println(F("Scanning for i2c Sensors..."));
  for (int i=0; i < 0x7f; i++) {
    if (scanI2CAddress(i)) {
      Serial.printf("Found I2C device at address %x\n", i);
    }
  }
}

void setupI2C(bool &need_save) {
  //Wire.setClock(0000L);

  Wire.begin();
  Wire.setClock(400000); // choose 400 kHz I2C rate
  
  scanAllI2C();

  #ifdef ENABLE_SHT30
  if (scanI2CAddress(0x45)) { //Sht on D1 sheild
    bool known = sensorStorage[(int)SensorType::sht30].find("0x45") != nullptr;
    if (!known) {
      Serial.print(F("New SHT Sensor found at: 0x45 "));
      SensorInfo *newSensor = new SHT30SensorInfo("0x45");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    
    setupSHT30();
  }
  #endif

  #ifdef ENABLE_MPU
  if (scanI2CAddress(0x68)) {
    bool known = sensorStorage[(int)SensorType::mpu925x].find("0x68") != nullptr;
    if (!known) {
      Serial.print(F("New MPU925X found at: 0x68 "));
      SensorInfo *newSensor = new MPU9250SensorInfo("0x68");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    
    setupMPU9250();
  }
  #endif
    
  #ifdef ENABLE_BMP280
  if (scanI2CAddress(0x77)) {
    bool known = sensorStorage[(int)SensorType::bmp280].find("0x77") != nullptr;
    if (!known) {
      Serial.print(F("New BMP280 found at: 0x77 "));
      SensorInfo *newSensor = new BMP280SensorInfo("0x77");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }
    setupBMP280();
  }
  #endif

  #ifdef ENABLE_BME280
  if (scanI2CAddress(0x76)) {
    bool known = sensorStorage[(int)SensorType::bme280].find("0x76") != nullptr;
    if (!known) {
      Serial.print(F("New BME280 found at: 0x76 "));
      SensorInfo *newSensor = new BME280SensorInfo("0x76");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }
    setupBME280();
  }
  #endif

  #ifdef ENABLE_ADS1115
  if (scanI2CAddress(0x48)) {
    Serial.println(F("Found ADS1115 chip at 0x48"));
    bool known = sensorStorage[(int)SensorType::ads1115].find("0x48") != nullptr;
    if (!known) {
      Serial.print(F("New ADS1115 chip found at: 0x48 "));
      SensorInfo *newSensor = new ADSSensorInfo("0x48");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    
    setupADS1115();
  }
  #endif
}
