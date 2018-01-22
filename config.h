/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

// Signal K maximum path length

#define MAX_SIGNALK_PATH_LEN 150

// Digital input pins

#define D1_PIN 14 // D5 
#define D2_PIN 12 // D6

// One-Wire settings

#define ONE_WIRE_BUS 2   // D7 pin on ESP

// set these together! Precision for OneWire
// 9  is 0.5C in 94ms
// 10 is 0.25C in 187ms
// 11 is 0.125C in 375ms
// 12 is 0.0625C in 750ms
#define TEMPERATURE_PRECISION 10
#define ONEWIRE_READ_DELAY 187

// Button configuration for resetting the device

#define RESET_CONFIG_PIN 0

#define SHORT_BUTTON_PRESS_MS 1000
#define LONG_BUTTON_PRESS_MS 5000

#if __has_include("my-config.h")
#include "my-config.h"
#endif
