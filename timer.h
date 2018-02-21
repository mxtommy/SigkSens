#ifndef _timer_H_
#define _timer_H_

#include "config.h"

#include "sigksens.h"



void setupTimers();
void handleTimers(bool &deltaReady);

uint32_t getDeltaDelay();
void setDeltaDelay(uint32_t newDelay);

#endif