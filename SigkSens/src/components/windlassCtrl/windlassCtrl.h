#ifndef _windlassCtrl_H_
#define _windlassCtrl_H_

#include "../componentSensor.h"


enum WindlassStates {
  idle,
  channel1Active,
  channel2Active,
  idleWait
};


class ComponentWindlassCtrl : public ComponentSensor {
  public:
    using ComponentSensor::ComponentSensor; //inherit constructor
    WindlassStates windlassState = idle;
    uint32_t lastEventTime = 0;

    void setupComponent();
    void handleComponent();

    void handleCallbackUp(bool newValue);
    void handleCallbackDown(bool newValue);

    void setChannel1Active();
    void setChannel2Active();
    void setWindlassIdle();
    
};


#endif