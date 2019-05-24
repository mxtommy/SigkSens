#ifdef SIGK_HW_ENGINEMON

#include "../components/oneWireTemp/oneWireTemp.h"

#ifdef ESP32
  ComponentOneWireTemp oneW("oneWire", 15);
#endif

#ifdef ESP8266
  ComponentOneWireTemp oneW("oneWire", D7);
#endif


#endif