#ifndef _i2c_H_
#define _i2c_H_

extern "C" {
#include "user_interface.h"
}

#include <Wire.h>

#include "sigksens.h"

#include "config.h"

void setupI2C(bool&);
void handleI2C(bool&);

#ifdef ENABLE_SHT30
#include "src/sensors/sht30/sht30.h"
bool getSensorSHT30Present();
#endif

#ifdef ENABLE_MPU
#include "src/sensors/mpu9250/mpu9250.h"
bool getSensorMPU925XPresent();
#endif

#ifdef ENABLE_BMP280
#include "bmp280.h"
bool getSensorBMP280Present();
#endif

#ifdef ENABLE_ADS1115
#include "ads1115.h"
bool getSensorADS1115Present();
#endif



  
#endif
