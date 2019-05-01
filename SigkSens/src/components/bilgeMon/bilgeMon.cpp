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
  _currentState = false;
  _oldRunTime = 0;
  _currentRunStartTime = 0;

}


void ComponentBilgeMonitor::setupComponent() {

  pinMode(_inputPin, INPUT);

  //sets default if not already defined
  config.getBool("enabled", true);
  config.getString("pathRunCount",    "electrical.bilge." + componentName + ".runCount");
  config.getString("pathRunTime",  "electrical.bilge." + componentName + ".runTime");
  config.getString("pathState", "electrical.bilge." + componentName + ".state");
  _runTime = config.getUInt32("runTime", 0);
  _runCount = config.getUInt32("runCount", 0);


  app.onRepeat(200, [this]() { this->handleComponent(); });

}

void ComponentBilgeMonitor::handleComponent() {
  config.handle(); //saves config if any changes
  if (!config.getBool("enabled")) {
    return;
  }
  if (digitalRead(_inputPin)) {
    // Bilge running
    if (!_currentState) {
      //new running state
      _runCount++;
      _currentState = true;

      //to update time while running we increment _runTime. But due to timing it may be inacurate. 
      //To stay accurate we note start/end time, and reset _runTime to known good
      _oldRunTime = _runTime;
      _currentRunStartTime = millis();
    }
    _runTime = _runTime + 200; //100ms
  } else {
    // Bilge not running
    if (_currentState) {
      //we're stopping...
      uint32_t runTime = millis() - _currentRunStartTime;
      _runTime = _oldRunTime + runTime;
      _currentState = false;
      config.putUInt32("runTime", _runTime);
      config.putUInt32("runCount", _runCount);
    }
  }

  signalK.addValue(config.getString("pathState"), _currentState);
  signalK.addValue(config.getString("pathRunTime"), _runTime/1000);
  signalK.addValue(config.getString("pathRunCount"), _runCount);
}