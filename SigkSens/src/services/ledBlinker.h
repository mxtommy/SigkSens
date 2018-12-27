#ifndef _ledBlinker_H_
#define _ledBlinker_H_

#include <ReactESP.h>


class LedBlinker {
  private:
    int currentState = 0;
    RepeatReaction* blinker = nullptr;
    void removeBlinkerIfNotNull();
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

#endif
