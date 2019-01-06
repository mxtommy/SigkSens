#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h"
#include "../../services/configStore.h"
#include "../../services/signalK.h"
#include "systemHz.h"

uint32_t systemHzCount = 0, systemHzMs = 0;

float systemHz = 0;

// forward declarations
void countSystemHz();
void updateSystemHz();


void setupSystemHz() {
  systemHzMs = millis();
  
  //sets default if not already defined
  configStore.getBool("enableSystemHz", true);
  configStore.getString("pathSystemHz",       String("sensors.") + configStore.getString("myHostname") + String(".systemHz"));
  configStore.getString("pathSystemFreeMem",  String("sensors.") + configStore.getString("myHostname") + String(".freeMem"));
  configStore.getString("pathSystemUptime",   String("sensors.") + configStore.getString("myHostname") + String(".uptime"));
  app.onTick(countSystemHz);
  app.onRepeat(1000, updateSystemHz);
}


void countSystemHz() {
  systemHzCount++;
}


void updateSystemHz() {
  if (!configStore.getBool("enableSystemHz")) { return; }
  uint32_t elapsed = millis() - systemHzMs;
  
  if (elapsed == 0) { return; } // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  systemHz = (systemHzCount*1000) / elapsed;
  
  signalK.addValue(configStore.getString("pathSystemHz"), systemHz);
  signalK.addValue(configStore.getString("pathSystemFreeMem"), ESP.getFreeHeap());
  signalK.addValue(configStore.getString("pathSystemUptime"), (float)millis()/1000.0);
  systemHzCount = 0;
  systemHzMs = millis();
}
