#ifndef _sensorInfo_H_
#define _sensorInfo_H_


#include "sensorType.h"

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class SensorInfo {
  public:
    char address[32];
    String attrName[MAX_SENSOR_ATTRIBUTES];
    String signalKPath[MAX_SENSOR_ATTRIBUTES];
    String valueJson[MAX_SENSOR_ATTRIBUTES];
    float scale[MAX_SENSOR_ATTRIBUTES];
    float offset[MAX_SENSOR_ATTRIBUTES];
    SensorType type;

    bool isUpdated;

    static SensorInfo *fromJson(JsonObject&);
    virtual void toJson(JsonObject&) = 0;
    virtual void setupWebServerHooks(AsyncWebServer&);
  private:
    String createKey(int type, String address);
};

typedef SensorInfo *(*fromJsonFunc)(JsonObject &);

extern fromJsonFunc fromJson[(int)SensorType::SensorType_MAX+1];

#endif