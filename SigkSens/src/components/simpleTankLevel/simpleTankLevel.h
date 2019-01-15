#ifndef _SIMPLETANKLEVEL_H_
#define _SIMPLETANKLEVEL_H_
#include "../componentSensor.h"
#include "../../../config.h"

class ComponentSimpleTankLevel : public ComponentSensor {
  public:
    using ComponentSensor::ComponentSensor; //inherit constructor

    uint8_t inputPins[NUMBER_SIMPLE_TANK_PINS] = SIMPLE_TANK_PINS;
    float outputValues[NUMBER_SIMPLE_TANK_PINS] = SIMPLE_TANK_LEVEL;

    void setupComponent();
    void handleComponent();
    

};

#endif