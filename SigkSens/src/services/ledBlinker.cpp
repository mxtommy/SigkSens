#include "../../config.h"
#include "../../sigksens.h"
#include "ledBlinker.h"

LedBlinker::LedBlinker() {
  pinMode(LED_PIN, OUTPUT);
  this->setState(0);
}

void LedBlinker::removeBlinkerIfNotNull() {
  if (this->blinker != nullptr) {
    this->blinker->remove();
  }
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
  this->removeBlinkerIfNotNull();
  this->blinker = app.onRepeat(1000, [] () {
    ledBlinker.setState(1);
    app.onDelay(100, [] () {
      ledBlinker.setState(0);
    });
  });
}

void LedBlinker::setWifiConnected() {
  this->removeBlinkerIfNotNull();
  this->blinker = app.onRepeat(1000, [] () {
    ledBlinker.flip();
  });
}

void LedBlinker::setServerConnected() {
  this->removeBlinkerIfNotNull();
  this->blinker = app.onRepeat(200, [] () {
    ledBlinker.flip();
  });
}

LedBlinker ledBlinker;