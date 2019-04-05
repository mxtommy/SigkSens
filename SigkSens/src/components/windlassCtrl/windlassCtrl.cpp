#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h" // for React app
#include "../../services/signalK.h"
#include "../../services/configStore.h"

#include "windlassCtrl.h"


#define WINDLASS_IDLE_TIME_MS 250 // ms to wait before allowing the windlass to start again after stopping
#define WINDLASS_WATCHDOG_MS 500 // maximum ms to allow windlass to run without getting another "on" before stopping 

ComponentWindlassCtrl::ComponentWindlassCtrl(const char * name, uint8_t upPin, uint8_t downPin) : ComponentSensor(name) {
  channelUpPin = upPin;
  channelDownPin = downPin;
  lockState = true;
}

void ComponentWindlassCtrl::handleCallbackUp(bool newValue) {
  if (lockState) {
    setWindlassIdle();
  }
  if (newValue) {
    setChannelUpActive();
  } else {
    setWindlassIdle();
  }
}

void ComponentWindlassCtrl::handleCallbackDown(bool newValue) {
  if (lockState) {
    setWindlassIdle();
  }  
  if (newValue) {
    setChannelDownActive();
  } else {
    setWindlassIdle();
  }
}

void ComponentWindlassCtrl::handleCallbackLock(bool newValue) {
  lockState = newValue;
}

void ComponentWindlassCtrl::setChannelUpActive() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
      Serial.println("WINDLASS: CHANNEL1 ON");
      digitalWrite(channelUpPin, HIGH);
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

void ComponentWindlassCtrl::setChannelDownActive() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
      Serial.println("WINDLASS: CHANNEL2 ON");
      digitalWrite(channelDownPin, HIGH);
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
  digitalWrite(channelUpPin, LOW);
  digitalWrite(channelDownPin, LOW);
  lastEventTime = millis();
  windlassState = idleWait;
}

void ComponentWindlassCtrl::handleComponent() {
  config.handle(); //saves config if any changes

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

void ComponentWindlassCtrl::sendLockStatus() {
  signalK.addValue(configStore.getString("pathWindlassLock"), lockState);
}


void ComponentWindlassCtrl::setupComponent() {
  //CTRL
  pinMode(channelUpPin, OUTPUT);
  pinMode(channelDownPin, OUTPUT);
  digitalWrite(channelUpPin, LOW); 
  digitalWrite(channelDownPin, LOW);

  //sets default if not already defined :)
  configStore.getString("pathWindlassUp",   "electrical.windlass.up");
  configStore.getString("pathWindlassDown", "electrical.windlass.down");
  configStore.getString("pathWindlassLock", "electrical.windlass.lock");

  signalK.registerCallbackBool(configStore.getString("pathWindlassUp"), [this](bool newValue) { this->handleCallbackUp(newValue); });
  signalK.registerCallbackBool(configStore.getString("pathWindlassDown"), [this](bool newValue) { this->handleCallbackDown(newValue); });
  signalK.registerCallbackBool(configStore.getString("pathWindlassLock"), [this](bool newValue) { this->handleCallbackLock(newValue); });


  app.onRepeat(1, [this]() { this->handleComponent(); });
  app.onRepeat(101, [this]() { this->sendLockStatus(); });

}
