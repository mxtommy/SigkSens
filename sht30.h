#ifndef _sht30_H_
#define _sht30_H_

extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"

class SHT30SensorInfo : public SensorInfo {
  public:
    SHT30SensorInfo(String addr);
    SHT30SensorInfo(String addr, String path1, String path2);

    void toJson(JsonObject &jsonSens);
    void fromJson();

    bool isSerializable();
};


void setupSHT30();
void handleSHT30();

uint32_t getSensorSHTReadDelay();
void setSHTReadDelay(uint32_t newDelay);

#endif
