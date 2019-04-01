#ifndef _sensorStorage_H_
#define _sensorStorage_H_

#include <map>

#include "sensorInfo.h"


class SensorStorage {
  public:
    void add(SensorInfo* sens);
    SensorInfo* find(String addr);
    template<typename F>
    void forEach(F&& lambda);
  private:
    std::map<String, SensorInfo*> sensorMap;
};

template<typename F>
void SensorStorage::forEach(F&& lambda) {
  // this returns an std::pair of key, value
  for (auto const& x : sensorMap) {
    lambda(x.second);
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



#endif