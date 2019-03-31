#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h" // for React app
#include "../../services/signalK.h"
#include "../../services/configStore.h"

#include "config.h"
#include "windlassCtrl.h"

ComponentWindlassCtrl componentWindlassCtrl("windlassCtrl");

void ComponentWindlassCtrl::handleCallbackUp(bool newValue) {
  if (newValue) {
    setChannel1Active();
  } else {
    setWindlassIdle();
  }
}
void ComponentWindlassCtrl::handleCallbackDown(bool newValue) {
  if (newValue) {
    setChannel2Active();
  } else {
    setWindlassIdle();
  }}


void ComponentWindlassCtrl::setChannel1Active() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
      Serial.println("WINDLASS: CHANNEL1 ON");
      digitalWrite(WINDLASS_OUTPUT_CHANNEL1_PIN, HIGH);
      lastEventTime = millis();
      windlassState = channel1Active;
      break;
    case channel1Active:
      lastEventTime = millis(); // we're resetting watchdog
      break;
    case channel2Active:
      return; // we don't accept when channel2 active
      break;
    case idleWait:
      return; // Still waiting, do nothing
      break;
  }
}

void ComponentWindlassCtrl::setChannel2Active() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
      Serial.println("WINDLASS: CHANNEL2 ON");
      digitalWrite(WINDLASS_OUTPUT_CHANNEL2_PIN, HIGH);
      lastEventTime = millis();
      windlassState = channel2Active;
      break;
    case channel1Active:
      return; // we don't accept when channel2 active
      break;
    case channel2Active:
      lastEventTime = millis(); // we're resetting watchdog
      break;
    case idleWait:
      return; // Still waiting, do nothing
      break;
  }
}



void ComponentWindlassCtrl::setWindlassIdle() {
  Serial.println("WINDLASS: IDLE");
  // turn off both channels just to be safe :)
  digitalWrite(WINDLASS_OUTPUT_CHANNEL1_PIN, LOW);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL2_PIN, LOW);
  lastEventTime = millis();
  windlassState = idleWait;
}

void ComponentWindlassCtrl::handleComponent() {

  switch(windlassState) {
    case idle:
      break;
    case channel1Active:
      if ((lastEventTime + WINDLASS_WATCHDOG_MS) < millis()) {
        setWindlassIdle();
      }
      break;
    case channel2Active:
      if ((lastEventTime + WINDLASS_WATCHDOG_MS) < millis()) {
        setWindlassIdle();
      }    
      break;
    case idleWait:
      // if we've waited long enough, go back to idle
      if ((lastEventTime + WINDLASS_IDLE_TIME_MS) < millis()) {
        windlassState = idle;
      }
      break;
  }

}


void ComponentWindlassCtrl::setupComponent() {
  //CTRL
  pinMode(WINDLASS_OUTPUT_CHANNEL1_PIN, OUTPUT);
  pinMode(WINDLASS_OUTPUT_CHANNEL2_PIN, OUTPUT);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL1_PIN, LOW); 
  digitalWrite(WINDLASS_OUTPUT_CHANNEL2_PIN, LOW);

  //sets default if not already defined :)
  configStore.getString("pathWindlassUp",   "electrical.windlass.up");
  configStore.getString("pathWindlassDown", "electrical.windlass.down");

  signalK.registerCallbackBool("pathWindlassUp", [this](bool newValue) { this->handleCallbackUp(newValue); });
  signalK.registerCallbackBool("pathWindlassDown", [this](bool newValue) { this->handleCallbackDown(newValue); });

  app.onRepeat(1, [this]() { this->handleComponent(); });
}
