#ifndef _systemHz_H_
#define _systemHz_H_

#include "config.h"

#include "sigksens.h"

class SystemHzSensorInfo : public SensorInfo {
  public:
    SystemHzSensorInfo(String addr);
    SystemHzSensorInfo(
        String addr, 
        String path1, 
        String path2);

    void toJson(JsonObject &jsonSens);
    void fromJson();

    bool isSerializable();
};


void setupSystemHz(bool&);
void handleSystemHz();

#endif
