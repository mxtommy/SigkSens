extern "C" {
#include "user_interface.h"
}

#include "config.h"

#include "sigksens.h"

#include "timer.h"

uint32_t deltaDelay = 1000;
os_timer_t deltaTimer;

bool deltaTimerReady = false;

void interruptDeltaDelay(void *pArg) {
  deltaTimerReady = true;
}

void setupTimers() {
  os_timer_setfn(&deltaTimer, interruptDeltaDelay, NULL);
  Serial.print("Starting Delta timer at: ");
  Serial.print(deltaDelay);  
  Serial.println("ms");
  os_timer_arm(&deltaTimer, deltaDelay, true); 
}

void handleTimers(bool &deltaReady) {
    if (deltaTimerReady) {
        deltaTimerReady = false;
        deltaReady = true;
    } 
}



uint32_t getDeltaDelay() {
    return deltaDelay;
}

void setDeltaDelay(uint32_t newDelay) {
  os_timer_disarm(&deltaTimer);
  deltaDelay = newDelay;
  Serial.print("Restarting Delta timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");  
  os_timer_arm(&deltaTimer, deltaDelay, true);  
}