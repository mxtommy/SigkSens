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

enum class SensorType {
  local,
  digitalIn,
  oneWire,
  sht30,
  mpu925x,
  bmp280,
  ads1115,
  SensorType_MAX = ads1115  // update this if you add items!
};

// memory to save sensor info
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
  private:
    String createKey(int type, String address);
};


class SensorStorage {
  public:
    void add(SensorInfo* sens);
    SensorInfo* find(String addr);
    template<typename F>
    void forEach(F&& lambda);
    int size();
  private: 
    LinkedList<SensorInfo*> sensorStorage;
};

template<typename F>
void SensorStorage::forEach(F&& lambda) {
  SensorInfo* current;
  for (int i=0 ; i < sensorStorage.size() ; ++i) {
    current = sensorStorage.get(i);
    lambda(current);
  }
}

// memory to save sensor info
extern SensorStorage sensorStorage[];

template<typename F>
void sensorStorageForEach(F&& lambda) {
  for (int i=0; i <= (int)SensorType::SensorType_MAX; ++i) {
    sensorStorage[i].forEach(lambda);
  }
}

typedef SensorInfo *(*fromJsonFunc)(JsonObject &);
extern fromJsonFunc fromJson[(int)SensorType::SensorType_MAX];

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);

#endif
