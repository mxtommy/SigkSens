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
  private:
    WindlassStates windlassState = idle;
    uint32_t lastEventTime = 0;
    uint8_t channelUpPin;
    uint8_t channelDownPin;
    bool lockState;

  public:
    ComponentWindlassCtrl(const char * name, uint8_t upPin, uint8_t downPin);

    void setupComponent();
    void handleComponent();
    void sendLockStatus();

    void handleCallbackUp(bool newValue);
    void handleCallbackDown(bool newValue);
    void handleCallbackLock(bool newValue);

    void setChannelUpActive();
    void setChannelDownActive();
    void setWindlassIdle();
    
};


#endif