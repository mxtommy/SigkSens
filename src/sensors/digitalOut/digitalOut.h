#ifndef _digitalOut_H_
#define _digitalOut_H_

extern "C" {
#include "user_interface.h"
}

#include "../../../config.h"
#include "../../../sigksens.h"


class DigitalOutSensorInfo : public SensorInfo {
  public:
    DigitalOutSensorInfo(String addr);
    DigitalOutSensorInfo(String addr, String path);

    static DigitalOutSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};

void setupDigitalOut(bool &need_save);

void initializeDigitalOutPin(uint8_t index, bool &need_save);
void digitalOutSetBooleanValue(char * address, bool value);
uint8_t digitalOutGetIndex(char * address);






#endif
