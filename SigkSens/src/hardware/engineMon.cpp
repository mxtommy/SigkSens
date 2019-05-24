#ifdef SIGK_HW_ENGINEMON

#include "../components/oneWireTemp/oneWireTemp.h"
#include "../components/sht30/sht30.h"

#ifdef ESP32
  ComponentOneWireTemp oneW("oneWire", 15);
#endif

#ifdef ESP8266
  ComponentOneWireTemp oneW("oneWire", D7);
#endif

ComponentSHT30 sht30("sht30", 0x45);

#endif