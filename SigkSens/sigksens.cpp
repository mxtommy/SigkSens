#include <Arduino.h>
#include <WString.h>

#include "sigksens.h"


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

String uuidv4() {
  char *hexvalues = "0123456789abcdef";
  String uuid = "";
  for (int i = 0 ; i < 32; i++) {
    if (i == 9) { uuid = uuid + "-"; continue; }
    if (i == 14) { uuid = uuid + "-"; continue; }
    if (i == 19) { uuid = uuid + "-"; continue; }
    if (i == 24) { uuid = uuid + "-"; continue; }
    uuid = uuid + String(hexvalues[random(0,15)]);
  }
  return uuid;
}