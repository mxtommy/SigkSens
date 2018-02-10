extern "C" {
#include "user_interface.h"
}

#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads;

#include "sigksens.h"
#include "ads1115.h"

#define SMOOTHING_GAIN 0.2

ADSSensorInfo::ADSSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  signalKPath[2] = "";
  signalKPath[3] = "";
  signalKPath[4] = "";
  signalKPath[5] = "";  
  attrName[0] = "diff0_1";
  attrName[1] = "diff2_3";
  attrName[2] = "chan0";
  attrName[3] = "chan1";
  attrName[4] = "chan2";
  attrName[5] = "chan3";  
  type = SensorType::ads1115;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  valueJson[3] = "null";
  valueJson[4] = "null";
  valueJson[5] = "null";
  offset[0] = 0;
  offset[1] = 0;
  offset[2] = 0;
  offset[3] = 0;
  offset[4] = 0;
  offset[5] = 0;
  scale[0] = 1;
  scale[1] = 1;
  scale[2] = 1;
  scale[3] = 1;
  scale[4] = 1;
  scale[5] = 1;

  isUpdated = false;
}

ADSSensorInfo::ADSSensorInfo( String addr, 
                              String path0, String path1, String path2, String path3, String path4, String path5,
                              float offset0, float offset1, float offset2, float offset3, float offset4, float offset5,
                              float scale0, float scale1, float scale2, float scale3, float scale4, float scale5) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path0;
  signalKPath[1] = path1;
  signalKPath[2] = path2;
  signalKPath[3] = path3;
  signalKPath[4] = path4;
  signalKPath[5] = path5;    
  attrName[0] = "diff0_1";
  attrName[1] = "diff2_3";
  attrName[2] = "chan0";
  attrName[3] = "chan1";
  attrName[4] = "chan2";
  attrName[5] = "chan3";  
  type = SensorType::ads1115;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  valueJson[3] = "null";
  valueJson[4] = "null";
  valueJson[5] = "null";
  offset[0] = offset0;
  offset[1] = offset1;
  offset[2] = offset2;
  offset[3] = offset3;
  offset[4] = offset4;
  offset[5] = offset5;
  scale[0] = scale0;
  scale[1] = scale1;
  scale[2] = scale2;
  scale[3] = scale3;
  scale[4] = scale4;
  scale[5] = scale5;
  
  isUpdated = false;
}

