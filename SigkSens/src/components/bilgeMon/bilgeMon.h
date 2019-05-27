#ifndef _BILGEMON_H_
#define _BILGEMON_H_

#include "../componentSensor.h"

#include <Adafruit_ADS1015.h>

class ComponentBilgeMonitor : public ComponentSensor {
  private: 
    
    uint8_t _inputPin;
    bool _pinStateRunning;
    uint32_t _runCount;
    uint32_t _runTime;

    bool _currentState;
    uint32_t _currentRunStartTime;
    uint32_t _oldRunTime;

    uint16_t _hourlyRunTime[24];

  public:
    //using ComponentSensor::ComponentSensor; //inherit constructor
    ComponentBilgeMonitor(const char * name, uint8_t inputPin, bool pinStateRunning = HIGH);
    
    

    void setupComponent();
    void handleComponent();
    

};

#endif
