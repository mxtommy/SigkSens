
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2   // D7 pin on ESP

// set these together! Precision for OneWire
// 9  is 0.5C in 94ms
// 10 is 0.25C in 187ms
// 11 is 0.125C in 375ms
// 12 is 0.0625C in 750ms
#define TEMPERATURE_PRECISION 10
#define ONEWIRE_READ_DELAY 187


bool setup1Wire(bool&);
void handle1Wire(bool&, bool&);
void setOneWireReadDelay(uint32_t);
void getOneWireReadDelay(uint32_t&);