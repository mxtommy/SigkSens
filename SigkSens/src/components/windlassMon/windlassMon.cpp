#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "config.h"
#include "src/sensors/windlassMon/config.h"
#include "sigksens.h" // for React app
#include "src/services/configStore.h"
#include "src/services/signalK.h"
#include "windlassMon.h"

// forward decleration
void handleWindlassMon();


bool channel1Monitor = false;
bool channel1MonitorLast = false;
bool channel2Monitor = false;
bool channel2MonitorLast = false;

uint32_t channel1ChangeTime = 0;
uint32_t channel2ChangeTime = 0;

uint32_t chainCounterCount = 0;



void handleWindlassMonitor() {
  /*
  if (digitalRead(WINDLASS_STATE_CHANNEL1_PIN) == HIGH) {
    channel1Monitor = true; //active
  } else {
    channel1Monitor = false; //not active
  }

  if (channel1Monitor && !channel1MonitorLast) { 
    //just changed!
    channel1MonitorLast = channel1Monitor;
    channel1ChangeTime = millis();
  }
  */
  signalK.addValue(configStore.getString("pathWindlassUp"),   (bool)digitalRead(WINDLASS_STATE_CHANNEL1_PIN));
  signalK.addValue(configStore.getString("pathWindlassDown"), (bool)digitalRead(WINDLASS_STATE_CHANNEL2_PIN));
}






void setupWindlassMon() {
  //MON
  pinMode(WINDLASS_STATE_CHANNEL1_PIN, INPUT);
  pinMode(WINDLASS_STATE_CHANNEL2_PIN, INPUT);
  pinMode(WINDLASS_COUNT_PIN, INPUT);

  //sets default if not already defined :)
  configStore.getString("pathWindlassUp",   "electrical.windlass.up");
  configStore.getString("pathWindlassDown", "electrical.windlass.down");

  app.onRepeat(1000, handleWindlassMonitor);
}
