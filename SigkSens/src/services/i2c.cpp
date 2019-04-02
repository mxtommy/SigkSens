#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#include "../../sigksens.h"

#include "i2c.h"


#include <Wire.h>




I2cBus::I2cBus() {
  started = false;
}



void I2cBus::start() {
  if (!started) {
    Wire.begin();
    Wire.setClock(400000); // choose 400 kHz I2C rate
    started = true;
    scanAll();
  }
}



bool I2cBus::scanAddress(uint8_t address) {
  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();
  if (errorCode == 0) {
    return true;
  }
  return false;
}

void I2cBus::scanAll() {
  Serial.println(F("Scanning for i2c Sensors..."));
  for (int i=0; i < 0x7f; i++) {
    if (scanAddress(i)) {
      Serial.printf("Found I2C device at address %x\n", i);
    }
  }
}
