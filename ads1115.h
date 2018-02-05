#ifndef _ads1115_H_
#define _ads1115_H_

extern "C" {
#include "user_interface.h"
}

class ADSSensorInfo : public SensorInfo {
  public:
    ADSSensorInfo(String addr);
    ADSSensorInfo(String addr, String path1, String path2);
    ADSSensorInfo(String addr, String path1, String path2, uint32_t offset1, uint32_t offset2, uint32_t scale1, uint32_t scale2);

    static ADSSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);

    bool isSerializable();
};

void setupADS1115();
void handleADS1115();

uint32_t getUpdateADSDelay();
void setADSUpdateDelay(uint32_t newDelay);

void interruptADSSensorInfo(void *pArg);
void updateADS1115();

#endif
