#ifndef _signalK_H_
#define _signalK_H_

#include "config.h"

void setupSignalK();
void handleSignalK();
void receiveDelta(uint8_t * payload);
void sendDelta();

#endif
