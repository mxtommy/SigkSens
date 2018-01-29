#ifndef _mpu9250_H_
#define _mpu9250_H_

#include "config.h"

#include "sigksens.h"

class MPU9250SensorInfo : public SensorInfo {
  public:
    MPU9250SensorInfo(String addr);
    MPU9250SensorInfo(
        String addr, 
        String path1, 
        String path2,
        String path3, 
        String path4);

    void toJson(JsonObject &jsonSens);
    void fromJson();

    bool isSerializable();
};

bool configureMPU9250();
void processMPU9250();
void updateQuaternion();
void updateMPUSensorInfo();

#endif
