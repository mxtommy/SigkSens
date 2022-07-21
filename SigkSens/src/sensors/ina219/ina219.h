#ifndef _ina219_H_
#define _ina219_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"


class INA219SensorInfo : public SensorInfo {
  public:
    INA219SensorInfo(String addr);
    INA219SensorInfo(String addr, String path1, String path2, float offset0, float offset1, float scale0, float scale1);

    static INA219SensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};


void setupINA219();
void handleINA219(bool&);
void updateINA219();
#endif