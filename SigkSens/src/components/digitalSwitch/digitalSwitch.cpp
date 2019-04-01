#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "digitalSwitch.h"

ComponentDigitalSwitch componentDigitalSwitch("S1", 26);
//ComponentDigitalSwitch componentDigitalSwitch("S2", 14);

ComponentDigitalSwitch::ComponentDigitalSwitch(const char * name, uint8_t outPin) : ComponentSensor(name) {
  outputPin = outPin;
  state = false;
}


void ComponentDigitalSwitch::setupComponent() {

  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);

  //sets default if not already defined
  config.getBool("enabled", false);
  config.getString("pathState",    "electrical.switch." + componentName + ".state");
  config.getString("pathCurrent",  "electrical.switch." + componentName + ".current");

  signalK.registerCallbackBool("pathState", [this](bool newValue) { this->handleCallback(newValue); });

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