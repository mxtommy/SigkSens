#ifndef _BILGEMON_H_
#define _BILGEMON_H_

#include "../componentSensor.h"

#include <Adafruit_ADS1015.h>

class ComponentBilgeMonitor : public ComponentSensor {
  private: 
    
    uint8_t _inputPin;
    uint16_t _runCount;
    uint16_t _runTime;

  public:
    //using ComponentSensor::ComponentSensor; //inherit constructor
    ComponentBilgeMonitor(const char * name, uint8_t inputPin);
    
    

    void setupComponent();
    void handleComponent();
    

};

#endif
