#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <Adafruit_ADS1015.h>
#define SMOOTHING_GAIN 0.2


#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "digitalSwitch.h"

ComponentDigitalSwitch componentDigitalSwitch("S1", 26, 1);
ComponentDigitalSwitch componentDigitalSwitch2("S2", 14,2);

ComponentDigitalSwitch::ComponentDigitalSwitch(const char * name, uint8_t outPin, uint8_t adsPin) : ComponentSensor(name) {
  outputPin = outPin;
  ads1115Pin = adsPin;
  state = false;
}


void ComponentDigitalSwitch::setupComponent() {

  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);

  // setup ads1115
  //ads.setGain(GAIN_ONE);


  //sets default if not already defined
  config.getBool("enabled", false);
  config.getString("pathState",    "electrical.switch." + componentName + ".state");
  config.getString("pathCurrent",  "electrical.switch." + componentName + ".current");

  signalK.registerCallbackBool(config.getString("pathState"), [this](bool newValue) { this->handleCallback(newValue); });

  app.onRepeat(100, [this]() { this->handleComponent(); });
}



void ComponentDigitalSwitch::handleComponent() {
  config.handle(); //saves config if any changes
  if (config.getBool("enabled")) {
    signalK.addValue(config.getString("pathState"), state);
  }
}


void ComponentDigitalSwitch::handleCallback(bool newValue) {
  if (newValue) {
    digitalWrite(outputPin, HIGH);
    state = true;
  } else
  {
    digitalWrite(outputPin, LOW);
    state = false;
  }
}