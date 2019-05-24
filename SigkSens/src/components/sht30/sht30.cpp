#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "../../services/i2c.h"
#include <Wire.h>
#include "sht30.h"

ComponentSHT30::ComponentSHT30(const char * name, uint8_t address) : ComponentSensor(name)  {
  _address = address;
}


void ComponentSHT30::setupComponent() {

  //sets default if not already defined
  config.getBool("enableSHT30", true);
  config.getString("pathTemp", "environment.inside.temperature");
  config.getString("pathHumidity", "environment.inside.humidity");



  i2cBus->start(); 

  app.onRepeat(1000, [this]() { this->handleComponent(); });
}


void ComponentSHT30::handleComponent() {
  config.handle(); //saves config if any changes
  if (!config.getBool("enableSHT30")) { return; }

  uint8_t errorCode;

  Wire.beginTransmission(_address);
  Wire.write(0x2C);
  Wire.write(0x06);
  errorCode = Wire.endTransmission();
  if (errorCode != 0) {
      Serial.print(F("Error pollling SHT at address: "));
      Serial.print(_address, HEX);
      Serial.print(F(" ErrorCode: "));
      Serial.println(errorCode);
      return;
  }
  app.onDelay(150, [this]() { this->readSHT(); });

}

void ComponentSHT30::readSHT() {
  uint8_t errorCode;
  uint8_t data[6] = { 0 };
  float tempK;
  float humidity;

  Wire.beginTransmission(_address);
  Wire.requestFrom(_address, (uint8_t)6); // either both int or both uint8_t...
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
    Serial.print(_address, HEX);
    Serial.print(F(" ErrorCode: "));
    Serial.println(errorCode);
    signalK.addValue(config.getString("pathTemp"), "null");
    signalK.addValue(config.getString("pathHumidity"), "null");
    return;
  }

  // Convert the data
  tempK = (((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45) + 273.15;
  humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

  signalK.addValue(config.getString("pathTemp"), tempK);
  signalK.addValue(config.getString("pathHumidity"), humidity);


}