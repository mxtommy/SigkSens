#ifndef _oneWire_H_
#define _oneWire_H_

#include "config.h"

extern bool sensorOneWirePresent;

bool setup1Wire(bool&);
void handle1Wire(bool&, bool&);
void setOneWireReadDelay(uint32_t);
uint32_t getOneWireReadDelay();

#endif
