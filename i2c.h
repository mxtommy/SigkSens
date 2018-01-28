#ifndef _i2c_H_
#define _i2c_H_

#include "config.h"

void setupI2C(bool&);
void handleI2C();
void handleI2C_slow();

bool getSensorSHT30Present();
bool getSensorMPU925XPresent();

#endif
