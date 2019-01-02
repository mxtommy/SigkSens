#ifndef _signalK_H_
#define _signalK_H_
#include <map>

void setupSignalK();
void handleSignalK();

void sendDelta();

class SignalK {
  protected:
    std::map<String, String> _mapValues; // values that need to be sent
    std::map<String, void (*)(bool)> _mapCallbackBool; // map of key (representing path in configStore, to callback if received delta matches that path)
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

    void registerCallbackBool(String key, void (* CallbackFunction)(bool));

    void sendDeltas();
    void receiveDelta(uint8_t * payload);

    void requestAuth();

    void handle();
};


extern SignalK signalK;



#endif
