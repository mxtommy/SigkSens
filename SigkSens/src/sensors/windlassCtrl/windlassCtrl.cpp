#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "config.h"

#include "windlassCtrl.h"

WindlassStates windlassState = idle;
uint32_t lastEventTime = 0;

void setWindlassChannel1Active() {
  switch(windlassState) {
    case idle:
      // we're activating the channel!
      digitalWrite(WINDLASS_OUTPUT_CHANNEL1, HIGH);
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
      digitalWrite(WINDLASS_OUTPUT_CHANNEL2, HIGH);
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
  digitalWrite(WINDLASS_OUTPUT_CHANNEL1, LOW);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL2, LOW);
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


void setupWindlassCtrl(bool &need_save) {
  pinMode(WINDLASS_OUTPUT_CHANNEL1, OUTPUT);
  pinMode(WINDLASS_OUTPUT_CHANNEL2, OUTPUT);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL1, LOW);
  digitalWrite(WINDLASS_OUTPUT_CHANNEL2, LOW);
}




