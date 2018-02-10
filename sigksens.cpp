#include "sigksens.h"

fromJsonFunc fromJson[(int)SensorType::SensorType_MAX];
/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Helper functions
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/


void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
  }
}

void SensorStorage::add(SensorInfo* sens) {
  sensorStorage.add(sens);
}

SensorInfo* SensorStorage::get(const int i) {
  return sensorStorage.get(i);
}

SensorInfo* SensorStorage::find(String addr) {
  SensorInfo* current;
  const char* addr_cstr = addr.c_str();
  for (int i=0 ; i < sensorStorage.size() ; i++) {
    current = sensorStorage.get(i);
    if (strcmp(current->address, addr_cstr) == 0) {
      return current;
    }
  }
}

int SensorStorage::size() {
  return sensorStorage.size();
}

SensorStorage sensorStorage[(int)SensorType::SensorType_MAX+1];
