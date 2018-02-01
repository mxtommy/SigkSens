#include "sigksens.h"

LinkedList<SensorInfo*> sensorList = LinkedList<SensorInfo*>();

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

// SensorInfo

bool SensorInfo::isSerializable() {
    // default SensorInfo object can't serialize itself
    return false;
}
