extern "C" {
#include "user_interface.h"
}

#include "../../../config.h"

#include "digitalIn.h"

#include "FunctionalInterrupt.h"

DigitalInSensorInfo::DigitalInSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  signalKPath[2] = "";
  attrName[0] = "state";
  attrName[1] = "freq";
  attrName[2] = "count";
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
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][2]["signalKPath"],
    jsonSens["attrs"][0]["offset"],
    jsonSens["attrs"][1]["offset"],
    jsonSens["attrs"][2]["offset"],
    jsonSens["attrs"][0]["scale"],
    jsonSens["attrs"][1]["scale"],
    jsonSens["attrs"][2]["scale"]
  );
}

void DigitalInSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::digitalIn;
  JsonArray& jsonAttrs = jsonSens.createNestedArray("attrs");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    JsonObject& attr = jsonAttrs.createNestedObject();
    attr["name"] = attrName[x];
    attr["signalKPath"] = signalKPath[x];
    attr["offset"] = offset[x];
    attr["scale"] = scale[x];
    attr["value"] = valueJson[x];
  }
}


int               digitalPins[NUMBER_DIGITAL_INPUT] = DIGITAL_INPUT_PINS;
char              digitalPinNames[NUMBER_DIGITAL_INPUT][10] = DIGITAL_INPUT_NAME;
int               digitalPinModes[NUMBER_DIGITAL_INPUT] = DIGITAL_INPUT_MODES;
uint32_t          digitPinLastUpdateState[NUMBER_DIGITAL_INPUT] = { 0 };
uint32_t          digitPinLastUpdatePeriodic[NUMBER_DIGITAL_INPUT] = { 0 };
uint32_t          digitalPinCountLast[NUMBER_DIGITAL_INPUT] = { 0 };
volatile bool     digitalPinStateChange[NUMBER_DIGITAL_INPUT] = { false };
volatile uint32_t digitalPinCount[NUMBER_DIGITAL_INPUT] = { 0 };


void getDigitalPinName(uint8_t index, char *dstCharArr) {
  strcpy(dstCharArr, digitalPinNames[index]);
}

void ICACHE_RAM_ATTR DigitalPinISR(int idx) {
  digitalPinStateChange[idx] = true; 
  digitalPinCount[idx]++;
}

// forward declarations

void updateDigitalInStates();
void updateDigitalInSensorInfo();

void setupDigitalIn(bool &need_save) {
  for (unsigned int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    initializeDigitalPin(index, need_save); 
    attachInterrupt(digitalPins[index], std::bind(DigitalPinISR, index), digitalPinModes[index]);
  }

  app.onRepeat(20, updateDigitalInStates);
  app.onRepeat(SLOW_LOOP_DELAY, updateDigitalInSensorInfo);
}


void updateDigitalInStates() {
  for (unsigned int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    if (digitalPinStateChange[index]) {
      updateDigitalInState(index);
    }
  }
}


void updateDigitalInSensorInfo() {
  //Update Sensorinfo
  for (unsigned int index=0;index<(sizeof(digitalPins)/sizeof(digitalPins[0])); index++) {
    updateDigitalInPeriodic(index);   
  }
}


void initializeDigitalPin(uint8_t index, bool &need_save) {
  pinMode(digitalPins[index], INPUT);
  // Setup "sensors" if not already existing
  bool known = sensorStorage[(int)SensorType::digitalIn].find(
    digitalPinNames[index]) != nullptr;
  if (!known) {
    Serial.print(F("Setting up Digital Input on pin: "));
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
      if (digitalRead(digitalPins[index]) == LOGIC_LEVEL_TRUE) {
        si->valueJson[0] = "true";
      } else {
        si->valueJson[0] = "false";
      }
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
      if (digitalRead(digitalPins[index]) == LOGIC_LEVEL_TRUE) {
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

