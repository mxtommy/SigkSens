#ifndef _mpu9250_H_
#define _mpu9250_H_

#include "config.h"

bool configureMPU9250();
void processMPU9250();
void updateQuaternion();
void updateMPUSensorInfo();

#endif
