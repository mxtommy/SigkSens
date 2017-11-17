os_timer_t  updateHz; // once request cycle starts, this timer set so we can send when ready
bool readyToUpdateHz = false;

uint32_t systemHzCount = 0, systemHzMs = 0;

void setupSystemHz() {
  os_timer_setfn(&updateHz, interuptSystemHz, NULL);
  os_timer_arm(&updateHz, 1000, true);
  systemHzMs = millis();
}


void handleSystemHz() {
  if (readyToUpdateHz) {
    // reset interupt
    readyToUpdateHz = false;
    updateSystemHz();
  }
  systemHzCount++;
  
}


void interuptSystemHz(void *pArg) {
  readyToUpdateHz = true;
}


void updateSystemHz() {
  uint32_t elapsed = millis() - systemHzMs;
  systemHz = (systemHzCount*1000) / elapsed;
 // Serial.print ("System Hz :");
 // Serial.println (systemHz);


  systemHzCount = 0;
  systemHzMs = millis();
}

