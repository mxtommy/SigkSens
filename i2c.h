#ifndef _i2c_H_
#define _i2c_H_

#include "config.h"

void setupI2C(bool&);
void handleI2C(bool&);
void handleI2C_slow(bool&);

#ifdef ENABLE_SHT30
bool getSensorSHT30Present();
#endif

#ifdef ENABLE_MPU
bool getSensorMPU925XPresent();
#endif

#ifdef ENABLE_BMP280
bool getSensorBMP280Present();
#endif

#ifdef ENABLE_ADS1115
bool getSensorADS1115Present();
#endif



#endif
