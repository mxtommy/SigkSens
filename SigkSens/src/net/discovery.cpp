extern "C" {
#include "user_interface.h"
}

#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266SSDP.h>

#include "discovery.h"
#include "../../config.h"
#include "../../sigksens.h"



void setupDiscovery() {
  if (!MDNS.begin(myHostname)) {             // Start the mDNS responder for esp8266.local
    Serial.println(F("Error setting up MDNS responder!"));
  } else {
    Serial.print (F("mDNS responder started at "));
    Serial.print (myHostname);
    Serial.println(F(""));
  }
  MDNS.addService("http", "tcp", 80);
  
  Serial.println(F("Starting SSDP..."));
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName(myHostname);
    SSDP.setSerialNumber("12345");
    SSDP.setURL("index.html");
    SSDP.setModelName("WifiSensorNode");
    SSDP.setModelNumber("12345");
    SSDP.setModelURL("http://www.signalk.org");
    SSDP.setManufacturer("SigK");
    SSDP.setManufacturerURL("http://www.signalk.org");
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.begin();
}