#include "ota.h"

#include "../../config.h"
#include "../../sigksens.h"

#include <ArduinoOTA.h>


void handleOTA() {
  ArduinoOTA.handle();
}

void setupOTA() {
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
#endif
  ArduinoOTA.onStart([]() {
    Serial.println(F("Starting OTA"));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
  });
  ArduinoOTA.begin();
  //app.onTick(&handleOTA);
  app.onRepeat(20, handleOTA);
}