
extern "C" {
#include "user_interface.h"
}

#include "sigksens.h"
#include "digitalIn.h"

DigitalInSensorInfo::DigitalInSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  attrName[0] = "state";
  attrName[1] = "freq";
  type = SensorType::digitalIn;
  valueJson[0] = "null";
  valueJson[1] = "null";

  isUpdated = false;
}

DigitalInSensorInfo::DigitalInSensorInfo(String addr, 
                                         String path1, 
                                         String path2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "state";
  attrName[1] = "freq";
  type = SensorType::digitalIn;
  valueJson[0] = "null";
  valueJson[1] = "null";

  isUpdated = false;
}

DigitalInSensorInfo *DigitalInSensorInfo::fromJson(JsonObject &jsonSens) {
  return new DigitalInSensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0],
    jsonSens["signalKPaths"][1]
  );
}

void DigitalInSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::digitalIn;
  JsonArray& jsonPaths = jsonSens.createNestedArray("signalKPaths");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    jsonPaths.add(signalKPath[x]);
  }
}


int     digitalPins[NUMBER_DIGITAL_INPUT] = DIGITAL_INPUT_PINS;
char    digitalPinNames[NUMBER_DIGITAL_INPUT][10] = DIGITAL_INPUT_NAME;
bool    digitalUpdateReady[NUMBER_DIGITAL_INPUT] = { false };
uint32_t digitalPinCount[NUMBER_DIGITAL_INPUT] = { 0 };

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

void ICACHE_RAM_ATTR interruptDigitalPin0() {
  digitalUpdateReady[0] = true; 
  digitalPinCount[0]++;
}

void ICACHE_RAM_ATTR interruptDigitalPin1() {
  digitalUpdateReady[1] = true; 
  digitalPinCount[1]++;
}

void ICACHE_RAM_ATTR interruptDigitalPin2() {
  digitalUpdateReady[2] = true; 
  digitalPinCount[2]++;
}

void ICACHE_RAM_ATTR interruptDigitalPin3() {
  digitalUpdateReady[3] = true; 
  digitalPinCount[3]++;
}

void ICACHE_RAM_ATTR interruptDigitalPin4() {
  digitalUpdateReady[4] = true; 
  digitalPinCount[4]++;
}

void ICACHE_RAM_ATTR interruptDigitalPin5() {
  digitalUpdateReady[5] = true; 
  digitalPinCount[5]++;
}

void setupDigitalIn(bool &need_save) {
  for (int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    initializeDigitalPin(index, need_save); 
  }

  //configure interupts pins. would  be awesome to do this dynamically...
  if (NUMBER_DIGITAL_INPUT >= 1) { attachInterrupt(digitalPins[0], interruptDigitalPin0, CHANGE); }
  if (NUMBER_DIGITAL_INPUT >= 2) { attachInterrupt(digitalPins[1], interruptDigitalPin1, CHANGE); }
  if (NUMBER_DIGITAL_INPUT >= 3) { attachInterrupt(digitalPins[2], interruptDigitalPin2, CHANGE); }
  if (NUMBER_DIGITAL_INPUT >= 4) { attachInterrupt(digitalPins[3], interruptDigitalPin3, CHANGE); }
  if (NUMBER_DIGITAL_INPUT >= 5) { attachInterrupt(digitalPins[4], interruptDigitalPin4, CHANGE); }
  if (NUMBER_DIGITAL_INPUT >= 6) { attachInterrupt(digitalPins[5], interruptDigitalPin5, CHANGE); }

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




void updateDigitalIn(uint8_t index) {
  SensorInfo *thisSensorInfo;

  digitalUpdateReady[index] = false; // reset update ready

  thisSensorInfo = sensorStorage[(int)SensorType::digitalIn].find(
    digitalPinNames[index]
  );

  if (thisSensorInfo != nullptr) {
    if (digitalRead(digitalPins[index]) == LOW) {
      thisSensorInfo->valueJson[0] = "true";
    } else {
      thisSensorInfo->valueJson[0] = "false";
    }
    thisSensorInfo->valueJson[1] = "null";
    thisSensorInfo->isUpdated = true;    
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
