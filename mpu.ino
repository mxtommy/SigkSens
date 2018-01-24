
bool MPUisValid = false;
os_timer_t  mpuUpdateSensorInfo; // repeating timer that fires ever X/time to start temp request cycle
bool mpuUpdateReady = false;

volatile bool newData = false;


void htmlGetCalibration() {
  Serial.println("Aaa");
}


void setMPUUpdateDelay(uint32_t newDelay) {
  os_timer_disarm(&mpuUpdateSensorInfo);
  Serial.print("Restarting MPU polling timer at: ");
  Serial.print(getOneWireReadDelay());  
  Serial.println("ms");
  updateMPUDelay = newDelay;
  os_timer_arm(&mpuUpdateSensorInfo, updateMPUDelay, true);
}


void ICACHE_RAM_ATTR interuptMPUNewData()
{
  newData = true;
}



void setupMPU9250() {

  configureMPU9250();
  os_timer_setfn(&mpuUpdateSensorInfo, interuptMPUSensorInfo, NULL);
  os_timer_arm(&mpuUpdateSensorInfo, updateMPUDelay, true);
  attachInterrupt(12, interuptMPUNewData, RISING); // define interrupt for INT pin output of MPU9250

}




void handleMPU9250() {

  if (!MPUisValid) {
    return;
  }

  //if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {  //If there's new data
  if(newData) {
    newData = false; // reset newData flag
    processMPU9250();
  }

  updateQuaternion();


  if (mpuUpdateReady) {
    // reset interupt
    mpuUpdateReady = false;
    updateMPUSensorInfo();
  }


  
}


void interuptMPUSensorInfo(void *pArg) {
  mpuUpdateReady = true;
}


