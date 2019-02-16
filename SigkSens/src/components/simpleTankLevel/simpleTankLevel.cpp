#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h"
#include "../../services/signalK.h"
#include "simpleTankLevel.h"

#ifdef ENABLE_SIMPLE_TANK_LEVEL
ComponentSimpleTankLevel componentSimpleTank("simpleTankLevel");
#endif

int tankLevelLogicLevel[NUMBER_SIMPLE_TANK_PINS] = SIMPLE_TANK_LOGIC_LEVEL_TRUE;

void ComponentSimpleTankLevel::setupComponent() {
  // set default path
  config.getString("pathSimpleTankLevel", String(SIMPLE_TANK_DEFAULT_PATH));

  //setup pins
  for (uint8_t index=0;index<NUMBER_SIMPLE_TANK_PINS; index++) {
    pinMode(inputPins[index], INPUT);
  }

  app.onRepeat(1000, [this]() { this->handleComponent(); });
}


void ComponentSimpleTankLevel::handleComponent() {
  config.handle(); //saves config if any changes

  //loop through pins. Since in the array they are in order from high to low (tank level) first pin high = the level. 
  for (uint8_t index=0;index<NUMBER_SIMPLE_TANK_PINS; index++) {
     if (digitalRead(inputPins[index]) == tankLevelLogicLevel[index]) {
       signalK.addValue(config.getString("pathSimpleTankLevel"), outputValues[index]);
       return;
     }
  }
  // if we get here none of the pins were high
  signalK.addValue(config.getString("pathSimpleTankLevel"), 0.0);
}