#ifndef _oneWire_H_
#define _oneWire_H_

#include "sigksens.h"
#include "config.h"

class OneWireSensorInfo : public SensorInfo {
  public:
    OneWireSensorInfo(String addr);
    OneWireSensorInfo(String addr, String path);

    static OneWireSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
    

    bool isSerializable();
};

void setup1Wire(bool&);
void handle1Wire(bool&);
void setOneWireReadDelay(uint32_t);
uint32_t getOneWireReadDelay();
bool getSensorOneWirePresent();

#endif
