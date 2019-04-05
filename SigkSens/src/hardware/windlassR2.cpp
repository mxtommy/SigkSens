#ifdef SIGK_HW_WINDLASSR2

#include "../components/windlassCtrl/windlassCtrl.h"
#include "../components/windlassMon/windlassMon.h"
//Channel up   WROVER pin 11, ESP32 IO26
//Channel down WROVER pin 13, ESP32 IO14
//Monitor Up   WROVER pin 10, ESP32 IO25
//Monitor Down WROVER pin 13, ESP32 IO27
//Monitor Count WROVER pin 16, ESP32 IO13

ComponentWindlassCtrl componentWindlassCtrl("windlassCtrl", 26, 14);
ComponentWindlassMon componentWindlassMon("windlassMon",25,27,13);

#endif