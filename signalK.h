#ifndef _signalK_H_
#define _signalK_H_
extern "C" {
#include "user_interface.h"
}
#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson

#include "config.h"
#include "sigksens.h"
#include "webSocket.h"
#include "src/sensors/digitalOut/digitalOut.h"


void setupSignalK();
void handleSignalK();
void receiveDelta(uint8_t * payload);
void sendDelta();

#endif
