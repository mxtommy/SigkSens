extern "C" {
#include "user_interface.h"
}

#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads;

#include "sigksens.h"
#include "ads1115.h"

uint32_t updateADSDelay = 1000;
os_timer_t  adsUpdateSensorInfo; // repeating timer that fires ever X/time


void setupADS1115() {

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);           // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  
  ads.begin();
  
}


void handleADS1115() {
  int16_t results;
  float multiplier = 0.125F; /* ADS1115  @ +/- 4.096V gain (16-bit results) */
   results = ads.readADC_Differential_0_1(); 
    Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)");
     delay(1000);
}

uint32_t getUpdateADSDelay() { return updateADSDelay; }


void setADSUpdateDelay(uint32_t newDelay) {
  os_timer_disarm(&adsUpdateSensorInfo);
  Serial.print("Restarting ADS polling timer at: ");
  Serial.print(getUpdateADSDelay());  
  Serial.println("ms");
  updateADSDelay = newDelay;
  os_timer_arm(&adsUpdateSensorInfo, updateADSDelay, true);
}
