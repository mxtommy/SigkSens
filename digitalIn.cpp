
extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"
#include "digitalIn.h"

int     digitalPins[NUMBER_DIGITAL_INPUT] = DIGITAL_INPUT_PINS;
char    digitalPinNames[NUMBER_DIGITAL_INPUT][10] = DIGITAL_INPUT_NAME;
bool    digitalValueLast[NUMBER_DIGITAL_INPUT] = { false };
bool    digitalValue[NUMBER_DIGITAL_INPUT] = { false };
uint8_t digitalMode[NUMBER_DIGITAL_INPUT] = { DIGITAL_MODE_OFF };
bool    digitalUpdateReady[NUMBER_DIGITAL_INPUT] = { false };


//Timers
uint32_t updateDigitalInDelay = 1000;

os_timer_t  digitalInTimer; // timer to update signalk
bool periodicUpdateReady = false;

// getters

uint32_t getUpdateDigitalInDelay() { return updateDigitalInDelay; }

uint8_t getDigitalMode(uint8_t index) {
  return digitalMode[index];
}

void getDigitalPinName(uint8_t index, char *dstCharArr) {
  strcpy(dstCharArr, digitalPinNames[index]);
}

// setters

bool setDigitalMode(const char *nameCharArr, uint8_t mode) { 
  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    if (strcmp(nameCharArr, digitalPinNames[index]) == 0) {
      digitalMode[index] = mode;
      return true;
    }
  }
  // if we got here we did not find a match to name
  return false;
}

void interruptUpdateDigitalIn(void *pArg) {
  periodicUpdateReady = true;
}


void setupDigitalIn(bool &need_save) {

  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    initializeDigitalPin(index, need_save); 
  }

  os_timer_setfn(&digitalInTimer, interruptUpdateDigitalIn, NULL);
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);  
}


void handleDigitalIn() {

  //Check Pins
  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    // State
    if (digitalMode[index] == DIGITAL_MODE_IN_STATE) {
      digitalCheckState(index);
    }
  }

  //Check if periodic update ready
  if (periodicUpdateReady) {
    periodicUpdateReady = false;
    for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
      digitalUpdateReady[index] = true; //set them all to true
    }
  }

  //Update Sensorinfo
  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    if (digitalUpdateReady[index]) {
      updateDigitalIn(index);
    }
  }
}


void initializeDigitalPin(uint8_t index, bool &need_save) {
  SensorInfo *tmpSensorInfo;

  pinMode(digitalPins[index], INPUT);

  // Setup "sensors" if not already existing
  bool known = false;
  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (strcmp(tmpSensorInfo->type, "digitalIn") == 0) {
      if (strcmp(tmpSensorInfo->address, digitalPinNames[index]) == 0) {
        known = true;                
      }
    }

  }    
  if (!known) {
    Serial.print("Setting up Digital Input on pin: ");
    Serial.println(digitalPinNames[index]);
    SensorInfo *newSensor = new SensorInfo();
    strcpy(newSensor->address, digitalPinNames[index]);
    strcpy(newSensor->type,"digitalIn");
    newSensor->attrName[0] = "state";
    newSensor->attrName[1] = "freq";
    newSensor->signalKPath[0] = "";
    newSensor->signalKPath[1] = "";
    newSensor->valueJson[0] = "null";
    newSensor->valueJson[1] = "null";
    newSensor->isUpdated = false;
    sensorList.add(newSensor);         
    need_save = true;
  }      

  // take initial read.
  if (digitalRead(digitalPins[index]) == LOW) {
    digitalValueLast[index] = true;
    digitalValue[index] = true;
  }
  
}




void digitalCheckState(uint8_t index) {
  if (digitalRead(digitalPins[index]) == LOW) {
    digitalValue[index] = true;
  } else {
    digitalValue[index] = false; 
  }
  if (digitalValue[index] && !digitalValueLast[index]) { 
    //just activated!
    digitalValueLast[index] = digitalValue[index];
    digitalUpdateReady[index] = true;
  }
  if (!digitalValue[index] && digitalValueLast[index]) { 
    //just deactivated!
    digitalValueLast[index] = digitalValue[index];
    digitalUpdateReady[index] = true;
  }
}




void updateDigitalIn(uint8_t index) {
  SensorInfo *thisSensorInfo;

  digitalUpdateReady[index] = false; // reset update ready
  for (uint8_t i=0; i < sensorList.size(); i++) {
    thisSensorInfo = sensorList.get(i);
    
    if ((strcmp(thisSensorInfo->type, "digitalIn") == 0) && (strcmp(thisSensorInfo->address, digitalPinNames[index]) == 0) ) {
        if (digitalValue[index]) {
          thisSensorInfo->valueJson[0] = "true";
        } else {
          thisSensorInfo->valueJson[0] = "false";
        }
        thisSensorInfo->valueJson[1] = "null";
        thisSensorInfo->isUpdated = true;
    }
       
  }

}

void setDigitalInUpdateDelay(uint32_t newDelay) {
  os_timer_disarm(&digitalInTimer);
  Serial.print("Restarting DigitalIn polling timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");
  updateDigitalInDelay = newDelay;
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);
}



