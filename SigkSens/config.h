#ifndef _config_H_
#define _config_H_

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Feature selection
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

// Print Deltas to Serial? when enabled deltas are also printed to Serial, to 
// aid in debuging. If you do not want this comment this line out.
 #define ENABLE_SERIAL_DELTA


// To enable any of the features below, either uncomment out the row
// or add #define <feat> to my_config.h and enable my_config.h at the
// end of this file.

// Sensors

#define ENABLE_SYSTEMHZ
//#define ENABLE_DIGITALIN
//#define ENABLE_DIGITALOUT
//#define ENABLE_ANALOGIN
//#define ENABLE_ONEWIRE


// if any of these are enabled, make sure ENABLE_I2C below is as well.
//#define ENABLE_I2C
//#define ENABLE_SHT30
//#define ENABLE_MPU
//#define ENABLE_BMP280
//#define ENABLE_BME280
//#define ENABLE_ADS1115
//#define ENABLE_INA219

// Services

#define ENABLE_WEBSOCKET_SERVER

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Defines
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

// Wifi config portal timeout (seconds). The smaller the value, the faster
// the device will attempt to reconnect. If set too small, it might
// become impossible to actually configure the Wifi settings in the captive
// portal.
#define WIFI_CONFIG_PORTAL_TIMEOUT 180

// HTTP port for the configuration interface
#define HTTP_PORT 80

// Password for Over-the-air (OTA) updates
#define OTA_PASSWORD "bonvoyage"

// Slow loop duration (SK updates, slow sensor reads etc)
#define SLOW_LOOP_DELAY 200

// Signal K maximum path length

#define MAX_SIGNALK_PATH_LEN 150

// LED pin

#define LED_PIN LED_BUILTIN  // If your board doesn't have a defined LED_BUILTIN, comment this line out...
// #define LED_PIN 13        // ... and uncomment this line, and change 13 to any LED pin you have. If you have none, leave it at 13.
#define LED_ACTIVE_LOW true

// Digital input pins

#define LOGIC_LEVEL_TRUE HIGH // if HIGH the inputs will be true on Vcc. If LOW it will be true on gnd.
#define NUMBER_DIGITAL_INPUT 2  // set to number of elements in next three arrays
#define DIGITAL_INPUT_PINS { 14, 12 } 
#define DIGITAL_INPUT_NAME {"D1", "D2"}
#define DIGITAL_INPUT_MODES {CHANGE, CHANGE}  // one of CHANGE, RISING, FALLING

// Digital output Pins
#define NUMBER_DIGITAL_OUTPUT 2  // set to number of elements in next two arrays...s
#define DIGITAL_OUTPUT_PINS { 16, 15 }  // D0, D8 of Wemos)
#define DIGITAL_OUTPUT_NAME {"OUT1", "OUT2"}

// For the BME280
// 1013.25 is "standard pressure", but sea level pressure changes constantly, so the Altitude measurement is
// unlikely to ever be very accurate.
#define SEALEVELPRESSURE_HPA (1013.25)

// ADS1115 read interval
#define ADS1115_READ_INTERVAL 50

// One-Wire settings

#define ONE_WIRE_BUS 13   // D7 pin on Wemos D1 Mini

// set these together! Precision for OneWire
// 9  is 0.5C in 94ms
// 10 is 0.25C in 187ms
// 11 is 0.125C in 375ms
// 12 is 0.0625C in 750ms
#define TEMPERATURE_PRECISION 10
#define ONEWIRE_READ_DELAY 187

// MPU9250 uses interrupts for performance. The interrupt
// pin is defined below:

#define MPU_INTERRUPT_PIN 12 // D6 pin on Wemos D1 Mini

// INA219 shunt setup

#define SHUNT_AMP    200  // max ampere for shunt
#define SHUNT_MV     75   // mV output at max amp
#define MAX_VOLT     14.4 // max bus voltage (alternator/charge max)
#define MAX_AMP      150  // max amp to be measured over shunt

// Button configuration for resetting the device

#define RESET_CONFIG_PIN 0

#define SHORT_BUTTON_PRESS_MS 1000
#define LONG_BUTTON_PRESS_MS 5000

// if you want to use the optional my_config.h file
// to override configuration constants, 
// uncomment the following line.

// #include "my_config.h"

#endif
