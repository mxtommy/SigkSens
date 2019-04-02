#ifndef _DigitalSwitch_H_
#define _DigitalSwitch_H_

#include "../componentSensor.h"

class ComponentDigitalSwitch : public ComponentSensor {
  public:
    //using ComponentSensor::ComponentSensor; //inherit constructor
    ComponentDigitalSwitch(const char * name, uint8_t outPin, uint8_t ads1115Pin);
    
    uint8_t outputPin;
    uint8_t ads1115Pin;
    bool state;
    float current;

    void setupComponent();
    void handleComponent();
    
    void handleCallback(bool newValue);

};

#endif
