#ifndef _digitalIn_H_
#define _digitalIn_H_

#include "config.h"

#include "sigksens.h"

class DigitalInSensorInfo : public SensorInfo {
  public:
    DigitalInSensorInfo(String addr);
    DigitalInSensorInfo(
        String addr, 
        String path1, 
        String path2);

    static DigitalInSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);

    bool isSerializable();
};

void setupDigitalIn(bool &need_save);
void handleDigitalIn();
void initializeDigitalPin(uint8_t index, bool &need_save);
void digitalCheckState(uint8_t index);
void updateDigitalIn(uint8_t index);

void setDigitalInUpdateDelay(uint32_t newDelay);
uint32_t getUpdateDigitalInDelay();

uint8_t getDigitalMode(uint8_t index);
bool setDigitalMode(const char *nameCharArr, uint8_t mode);

void getDigitalPinName(uint8_t index, char *dstCharArr);

#endif
