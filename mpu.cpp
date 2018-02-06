extern "C" {
#include "user_interface.h"
}

#include "config.h"

#include "sigksens.h"
#include "mpu9250.h"
#include "mpu.h"

// timer delay
uint32_t updateMPUDelay = 1000;

uint32_t getUpdateMPUDelay() { return updateMPUDelay; }

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
  Serial.print(newDelay);  
  Serial.println("ms");
  updateMPUDelay = newDelay;
  os_timer_arm(&mpuUpdateSensorInfo, updateMPUDelay, true);
}


void ICACHE_RAM_ATTR interruptMPUNewData() {
  newData = true;
}


void interruptMPUSensorInfo(void *pArg) {
  mpuUpdateReady = true;
}


void setupMPU9250() {
  MPUisValid = configureMPU9250();
  os_timer_setfn(&mpuUpdateSensorInfo, interruptMPUSensorInfo, NULL);
  os_timer_arm(&mpuUpdateSensorInfo, updateMPUDelay, true);
  attachInterrupt(12, interruptMPUNewData, RISING); // define interrupt for INT pin output of MPU9250
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
