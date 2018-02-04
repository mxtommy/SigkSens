#ifndef _ads1115_H_
#define _ads1115_H_

extern "C" {
#include "user_interface.h"
}

void setupADS1115();
void handleADS1115();

uint32_t getUpdateADSDelay();
void setADSUpdateDelay(uint32_t newDelay);

#endif
