extern "C" {
#include "user_interface.h"
}

#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads;

#include "sigksens.h"
#include "ads1115.h"


ADSSensorInfo::ADSSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  attrName[0] = "diff0_1";
  attrName[1] = "diff2_3";
  type = SensorType::ads1115;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = 0;
  offset[1] = 0;
  scale[0] = 0;
  scale[1] = 0;
  isUpdated = false;
}

ADSSensorInfo::ADSSensorInfo(String addr, String path1, String path2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "diff0_1";
  attrName[1] = "diff2_3";
  type = SensorType::ads1115;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = 0;
  offset[1] = 0;
  scale[0] = 0;
  scale[1] = 0;
  isUpdated = false;
}

ADSSensorInfo::ADSSensorInfo(String addr, String path1, String path2, uint32_t offset1, uint32_t offset2, uint32_t scale1, uint32_t scale2) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  attrName[0] = "diff0_1";
  attrName[1] = "diff2_3";
  type = SensorType::ads1115;
  valueJson[0] = "null";
  valueJson[1] = "null";
  offset[0] = offset1;
  offset[1] = offset2;
  scale[0] = scale1;
  scale[1] = scale2;
  isUpdated = false;
}

ADSSensorInfo *ADSSensorInfo::fromJson(JsonObject &jsonSens) {
  return new ADSSensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0],
    jsonSens["signalKPaths"][1],
    jsonSens["offsets"][0],
    jsonSens["offsets"][1],
    jsonSens["scales"][0],
    jsonSens["scales"][1]
  );
}

void ADSSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::ads1115;
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


/* SensorType info

type="ads1115"
[0] = Diff0_1
[1] = Diff2_3

*/




uint32_t updateReadADSDelay = 20;
bool adsReadyRead = false;
os_timer_t  adsReadTimer; // repeating timer that fires to read ADS


uint32_t updateADSDelay = 1000;
bool adsReadyUpdate = false;
os_timer_t  adsUpdateSensorInfoTimer; // repeating timer that fires ever X/time


//Running values. (we need to keep running value to reduce noise with exponential filter)

float valueDiff_0_1 = 0;
float valueDiff_2_3 = 0;
float multiplier = 1; //default multiplier;

void setupADS1115() {

  //Read ADS
  os_timer_setfn(&adsReadTimer, interruptReadADSS, NULL);
  os_timer_arm(&adsReadTimer, updateReadADSDelay, true);

  // UpdateSensorInfo
  os_timer_setfn(&adsUpdateSensorInfoTimer, interruptUpdateADSSensorInfo, NULL);
  os_timer_arm(&adsUpdateSensorInfoTimer, updateADSDelay, true);

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads.setGain(GAIN_ONE);
  multiplier = 0.125F; /* ADS1115  @ +/- 4.096V gain (16-bit results) */
  
  ads.begin();
  
}


void handleADS1115() {

  if (adsReadyRead) {
    adsReadyRead = false;
    readADS1115();
  }

  if (adsReadyUpdate) {
    adsReadyUpdate = false;
    updateADS1115();
  }
  

}


void interruptUpdateADSSensorInfo(void *pArg) {
  adsReadyUpdate = true;
}


void interruptReadADSS(void *pArg) {
  adsReadyRead = true;
}


uint32_t getUpdateADSDelay() { 
  return updateADSDelay; 
}
uint32_t getReadADSDelay() { 
  return updateReadADSDelay; 
}

void setADSUpdateDelay(uint32_t newDelay) {
  os_timer_disarm(&adsUpdateSensorInfoTimer);
  Serial.print("Restarting ADS Update timer at: ");
  Serial.print(getUpdateADSDelay());  
  Serial.println("ms");
  updateADSDelay = newDelay;
  os_timer_arm(&adsUpdateSensorInfoTimer, updateADSDelay, true);
}

void setADSReadDelay(uint32_t newDelay) {
  os_timer_disarm(&adsReadTimer);
  Serial.print("Restarting ADS Read timer at: ");
  Serial.print(getReadADSDelay());  
  Serial.println("ms");
  updateReadADSDelay = newDelay;
  os_timer_arm(&adsReadTimer, updateReadADSDelay, true);
}


void updateADS1115() {

/*
  SensorInfo *thisSensorInfo;

  digitalUpdateReady[index] = false; // reset update ready
  for (uint8_t i=0; i < sensorList.size(); i++) {
    thisSensorInfo = sensorList.get(i);
    
    if ((thisSensorInfo->type==SensorType::digitalIn) && (strcmp(thisSensorInfo->address, digitalPinNames[index]) == 0) ) {
        if (digitalValue[index]) {
          thisSensorInfo->valueJson[0] = "true";
        } else {
          thisSensorInfo->valueJson[0] = "false";
        }
        thisSensorInfo->valueJson[1] = "null";
        thisSensorInfo->isUpdated = true;
    }    
  }*/
  
  Serial.println(valueDiff_0_1 * multiplier);
}

void readADS1115() {
  int16_t rawResult;
  rawResult = ads.readADC_Differential_0_1(); 
  valueDiff_0_1 = (0.2*rawResult) + ((1-0.2)*valueDiff_0_1);
}


