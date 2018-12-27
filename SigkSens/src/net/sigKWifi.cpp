#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "../../config.h"
#include "../../sigksens.h"
#include "../../FSConfig.h"
#include "sigKWifi.h"



bool shouldSaveConfig = false;


void saveConfigCallback() {
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

void setupWifi() {
  WiFiManager wifiManager;
 
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

  WiFiManagerParameter custom_hostname("myHostname", "Set Hostname", myHostname, 16);
  wifiManager.addParameter(&custom_hostname);
  
  if (!wifiManager.autoConnect("Unconfigured Sensor")) {
    Serial.println(F("Failed to connect to wifi and config timed out."));
    ESP.restart();
  }

  Serial.println(F("Connected to Wifi!"));
  ledBlinker.setWifiConnected();

  // Save config if needed
  if (shouldSaveConfig) {
    strcpy(myHostname, custom_hostname.getValue());
    saveConfig();
    // For some reason, connection isn't properly established after exiting the
    // captive portal. Just reset to be safe.
    ESP.restart();
  }
}

