#ifndef _i2c_H_
#define _i2c_H_

#include "config.h"

void setupI2C(bool&);
void handleI2C(bool&);
void handleI2C_slow();

#ifdef ENABLE_SHT30
bool getSensorSHT30Present();
#endif
bool getSensorMPU925XPresent();

#endif
