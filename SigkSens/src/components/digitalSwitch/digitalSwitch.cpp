#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif


#define SMOOTHING_GAIN 0.2


#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "digitalSwitch.h"

//ComponentDigitalSwitch componentDigitalSwitch("S1", 26, 1);
//ComponentDigitalSwitch componentDigitalSwitch2("S2", 14,2);

ComponentDigitalSwitch::ComponentDigitalSwitch(const char * name, uint8_t outPin, uint8_t adsAddress, uint8_t adsChan) : ComponentSensor(name) {
  ads = Adafruit_ADS1115(adsAddress);
  outputPin = outPin;
  adsChannel = adsChan;
  state = false;
  current = 0;
}


void ComponentDigitalSwitch::setupComponent() {

  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);

  // setup ads1115
  ads.begin();
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



  //sets default if not already defined
  config.getBool("enabled", false);
  config.getString("pathState",    "electrical.switch." + componentName + ".state");
  config.getString("pathCurrent",  "electrical.switch." + componentName + ".current");

  signalK.registerCallbackBool(config.getString("pathState"), [this](bool newValue) { this->handleCallback(newValue); });

  app.onRepeat(100, [this]() { this->handleComponent(); });
}



void ComponentDigitalSwitch::handleComponent() {
  float rawResult;
   
  config.handle(); //saves config if any changes
  if (config.getBool("enabled")) {
    signalK.addValue(config.getString("pathState"), state);
    rawResult = ads.readADC_SingleEnded(adsChannel) * gainMultiplier; // rawresult is now mV measured
    rawResult = ((rawResult/3300)*20000)/1000; //3300 = ohm of sense resistor. 20000 = ration of real current to sense current. /1000 answer in mA, convert to amps
    current = (SMOOTHING_GAIN*rawResult) + ((1-SMOOTHING_GAIN)*current);
    signalK.addValue(config.getString("pathCurrent"), current);
  }
}


void ComponentDigitalSwitch::handleCallback(bool newValue) {
  if (newValue) {
    digitalWrite(outputPin, HIGH);
    state = true;
  } else
  {
    digitalWrite(outputPin, LOW);
    state = false;
  }
}