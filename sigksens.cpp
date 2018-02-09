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
  sensorList.add(sens);
}

SensorInfo* SensorStorage::get(const int i) {
  return sensorList.get(i);
}

SensorInfo* SensorStorage::find(String key) {
  SensorInfo* current;
  for (int i=0 ; i < sensorList.size() ; i++) {
    current = sensorList.get(i);
    if (current->key == key) {
      return current;
    }
  }
}

int SensorStorage::size() {
  return sensorList.size();
}

SensorStorage sensorList = SensorStorage();