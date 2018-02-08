#ifndef _bmp280_H_
#define _bmp280_H_

extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"

class BMP280SensorInfo : public SensorInfo {
  public:
    BMP280SensorInfo(String addr);
    BMP280SensorInfo(String addr, String path1, String path2);

    static BMP280SensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};


void setupBMP280();
void handleBMP280();

uint32_t getSensorBMP280ReadDelay();
void setBMP280ReadDelay(uint32_t newDelay);

#endif
