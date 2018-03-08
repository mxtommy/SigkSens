#ifndef _oneWire_H_
#define _oneWire_H_

#include <Reactduino.h>

#include "sigksens.h"
#include "config.h"

class OneWireSensorInfo : public SensorInfo {
  public:
    OneWireSensorInfo(String addr);
    OneWireSensorInfo(String addr, String path);

    static OneWireSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};

void setup1Wire(bool&);
bool getSensorOneWirePresent();

#endif
