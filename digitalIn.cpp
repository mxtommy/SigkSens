
extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"
#include "digitalIn.h"

DigitalInSensorInfo::DigitalInSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  signalKPath[2] = "";
  attrName[0] = "state";
  attrName[1] = "freq";
  attrName[1] = "count";
  type = SensorType::digitalIn;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  offset[0] = 0;
  offset[1] = 0;
  offset[2] = 0;
  scale[0] = 1;
  scale[1] = 1;
  scale[2] = 1;

  isUpdated = false;
}

DigitalInSensorInfo::DigitalInSensorInfo(String addr, String path1, String path2, String path3,
                                         float offset0, float offset1, float offset2,
                                         float scale0, float scale1, float scale2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  signalKPath[2] = path3;
  attrName[0] = "state";
  attrName[1] = "freq";
  attrName[2] = "count";
  type = SensorType::digitalIn;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  offset[0] = offset0;
  offset[1] = offset1;
  offset[2] = offset2;
  scale[0] = scale0;
  scale[1] = scale1;
  scale[2] = scale2;

  isUpdated = false;
}

DigitalInSensorInfo *DigitalInSensorInfo::fromJson(JsonObject &jsonSens) {
  return new DigitalInSensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0],
    jsonSens["signalKPaths"][1],
    jsonSens["signalKPaths"][2],
    jsonSens["offsets"][0],
    jsonSens["offsets"][1],
    jsonSens["offsets"][2],
    jsonSens["scales"][0],
    jsonSens["scales"][1],
    jsonSens["scales"][2]
  );
}

void DigitalInSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::digitalIn;
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  JsonArray& jsonOffsets = jsonSens.createNestedArray("offsets");
  JsonArray& jsonScales = jsonSens.createNestedArray("scales");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonPaths.add(signalKPath[x]);
    jsonOffsets.add(offset[x]);
    jsonScales.add(scale[x]);
  }
}


int               digitalPins[NUMBER_DIGITAL_INPUT] = DIGITAL_INPUT_PINS;
char              digitalPinNames[NUMBER_DIGITAL_INPUT][10] = DIGITAL_INPUT_NAME;
uint32_t          digitPinLastUpdateState[NUMBER_DIGITAL_INPUT] = { 0 };
uint32_t          digitPinLastUpdatePeriodic[NUMBER_DIGITAL_INPUT] = { 0 };
uint32_t          digitalPinCountLast[NUMBER_DIGITAL_INPUT] = { 0 };
bool              digitalUpdateReady[NUMBER_DIGITAL_INPUT] = { false };
volatile bool     digitalPinStateChange[NUMBER_DIGITAL_INPUT] = { false };
volatile uint32_t digitalPinCount[NUMBER_DIGITAL_INPUT] = { 0 };


//Timers
uint32_t updateDigitalInDelay = 1000;

os_timer_t  digitalInTimer; // timer to update signalk
bool periodicUpdateReady = false;

// getters

uint32_t getUpdateDigitalInDelay() { return updateDigitalInDelay; }

void getDigitalPinName(uint8_t index, char *dstCharArr) {
  strcpy(dstCharArr, digitalPinNames[index]);
}


void interruptUpdateDigitalIn(void *pArg) {
  periodicUpdateReady = true;
}

void ICACHE_RAM_ATTR interruptDigitalPin(int pin) {
  digitalPinStateChange[pin] = true; 
  digitalPinCount[pin]++;
}

void setupDigitalIn(bool &need_save) {
  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    initializeDigitalPin(index, need_save); 
  }

  //configure interupts pins. would  be awesome to do this dynamically...
  #if (NUMBER_DIGITAL_INPUT > 0)
  attachInterrupt(digitalPins[0], []() {interruptDigitalPin(0);}, CHANGE);
  #endif
  #if (NUMBER_DIGITAL_INPUT > 1)
  attachInterrupt(digitalPins[1], []() {interruptDigitalPin(1);}, CHANGE);
  #endif
  #if (NUMBER_DIGITAL_INPUT > 2)
  attachInterrupt(digitalPins[2], []() {interruptDigitalPin(2);}, CHANGE);
  #endif
  #if (NUMBER_DIGITAL_INPUT > 3)
  attachInterrupt(digitalPins[3], []() {interruptDigitalPin(3);}, CHANGE);
  #endif
  #if (NUMBER_DIGITAL_INPUT > 4)
  attachInterrupt(digitalPins[4], []() {interruptDigitalPin(4);}, CHANGE);
  #endif
  #if (NUMBER_DIGITAL_INPUT > 5)
  attachInterrupt(digitalPins[5], []() {interruptDigitalPin(5);}, CHANGE);
  #endif

  os_timer_setfn(&digitalInTimer, interruptUpdateDigitalIn, NULL);
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);  
}


