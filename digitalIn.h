#ifndef _digitalIn_H_
#define _digitalIn_H_

#include "config.h"

void setupDigitalIn(bool&);
void handleDigitalIn();
void setDigitalInUpdateDelay(uint32_t newDelay);

uint32_t getUpdateDigitalInDelay();
int getD1Mode();
int getD2Mode();
void setD1Mode(int mode);
void setD2Mode(int mode);

#endif
