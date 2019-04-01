#include "sensorStorage.h"

void SensorStorage::add(SensorInfo* sens) {
  sensorMap[sens->address] = sens;
}

SensorInfo* SensorStorage::find(String addr) {
  return sensorMap[addr];
}

SensorStorage sensorStorage[(int)SensorType::SensorType_MAX+1];
