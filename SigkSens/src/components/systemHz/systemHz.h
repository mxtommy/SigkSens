#ifndef _systemHz_H_
#define _systemHz_H_

#include "../componentSensor.h"

class ComponentSystemHz : public ComponentSensor {
  public:
    using ComponentSensor::ComponentSensor; //inherit constructor
    uint32_t systemHzCount;
    uint32_t systemHzMs;
    float systemHz;

    void setupComponent();
    void handleComponent();
    
    void countSystemHz();

};

#endif
