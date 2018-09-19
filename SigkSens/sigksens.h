#ifndef _sigksens_H_
#define _sigksens_H_

#include "config.h"
#include <Reactduino.h>
#include <ArduinoJson.h>


extern char myHostname[16];

class LedBlinker {
  private:
    int currentState = 0;
    reaction blinker = INVALID_REACTION;
  protected:
    void setState(int newState);
  public:
    void setWifiConnected();
    void setWifiDisconnected();
    void setServerConnected();
    inline void setServerDisconnected() {
      setWifiConnected();
    }
    void flip();
    LedBlinker();
};

extern LedBlinker ledBlinker;

extern Reactduino app;

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);

#endif
