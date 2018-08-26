#ifndef _digitalIn_H_
#define _digitalIn_H_

#include "../../../sigksens.h"



class DigitalInSensorInfo : public SensorInfo {
  public:
    DigitalInSensorInfo(String addr);
    DigitalInSensorInfo(String addr, String path1, String path2, String path3,
                                         float offset0, float offset1, float offset2,
                                         float scale0, float scale1, float scale2);

    static DigitalInSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};

void setupDigitalIn(bool &need_save);
void initializeDigitalPin(uint8_t index, bool &need_save);

void updateDigitalInState(uint8_t index);
void updateDigitalInPeriodic(uint8_t index);

void getDigitalPinName(uint8_t index, char *dstCharArr);

#endif
