#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h" // for React app
#include "../../services/configStore.h"
#include "../../services/signalK.h"

#include "windlassMon.h"


// interupts must be global...
volatile bool countFlag = false;
void ICACHE_RAM_ATTR countInterrupt() {
  countFlag = true;
}


ComponentWindlassMon::ComponentWindlassMon(const char * name, uint8_t upPin, uint8_t downPin, uint8_t countPin) : ComponentSensor(name){
  _upPin = upPin;
  _downPin = downPin;
  _countPin = countPin;
  _chainCounterCount = 0;
  _lastCountSave = millis();
}    





void ComponentWindlassMon::handleComponent() {
  config.handle(); //saves config if any changes
  upState = digitalRead(_upPin);
  downState = digitalRead(_downPin);

  //check if count interrupt fired
  if (countFlag) {
    countFlag = false;
    if (upState) {
      _chainCounterCount--; //up= chain pulled in, so getting shorter
    }
    if (downState) {
      _chainCounterCount++;
    }
   
  }

  if (_lastCountSave < (millis() - 120000 )) { //save max every minute
    _lastCountSave = millis();
    if (_chainCounterCount != configStore.getInt16("chainCounterValue")) {
      configStore.putInt16("chainCounterValue", _chainCounterCount);
    }
  }

  signalK.addValue(configStore.getString("pathWindlassUp"), upState);
  signalK.addValue(configStore.getString("pathWindlassDown"), downState);
  signalK.addValue(configStore.getString("pathWindlassCount"), _chainCounterCount*configStore.getFloat("windlassScale"));

}






void ComponentWindlassMon::setupComponent() {
  //MON
  pinMode(_upPin, INPUT);
  pinMode(_downPin, INPUT);
  pinMode(_countPin, INPUT);

  attachInterrupt(_countPin, countInterrupt, RISING);

  //sets default if not already defined :)
  configStore.getString("pathWindlassUp",   "electrical.windlass.up");
  configStore.getString("pathWindlassDown", "electrical.windlass.down");
  configStore.getString("pathWindlassCount", "electrical.windlass.count");
  _chainCounterCount = configStore.getInt16("chainCounterValue", 0);
  configStore.getFloat("windlassScale",1); //for counting pulses
  
  app.onRepeat(100, [this]() { this->handleComponent(); });
}
