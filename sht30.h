#ifndef _sht30_H_
#define _sht30_H_

extern "C" {
#include "user_interface.h"
}

void setupSHT30();
void handleSHT30();

uint32_t getSensorSHTReadDelay();
void setSHTReadDelay(uint32_t newDelay);

#endif
