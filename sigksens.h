#ifndef _sigksens_H_
#define _sigksens_H_

#include <Arduino.h>
#include <WString.h>
#include <LinkedList.h>
#include <ArduinoJson.h>


#define MAX_SENSOR_ATTRIBUTES 10

//Digital Input
#define DIGITAL_MODE_OFF 0
#define DIGITAL_MODE_IN_STATE 1
#define DIGITAL_MODE_IN_FREQ 2

extern char myHostname[16];

// memory to save sensor info
class SensorInfo {
  public:
    char address[32];
    String attrName[MAX_SENSOR_ATTRIBUTES];
    String signalKPath[MAX_SENSOR_ATTRIBUTES];
    String valueJson[MAX_SENSOR_ATTRIBUTES];
    char type[10];
    bool isUpdated;

    bool isSerializable();
    void toJson(JsonObject&);
};

// memory to save sensor info
extern LinkedList<SensorInfo*> sensorList;

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);

#endif
