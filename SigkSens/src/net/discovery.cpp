#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif

#include <uSSDP.h>

#include "discovery.h"
#include "../../config.h"
#include "../../sigksens.h"

uDevice device;
uSSDP SSDP;

void setupDiscovery() {
  if (!MDNS.begin(myHostname)) {             // Start the mDNS responder for esp8266.local
    Serial.println(F("Error setting up MDNS responder!"));
  } else {
    Serial.print (F("mDNS responder started at "));
    Serial.print (myHostname);
    Serial.println(F(""));
  }
  MDNS.addService("http", "tcp", 80);
  
  
  byte mac[6];
  char base[UUIDBASE_SIZE];
  WiFi.macAddress(mac); 
  


  device.begin((const char*)base, mac);
  device.serialNumber((char*)"1241");
  device.manufacturer((char*)"SigKSens");
  device.manufacturerURL((char*)"http://www.signalk.org");
  device.modelName((char*)"ESP-32");
  device.modelNumber(1, 0);
  device.friendlyName((char*)"ESP32-WROVER");
  device.presentationURL((char*)"/");
  SSDP.begin(&device);
  Serial.println("SSDP Started");
/*
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
*/

}