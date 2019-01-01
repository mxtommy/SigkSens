#ifndef _signalK_H_
#define _signalK_H_

void setupSignalK();
void handleSignalK();
void receiveDelta(uint8_t * payload);
void sendDelta();

class SignalK {
  protected:
    std::map<String, String> _mapValues;
  public:
    void addValue(String path, String jsonValue);
    void handle();
};


extern SignalK signalK;



#endif
