#ifndef _DigitalSwitch_H_
#define _DigitalSwitch_H_

#include "../componentSensor.h"

#include <Adafruit_ADS1015.h>

class ComponentDigitalSwitch : public ComponentSensor {
  private: 
    Adafruit_ADS1115 ads;
    
    uint8_t outputPin;
    uint8_t adsAddress;
    uint8_t adsChannel;
    bool state;
    float current;    

  public:
    //using ComponentSensor::ComponentSensor; //inherit constructor
    ComponentDigitalSwitch(const char * name, uint8_t outPin, uint8_t adsAddress, uint8_t adsChannel);
    
    

    void setupComponent();
    void handleComponent();
    
    void handleCallback(bool newValue);

};

#endif
