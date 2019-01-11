#ifndef _systemHz_H_
#define _systemHz_H_

#include "../sensorComponent.h"

class ComponentSystemHz : public SensorComponent {
  public:
    using SensorComponent::SensorComponent; //inherit constructor
    //ComponentSystemHz(const char * name);
    uint32_t systemHzCount;
    uint32_t systemHzMs;
    float systemHz;

    void setupComponent();
    void handleComponent();
    
    void countSystemHz();

};

#endif
