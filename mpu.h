#ifndef _mpu_H_
#define _mpu_H_

extern "C" {
#include "user_interface.h"
}

void setupMPU9250();
void handleMPU9250();

uint32_t getUpdateMPUDelay();
void setMPUUpdateDelay(uint32_t newDelay);

#endif
