#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h"
#include "../../services/configStore.h"
#include "../../services/signalK.h"
#include "config.h"
#include "simpleTankLevel.h"

uint8_t inputPins[NUMBER_SIMPLE_TANK_PINS] = SIMPLE_TANK_PINS;
float outputValues[NUMBER_SIMPLE_TANK_PINS] = SIMPLE_TANK_LEVEL;

//forward decleration
void updateSimpleTankLevel();

void setupSimpleTankLevel() {
  // set default path
  configStore.getString("pathSimpleTankLevel", String(SIMPLE_TANK_DEFAULT_PATH));

  //setup pins
  for (uint8_t index=0;index<NUMBER_SIMPLE_TANK_PINS; index++) {
    pinMode(inputPins[index], INPUT);
  }

  app.onRepeat(1000, updateSimpleTankLevel);
}


void updateSimpleTankLevel() {
  //loop through pins. Since in the array they are in order from high to low (tank level) first pin high = the level. 
  for (uint8_t index=0;index<NUMBER_SIMPLE_TANK_PINS; index++) {
     if (digitalRead(inputPins[index])) {
       signalK.addValue(configStore.getString("pathSimpleTankLevel"), outputValues[index]);
       return;
     }
  }
  // if we get here none of the pins were high
  signalK.addValue(configStore.getString("pathSimpleTankLevel"), 0.0);
}