void handleDigitalIn() {

  //Check if periodic update ready
  if (periodicUpdateReady) {
    periodicUpdateReady = false;
    for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
      digitalUpdateReady[index] = true; //set them all to true  
    }
  }

  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    if (digitalPinStateChange[index]) {
      updateDigitalInState(index);
    }
  }

  //Update Sensorinfo
  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    if (digitalUpdateReady[index]) {
      digitalUpdateReady[index] = false; // reset update ready
      updateDigitalInPeriodic(index);
      
    }
  }
}


void initializeDigitalPin(uint8_t index, bool &need_save) {
  SensorInfo *tmpSensorInfo;

  pinMode(digitalPins[index], INPUT);

  // Setup "sensors" if not already existing
  bool known = sensorStorage[(int)SensorType::digitalIn].find(
    digitalPinNames[index]) != nullptr;
  if (!known) {
    Serial.print("Setting up Digital Input on pin: ");
    Serial.println(digitalPinNames[index]);
    DigitalInSensorInfo *newSensor = new DigitalInSensorInfo(
      digitalPinNames[index]
    );
    sensorStorage[(int)newSensor->type].add(newSensor);         
    need_save = true;
  }      

}

void updateDigitalInState(uint8_t index) {
  SensorInfo *si;
  uint32_t timeNow = micros();
  uint32_t delta;


  if (timeNow < digitPinLastUpdateState[index]) { //protection against wrap around
    digitPinLastUpdateState[index] = timeNow;
    return; //skip this update...
  }
  delta = timeNow - digitPinLastUpdateState[index];
  if (delta < 200000) {
    return; //skip this update... don't send more than once every 20ms (10/sec)
  }      
  //reset flags
  digitPinLastUpdateState[index] = timeNow;
  digitalPinStateChange[index] = false; // reset update ready
  
  si = sensorStorage[(int)SensorType::digitalIn].find(
    digitalPinNames[index]
  );

  if (si != nullptr) {
    //current state
    if (strcmp(si->signalKPath[0].c_str(), "") != 0) {
      if (digitalRead(digitalPins[index]) == LOW) {
        si->valueJson[0] = "true";
      } else {
        si->valueJson[0] = "false";
      }
      si->isUpdated = true; 
    }
  }
}


void updateDigitalInPeriodic(uint8_t index) {
  uint32_t timeNow = micros();
  uint32_t delta;
  SensorInfo *si;
  float rawHz;
    
  si = sensorStorage[(int)SensorType::digitalIn].find(
    digitalPinNames[index]
  );

  if (si != nullptr) {
    //current state
    if (strcmp(si->signalKPath[0].c_str(), "") != 0) {
      if (digitalRead(digitalPins[index]) == LOW) {
        si->valueJson[0] = "true";
      } else {
        si->valueJson[0] = "false";
      }
      si->isUpdated = true; 
    }
    //Hz (pulse/time)
    if (strcmp(si->signalKPath[1].c_str(), "") != 0) {
      if (timeNow < digitPinLastUpdatePeriodic[index]) { //protection against wrap around
        digitPinLastUpdatePeriodic[index] = timeNow;
        return; //skip this update...
      }

      delta = timeNow - digitPinLastUpdatePeriodic[index];
      rawHz = (float)(((digitalPinCount[index]-digitalPinCountLast[index])*1000000)/delta)/2;  // divide by 2 because interupt is on change (both rise and fall)

      si->valueJson[1] = (rawHz * si->scale[1]) + si->offset[1];
      si->isUpdated = true; 
      digitalPinCountLast[index] = digitalPinCount[index];
      digitPinLastUpdatePeriodic[index] = timeNow;
    }       
    //count
    if (strcmp(si->signalKPath[2].c_str(), "") != 0) {

      si->valueJson[2] = ((digitalPinCount[index]/2) * si->scale[2]) + si->offset[2];
      si->isUpdated = true;       

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
