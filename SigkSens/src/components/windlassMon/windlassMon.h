#ifndef _windlassMon_H_
#define _windlassMon_H_

#include "../componentSensor.h"

class ComponentWindlassMon: public ComponentSensor {
  public:
    using ComponentSensor::ComponentSensor; //inherit constructor

    void setupComponent();
    void handleComponent();
};


#endif