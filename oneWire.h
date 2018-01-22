
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"

bool setup1Wire(bool&);
void handle1Wire(bool&, bool&);
void setOneWireReadDelay(uint32_t);
void getOneWireReadDelay(uint32_t&);