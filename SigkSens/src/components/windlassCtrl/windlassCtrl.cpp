#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "config.h"
#include "src/sensors/windlassCtrl/config.h"
#include "sigksens.h" // for React app
#include "src/services/configStore.h"
#include "src/services/signalK.h"
#include "windlassCtrl.h"

WindlassStates windlassState = idle;
uint32_t lastEventTime = 0;

//forward declerations
void handleCallbackUp(bool newValue);



void setWindlassChannel1Active() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
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

void setWindlassChannel2Active() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
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



void setWindlassIdle() {
  // turn off both channels just to be safe :)
  digitalWrite(WINDLASS_OUTPUT_CHANNEL1_PIN, LOW);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL2_PIN, LOW);
  lastEventTime = millis();
  windlassState = idleWait;
}

void handleWindlassCtrl() {

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


void setupWindlassCtrl() {
  //CTRL
  pinMode(WINDLASS_OUTPUT_CHANNEL1_PIN, OUTPUT);
  pinMode(WINDLASS_OUTPUT_CHANNEL2_PIN, OUTPUT);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL1_PIN, LOW); 
  digitalWrite(WINDLASS_OUTPUT_CHANNEL2_PIN, LOW);

  //sets default if not already defined :)
  configStore.getString("pathWindlassUp",   "electrical.windlass.up");
  configStore.getString("pathWindlassDown", "electrical.windlass.down");

  signalK.registerCallbackBool("pathWindlassUp", &handleCallbackUp);


}

void handleCallbackUp(bool newValue) {
  Serial.println("OMG....");
}
