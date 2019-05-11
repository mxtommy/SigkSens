#ifndef _ONEWIRETEMP_H_
#define _ONEWIRETEMP_H_

#include "../componentSensor.h"

#include <OneWire.h>
#include <DallasTemperature.h>

class ComponentOneWireTemp : public ComponentSensor {
  private: 
    
    uint8_t _oneWirePin;
    OneWire* _oneWireBus;
    DallasTemperature _sensors;
    

  public:
    //using ComponentSensor::ComponentSensor; //inherit constructor
    ComponentOneWireTemp(const char * name, uint8_t oneWirePin);
    
    

    void setupComponent();
    void handleComponent();
    
    void readSensors();
    void addrToChar(char *strAddress, uint8_t *deviceAddress);

};

#endif