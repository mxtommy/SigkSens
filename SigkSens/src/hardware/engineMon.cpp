#ifdef SIGK_HW_ENGINEMON

#include "../components/oneWireTemp/oneWireTemp.h"
#include "../components/sht30/sht30.h"
#include "../components/bilgeMon/bilgeMon.h"

#ifdef ESP32
  ComponentOneWireTemp oneW("oneWire", 15);
  ComponentBilgeMonitor componentBilgeMonitor("pump1", 16, HIGH);
#endif

#ifdef ESP8266
  ComponentOneWireTemp oneW("oneWire", D7);
  ComponentBilgeMonitor componentBilgeMonitor("pump1", D6, LOW);
  ComponentSHT30 sht30("sht30", 0x45);
#endif


#endif