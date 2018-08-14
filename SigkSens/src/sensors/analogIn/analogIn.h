#ifndef _analogin_H_
#define _analogin_H_

#include "../../../sigksens.h"


bool getSensorAnalogInPresent();

class AinSensorInfo : public SensorInfo {
  public:
    AinSensorInfo(String addr);
    AinSensorInfo( String addr, 
                   String path0, 
                   float offset0,
                   float scale0);
    static AinSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);

};

void setupAnalogIn(bool&);
void handleAnalogIn(bool&);
void interruptReadADC(void *pArg);
void readADC();
void updateAnalogIn();


#endif
