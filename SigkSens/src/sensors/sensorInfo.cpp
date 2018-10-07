
#include <ArduinoJson.h>


#include "sensorInfo.h"

void SensorInfo::setupWebServerHooks(AsyncWebServer& server){ }


fromJsonFunc fromJson[(int)SensorType::SensorType_MAX+1];
