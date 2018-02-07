#ifndef _ads1115_H_
#define _ads1115_H_

extern "C" {
#include "user_interface.h"
}

class ADSSensorInfo : public SensorInfo {
  public:
    ADSSensorInfo(String addr);
    ADSSensorInfo( String addr, 
                   String path0, String path1, String path2, String path3, String path4, String path5,
                   float offset0, float offset1, float offset2, float offset3, float offset4, float offset5,
                   float scale0, float scale1, float scale2, float scale3, float scale4, float scale5);
    static ADSSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);

};

void setupADS1115();
void handleADS1115();

uint32_t getUpdateADSDelay();
uint32_t getReadADSDelay();
void setADSUpdateDelay(uint32_t newDelay);
void setADSReadDelay(uint32_t newDelay);

void interruptUpdateADSSensorInfo(void *pArg);
void interruptReadADSS(void *pArg);

void updateADS1115();
void readADS1115();

#endif
