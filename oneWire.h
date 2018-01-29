#ifndef _oneWire_H_
#define _oneWire_H_

#include "sigksens.h"
#include "config.h"

class OneWireSensorInfo : public SensorInfo {
  public:
    OneWireSensorInfo(String addr);
    OneWireSensorInfo(String addr, String path);

    void toJson(JsonObject &jsonSens);
    void fromJson();

    bool isSerializable();
};

extern bool sensorOneWirePresent;

bool setup1Wire(bool&);
void handle1Wire(bool&, bool&);
void setOneWireReadDelay(uint32_t);
uint32_t getOneWireReadDelay();

#endif
