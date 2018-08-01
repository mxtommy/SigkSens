#include <Arduino.h>
#include <WString.h>

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
  sensorMap[sens->address] = sens;
}

SensorInfo* SensorStorage::find(String addr) {
  return sensorMap[addr];
}

SensorStorage sensorStorage[(int)SensorType::SensorType_MAX+1];
