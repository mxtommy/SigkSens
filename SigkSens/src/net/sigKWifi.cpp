extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
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
  
  wifiManager.setConfigPortalTimeout(180);

  WiFiManagerParameter custom_hostname("myHostname", "Set Hostname", myHostname, 16);
  wifiManager.addParameter(&custom_hostname);
  
  if (!wifiManager.autoConnect("Unconfigured Sensor")) {
    Serial.println(F("Failed to connect to wifi and config timed out."));
    ESP.restart();
  }

  Serial.println(F("Connected to Wifi!"));

  // Save config if needed
  if (shouldSaveConfig) {
    strcpy(myHostname, custom_hostname.getValue());
    saveConfig();
    // For some reason, connection isn't properly established after exiting the
    // captive portal. Just reset to be safe.
    ESP.restart();
  }
}

