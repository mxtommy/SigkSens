#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif


#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "bilgemon.h"


//ComponentBilgeMonitor componentBilgeMonitor("S1", 26, 1);
//ComponentBilgeMonitor componentBilgeMonitor("S2", 14,2);

ComponentBilgeMonitor::ComponentBilgeMonitor(const char * name, uint8_t inputPin) : ComponentSensor(name) {
 
  _inputPin = inputPin;
  _runCount = 0;
  _runTime = 0;
}


void ComponentBilgeMonitor::setupComponent() {

  pinMode(_inputPin, INPUT);

  //sets default if not already defined
  config.getBool("enabled", false);
  config.getString("pathRunCount",    "electrical.bilge." + componentName + ".runCount");
  config.getString("pathRunTime",  "electrical.bilge." + componentName + ".runTime");



  app.onRepeat(100, [this]() { this->handleComponent(); });

}

void ComponentBilgeMonitor::handleComponent() {
  config.handle(); //saves config if any changes


}