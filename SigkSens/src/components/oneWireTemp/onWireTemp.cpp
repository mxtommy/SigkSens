#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif


#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "oneWireTemp.h"

#include <OneWire.h>
#include <DallasTemperature.h>


ComponentOneWireTemp::ComponentOneWireTemp(const char * name, uint8_t oneWirePin) : ComponentSensor(name)  {
  _oneWirePin = oneWirePin;
  _oneWireBus = new OneWire(_oneWirePin);
  _sensors = new DallasTemperature(&*_oneWireBus);
}



void ComponentOneWireTemp::setupComponent() {
  

  _sensors->begin();
  _sensors->setWaitForConversion(false);
  Serial.print(F("1-Wire Parasite power is: ")); 
  if (_sensors->isParasitePowerMode()) Serial.println(F("ON"));
  else Serial.println(F("OFF"));
  Serial.print(F("1-Wire Device precision currently: "));
  Serial.print(_sensors->getResolution());
  Serial.print(F(" setting to "));
  Serial.print(TEMPERATURE_PRECISION);
  _sensors->setResolution(TEMPERATURE_PRECISION);
  Serial.println(F(" Done!"));
  Serial.print(F("Scanning 1-Wire Bus on pin: "));
  Serial.println(_oneWirePin);
  scanBus();

  app.onRepeat(1000, [this]() { this->handleComponent(); });

}

void ComponentOneWireTemp::handleComponent() {
  config.handle(); //saves config if any changes
  if (!config.getBool("enabled")) {
    return;
  }
  
  _sensors->requestTemperatures();
  app.onDelay(ONEWIRE_READ_DELAY, [this]() { this->readSensors(); });
  
}

void ComponentOneWireTemp::readSensors() {
  uint8_t deviceAddress[8];
  char charAddress[32];
  int numberOfDevices = 0;
  float tempK;
  float tempC;

  _sensors->begin(); //needed so the library searches for new sensors that came up since boot

  numberOfDevices = _sensors->getDeviceCount();
  for(int i=0;i<numberOfDevices; i++) {
    if(_sensors->getAddress(deviceAddress, i)) {
      addrToChar(charAddress, deviceAddress);

      tempC = _sensors->getTempC(deviceAddress);
      if (tempC == DEVICE_DISCONNECTED_C) {
        signalK.addValue(config.getString(charAddress, "environment.inside." + String(charAddress) + ".temperature" ), "null");
        break;
      } else {
        tempK = tempC + 273.15;
        signalK.addValue(config.getString(charAddress, "environment.inside." + String(charAddress) + ".temperature" ), tempK);
      }      
    }
  }

}

void ComponentOneWireTemp::scanBus() {
  uint8_t tempDeviceAddress[8];
  char charAddress[32];

  int numberOfDevices = 0;

  _sensors->begin(); //needed so the library searches for new sensors that came up since boot
  numberOfDevices = _sensors->getDeviceCount();
  Serial.println(numberOfDevices);

  for(int i=0;i<numberOfDevices; i++) {
    if(_sensors->getAddress(tempDeviceAddress, i))
    {
      addrToChar(charAddress, tempDeviceAddress);
      //set a path (if not already set :) )

      config.getString(charAddress, "environment.inside." + String(charAddress) + ".temperature" );

      Serial.print("found oneWire temp at: "); Serial.println(charAddress);
    }
  }
}


void ComponentOneWireTemp::addrToChar(char *strAddress, uint8_t *deviceAddress) {
  // convert to string
  sprintf(strAddress, "%02X_%02X_%02X_%02X_%02X_%02X_%02X_%02X",
    deviceAddress[0],
    deviceAddress[1],
    deviceAddress[2],
    deviceAddress[3],
    deviceAddress[4],
    deviceAddress[5],
    deviceAddress[6],
    deviceAddress[7] );  
}




