#ifndef _signalK_H_
#define _signalK_H_
#include <map>

void setupSignalK();
void handleSignalK();
void receiveDelta(uint8_t * payload);
void sendDelta();

class SignalK {
  protected:
    std::map<String, String> _mapValues;
  public:
    void addValue(String path, String value);
    void addValue(String path, uint8_t value);
    void addValue(String path, uint16_t value);
    void addValue(String path, uint32_t value);
    void addValue(String path, int8_t value);
    void addValue(String path, int16_t value);
    void addValue(String path, int32_t value);
    void addValue(String path, bool value);
    void addValue(String path, float value);
    void addValue(String path, double value);

    void handle();
};


extern SignalK signalK;



#endif
