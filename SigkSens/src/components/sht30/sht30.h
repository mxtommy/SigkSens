#ifndef _SHT30_H_
#define _SHT30_H_
#include "../componentSensor.h"
#include "../../../config.h"

class ComponentSHT30 : public ComponentSensor {
  public:
    //using ComponentSensor::ComponentSensor; //inherit constructor
    ComponentSHT30(const char * name, uint8_t address);
    uint8_t _address;

    void setupComponent();
    void handleComponent();
    
    void readSHT();

};

#endif