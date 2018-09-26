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


LedBlinker::LedBlinker() {
  pinMode(LED_PIN, OUTPUT);
  this->setState(0);
}

void LedBlinker::setState(int newState) {
  this->currentState = newState;
  #ifdef LED_ACTIVE_LOW
  digitalWrite(LED_PIN, !newState);
  #else
  digitalWrite(LED_PIN, newState);
  #endif
}

void LedBlinker::flip() {
  this->setState(!this->currentState);
}

void LedBlinker::setWifiDisconnected() {
  app.free(this->blinker);
  this->blinker = app.repeat(1000, [] () {
    ledBlinker.setState(1);
    app.delay(100, [] () {
      ledBlinker.setState(0);
    });
  });
}

void LedBlinker::setWifiConnected() {
  app.free(this->blinker);
  this->blinker = app.repeat(1000, [] () {
    ledBlinker.flip();
  });
}

void LedBlinker::setServerConnected() {
  app.free(this->blinker);
  this->blinker = app.repeat(200, [] () {
    ledBlinker.flip();
  });
}

LedBlinker ledBlinker;
