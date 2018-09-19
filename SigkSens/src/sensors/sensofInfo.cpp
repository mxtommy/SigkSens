
#include <ArduinoJson.h>


#include "sensorInfo.h"


fromJsonFunc fromJson[(int)SensorType::SensorType_MAX+1];