ADSSensorInfo *ADSSensorInfo::fromJson(JsonObject &jsonSens) {
  return new ADSSensorInfo(
    jsonSens["address"],
    jsonSens["signalKPaths"][0],
    jsonSens["signalKPaths"][1],
    jsonSens["signalKPaths"][2],
    jsonSens["signalKPaths"][3],
    jsonSens["signalKPaths"][4],
    jsonSens["signalKPaths"][5],        
    jsonSens["offsets"][0],
    jsonSens["offsets"][1],
    jsonSens["offsets"][2],
    jsonSens["offsets"][3],
    jsonSens["offsets"][4],
    jsonSens["offsets"][5],
    jsonSens["scales"][0],
    jsonSens["scales"][1],
    jsonSens["scales"][2],
    jsonSens["scales"][3],
    jsonSens["scales"][4],
    jsonSens["scales"][5]
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
[2] = Chan0
[3] = Chan1
[4] = Chan2
[5] = chan3

*/




uint32_t updateReadADSDelay = 50;
bool adsReadyRead = false;
os_timer_t  adsReadTimer; // repeating timer that fires to read ADS


uint32_t updateADSDelay = 1000;
bool adsReadyUpdate = false;
os_timer_t  adsUpdateSensorInfoTimer; // repeating timer that fires ever X/time


//Running values. (we need to keep running value to reduce noise with exponential filter)

float valueDiff_0_1 = 0;
float valueDiff_2_3 = 0;
float valueChan0 = 0;
float valueChan1 = 0;
float valueChan2 = 0;
float valueChan3 = 0;
float gainMultiplier = 1; //default multiplier;

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
  gainMultiplier = 0.125F; /* ADS1115  @ +/- 4.096V gain (16-bit results) */
  
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
  Serial.print(newDelay);  
  Serial.println("ms");
  updateADSDelay = newDelay;
  os_timer_arm(&adsUpdateSensorInfoTimer, updateADSDelay, true); 
}

void setADSReadDelay(uint32_t newDelay) {
  os_timer_disarm(&adsReadTimer);
  Serial.print("Restarting ADS Read timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");
  updateReadADSDelay = newDelay;
  os_timer_arm(&adsReadTimer, updateReadADSDelay, true);
}


void updateADS1115() {
  sensorStorage[(int)SensorType::ads1115].forEach([&](SensorInfo* si) {
    //0 = diff0_1
    if (strcmp(si->signalKPath[0].c_str(),  "") != 0) {
      si->valueJson[0] = ( (valueDiff_0_1 * gainMultiplier) * si->scale[0] ) + si->offset[0];
      si->isUpdated = true;         
    }
    //1 = diff2_3
    if (strcmp(si->signalKPath[1].c_str(),  "") != 0) {
      si->valueJson[1] = ( (valueDiff_2_3 * gainMultiplier) * si->scale[1] ) + si->offset[1];
      si->isUpdated = true;         
    }
    //2 = chan0
    if (strcmp(si->signalKPath[2].c_str(),  "") != 0) {
      si->valueJson[2] = ( (valueChan0 * gainMultiplier) * si->scale[2] ) + si->offset[2];
      si->isUpdated = true;         
    }
    //3 = chan1
    if (strcmp(si->signalKPath[3].c_str(),  "") != 0) {
      si->valueJson[3] = ( (valueChan1 * gainMultiplier) * si->scale[3] ) + si->offset[3];
      si->isUpdated = true;         
    }
    //4 = chan2
    if (strcmp(si->signalKPath[4].c_str(),  "") != 0) {
      si->valueJson[4] = ( (valueChan2 * gainMultiplier) * si->scale[4] ) + si->offset[4];
      si->isUpdated = true;         
    }
    //5 = chan3
    if (strcmp(si->signalKPath[5].c_str(),  "") != 0) {
      si->valueJson[5] = ( (valueChan3 * gainMultiplier) * si->scale[5] ) + si->offset[5];
      si->isUpdated = true;         
    }
  });
}

void readADS1115() {
  sensorStorage[(int)SensorType::ads1115].forEach([&](SensorInfo* si){
    int16_t rawResult;
  
    if (si->type != SensorType::ads1115) return;
      
    //0 = diff0_1
    if (strcmp(si->signalKPath[0].c_str(),  "") != 0) {
      rawResult = ads.readADC_Differential_0_1(); 
      valueDiff_0_1 = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*valueDiff_0_1);
    }

    //1 = diff2_3
    if (strcmp(si->signalKPath[1].c_str(),  "") != 0) {
      rawResult = ads.readADC_Differential_2_3(); 
      valueDiff_2_3 = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*valueDiff_2_3);
    }      

    //2 = chan0
    if (strcmp(si->signalKPath[2].c_str(),  "") != 0) {
      rawResult = ads.readADC_SingleEnded(0); 
      valueChan0 = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*valueChan0);
    } 

    //3 = chan1
    if (strcmp(si->signalKPath[3].c_str(),  "") != 0) {
      rawResult = ads.readADC_SingleEnded(1); 
      valueChan1 = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*valueChan1);
    } 

    //4 = chan2
    if (strcmp(si->signalKPath[4].c_str(),  "") != 0) {
      rawResult = ads.readADC_SingleEnded(2); 
      valueChan2 = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*valueChan2);
    } 

    //5 = chan3
    if (strcmp(si->signalKPath[5].c_str(),  "") != 0) {
      rawResult = ads.readADC_SingleEnded(3); 
      valueChan3 = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*valueChan3);
    }
  });
}


