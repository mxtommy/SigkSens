#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "../../../sigksens.h" // for app instance
#include "../../services/signalK.h"
#include "systemHz.h"

ComponentSystemHz componentSystemHz("systemHz");

void ComponentSystemHz::setupComponent() {
  systemHzMs = millis();
  systemHzCount = 0;
  systemHz = 0;

  //sets default if not already defined
  configStore.getBool("enableSystemHz", true);
  configStore.getString("pathSystemHz",       String("sensors.") + configStore.getString("myHostname") + String(".systemHz"));
  configStore.getString("pathSystemFreeMem",  String("sensors.") + configStore.getString("myHostname") + String(".freeMem"));
  configStore.getString("pathSystemUptime",   String("sensors.") + configStore.getString("myHostname") + String(".uptime"));
  //app.onTick(countSystemHz);
  //app.onRepeat(1000, [this]() { this->handleComponent(); });
}


void ComponentSystemHz::countSystemHz() {
  systemHzCount++;
}


void ComponentSystemHz::handleComponent() {
  if (!config.getBool("enableSystemHz")) { return; }
  uint32_t elapsed = millis() - systemHzMs;
  
  if (elapsed == 0) { return; } // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  systemHz = (systemHzCount*1000) / elapsed;
  
  signalK.addValue(configStore.getString("pathSystemHz"), systemHz);
  signalK.addValue(configStore.getString("pathSystemFreeMem"), ESP.getFreeHeap());
  signalK.addValue(configStore.getString("pathSystemUptime"), (float)millis()/1000.0);
  systemHzCount = 0;
  systemHzMs = millis();
}
