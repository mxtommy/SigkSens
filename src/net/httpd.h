#ifndef _httpd_H_
#define _httpd_H_

extern "C" {
#include "user_interface.h"
}

#include "../../config.h"
#include "../../FSConfig.h"
#include "webSocket.h"
#include "../../sigksens.h"
#include "../../configReset.h"


#ifdef ENABLE_I2C
  #include "../../i2c.h"
#endif
#ifdef ENABLE_MPU
  #include "../sensors/mpu9250/mpu9250.h"
#endif
#ifdef ENABLE_ADS1115
  #include "../sensors/ads1115/ads1115.h"
#endif
#ifdef ENABLE_ONEWIRE
  #include "../sensors/oneWire/oneWire.h"
#endif
#ifdef ENABLE_DIGITALIN
  #include "../sensors/digitalIn/digitalIn.h"
#endif
#ifdef ENABLE_ANALOGIN
  #include "../sensors/analogIn/analogIn.h"
#endif

void setupHTTP();
void handleHttp(bool &need_save);
#endif
