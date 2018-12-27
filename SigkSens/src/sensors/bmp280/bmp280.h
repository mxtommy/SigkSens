#ifndef _bmp280_H_
#define _bmp280_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"


class BMP280SensorInfo : public SensorInfo {
  public:
    BMP280SensorInfo(String addr);
    BMP280SensorInfo(String addr, String path1, String path2, float offset0, float offset1, float scale0, float scale1);

    static BMP280SensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};


void setupBMP280();
void handleBMP280(bool&);
void interruptReadBMP(void *pArg);
void updateBMP280();
#endif
