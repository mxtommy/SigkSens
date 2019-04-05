#ifndef _windlassMon_H_
#define _windlassMon_H_

#include "../componentSensor.h"

class ComponentWindlassMon: public ComponentSensor {
  private:
    uint8_t _upPin;
    uint8_t _downPin;
    uint8_t _countPin;

    bool upState;
    bool downState;
    int16_t _chainCounterCount;
    uint32_t _lastCountSave;


  public:
    ComponentWindlassMon(const char * name, uint8_t upPin, uint8_t downPin, uint8_t countPin);    

    void setupComponent();
    void handleComponent();
};


#endif