#ifndef _bme280_H_
#define _bme280_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"


class BME280SensorInfo : public SensorInfo {
  public:
    BME280SensorInfo(String addr);
    BME280SensorInfo(String addr, 
    				 String path1, String path2, String path3, String path4,
    				 float offset0, float offset1, float offset2, float offset3, 
    				 float scale0, float scale1, float scale2, float scale3);

    static BME280SensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};


void setupBME280();
void handleBME280(bool&);
void interruptReadBME(void *pArg);
void updateBME280();
#endif
