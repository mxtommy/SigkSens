extern "C" {
#include "user_interface.h"
}

#include <Wire.h>

#include "config.h"

#include "sigksens.h"

#include "i2c.h"


#ifdef ENABLE_SHT30
  #include "sht30.h"
#endif
#ifdef ENABLE_MPU
  #include "mpu9250.h"
#endif
#ifdef ENABLE_BMP280
  #include "bmp280.h"
#endif

#ifdef ENABLE_ADS1115
  #include "ads1115.h"
#endif


#ifdef ENABLE_SHT30
bool sensorSHT30Present = false;
bool getSensorSHT30Present() { return sensorSHT30Present; }
#endif

#ifdef ENABLE_MPU
bool sensorMPU925XPresent = false;
bool getSensorMPU925XPresent() { return sensorMPU925XPresent; }
#endif

#ifdef ENABLE_BMP280
bool sensorBMP280Present = false;
bool getSensorBMP280Present() { return sensorBMP280Present; }
#endif

#ifdef ENABLE_ADS1115
bool sensorADS1115Present = false;
bool getSensorADS1115Present() { return sensorADS1115Present; }
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
  for (int i=0; i < 0x7f; i++) {
    if (scanI2CAddress(i)) {
      Serial.printf("Found I2C device at address %x\n", i);
    }
  }
}

void scanI2C(bool &need_save) {
  SensorInfo *tmpSensorInfo;

  Serial.println("Scanning for i2c Sensors...");

  scanAllI2C();

  #ifdef ENABLE_SHT30
  //SHT30
  if (scanI2CAddress(0x45)) { //Sht on D1 sheild
    sensorSHT30Present = true;
    bool known = sensorStorage[(int)SensorType::sht30].find(
      "0x45") != nullptr;
    if (!known) {
      Serial.print("New SHT Sensor found at: 0x45 ");
      SensorInfo *newSensor = new SHT30SensorInfo("0x45");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    
  }
  #endif

  #ifdef ENABLE_MPU
  //MPU925X
  if (scanI2CAddress(0x68)) {
    sensorMPU925XPresent = true;
    bool known = sensorStorage[(int)SensorType::mpu925x].find(
      "0x68") != nullptr;
    if (!known) {
      Serial.print("New MPU925X found at: 0x68 ");
      SensorInfo *newSensor = new MPU9250SensorInfo("0x68");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    
  }
  #endif
  
  #ifdef ENABLE_BMP280
  //BMP280
  if (scanI2CAddress(0x77)) {
    sensorBMP280Present = true;
    bool known = sensorStorage[(int)SensorType::bmp280].find(
      "0x77") != nullptr;
    if (!known) {
      Serial.print("New BMP280 found at: 0x77 ");
      SensorInfo *newSensor = new BMP280SensorInfo("0x77");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }
  }
  #endif

  //ADS1115
  #ifdef ENABLE_ADS1115
  if (scanI2CAddress(0x48)) {
    sensorADS1115Present = true;
    Serial.println("Found ADS1115 chip at 0x48");
    bool known = sensorStorage[(int)SensorType::ads1115].find(
      "0x48") != nullptr;
    if (!known) {
      SensorInfo *newSensor = new ADSSensorInfo("0x48");
      sensorStorage[(int)newSensor->type].add(newSensor);
      need_save = true;
    }    
  }
  #endif
}


void setupI2C(bool &need_save) {
  //Wire.setClock(0000L);

  Wire.begin();
  Wire.setClock(400000); // choose 400 kHz I2C rate
  
  scanI2C(need_save);

  #ifdef ENABLE_SHT30
  if (sensorSHT30Present) {
    setupSHT30();
  }
  #endif

  #ifdef ENABLE_MPU
  if (sensorMPU925XPresent) {
    setupMPU9250();
  } 
  #endif
    
  #ifdef ENABLE_BMP280
  if (sensorBMP280Present) {
      setupBMP280();
  }
  #endif

  #ifdef ENABLE_ADS1115
  if (sensorADS1115Present) {
    setupADS1115();
  }
  #endif
}
