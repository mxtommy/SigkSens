#include <Reactduino.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266SSDP.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson

#include <string>

#include "config.h"
#include "FSConfig.h"
#ifdef ENABLE_I2C
#include "i2c.h"
#endif
#ifdef ENABLE_MPU
  #include "mpu9250.h"
#endif
#ifdef ENABLE_SHT30
  #include "sht30.h"
#endif
#ifdef ENABLE_BMP280
  #include "bmp280.h"
#endif
#ifdef ENABLE_ADS1115
#include "ads1115.h"
#endif
#ifdef ENABLE_ONEWIRE
  #include "oneWire.h"
#endif
#ifdef ENABLE_DIGITALIN
  #include "digitalIn.h"
#endif
#ifdef ENABLE_DIGITALOUT
  #include "digitalOut.h"
#endif
#ifdef ENABLE_ANALOGIN
  #include "analogIn.h"
#endif
#ifdef ENABLE_SYSTEMHZ
  #include "systemHz.h"
#endif

#include "configReset.h"
#include "webSocket.h"
#include "signalK.h"
#include "httpd.h"
#include "sigksens.h"

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Global Variables
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

char myHostname[16];

uint16_t mainLoopCount = 0; //some stuff needs to run constantly, others not. so run some stuff only every X loops.

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
General Setup
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

void setupFromJson() {
  #ifdef ENABLE_SYSTEMHZ
  fromJson[(int)SensorType::local] =
    (fromJsonFunc)&(SystemHzSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_DIGITALIN
  fromJson[(int)SensorType::digitalIn] =
    (fromJsonFunc)&(DigitalInSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_DIGITALOUT
  fromJson[(int)SensorType::digitalOut] =
    (fromJsonFunc)&(DigitalOutSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_ANALOGIN
  fromJson[(int)SensorType::analogIn] =
    (fromJsonFunc)&(AinSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_ONEWIRE
  fromJson[(int)SensorType::oneWire] =
    (fromJsonFunc)&(OneWireSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_SHT30
  fromJson[(int)SensorType::sht30] =
    (fromJsonFunc)&(SHT30SensorInfo::fromJson);
  #endif

  #ifdef ENABLE_MPU
  fromJson[(int)SensorType::mpu925x] =
    (fromJsonFunc)&(MPU9250SensorInfo::fromJson);
  #endif    

  #ifdef ENABLE_ADS1115
  fromJson[(int)SensorType::ads1115] =
    (fromJsonFunc)&(ADSSensorInfo::fromJson);    
  #endif

  #ifdef ENABLE_BMP280
  fromJson[(int)SensorType::bmp280] =
    (fromJsonFunc)&(BMP280SensorInfo::fromJson);
  #endif
}


void setupWifi() {
  WiFiManager wifiManager;
 
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_hostname("myHostname", "Set Hostname", myHostname, 16);

  wifiManager.addParameter(&custom_hostname);
  
  wifiManager.autoConnect("Unconfigured Sensor");
  Serial.println("Connected to Wifi!");

  // Save config if needed
  if (shouldSaveConfig) {
    strcpy(myHostname, custom_hostname.getValue());
    saveConfig();
  }
}


void setupDiscovery() {
  if (!MDNS.begin(myHostname)) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.print ("mDNS responder started at ");
    Serial.print (myHostname);
    Serial.println("");
  }
  MDNS.addService("http", "tcp", 80);
  
  Serial.printf("Starting SSDP...\n");
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

// forward declarations
void loop_();
void slow_loop();

Reactduino app([] () {
    bool need_save = false;
  // put your setup code here, to run once:
  Serial.begin(115200);

  setupFromJson();

  setupFS();


  setupWifi();
  loadConfig();
  setupDiscovery();
  setupHTTP();
  setupWebSocket();
  setupSignalK();

  setupConfigReset();
  #ifdef ENABLE_ONEWIRE
  setup1Wire(need_save);
  #endif
  #ifdef ENABLE_I2C
  setupI2C(need_save);
  #endif
  #ifdef ENABLE_DIGITALIN
  setupDigitalIn(need_save);
  #endif
  #ifdef ENABLE_DIGITALOUT
  setupDigitalOut(need_save);
  #endif
  #ifdef ENABLE_ANALOGIN
  setupAnalogIn(need_save);
  #endif
  #ifdef ENABLE_SYSTEMHZ
  setupSystemHz(need_save);
  #endif
  
  if (need_save) {
    saveConfig();
  }
  
  Serial.printf("Ready!\n");

  app.repeat(SLOW_LOOP_DELAY, &slow_loop);

  app.onTick(&loop_);
});

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Main Loop!
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

void slow_loop() {
  handleWebSocket();
  
  handleConfigReset(); 
  mainLoopCount = 0;
}

void loop_() {
  mainLoopCount++;
}
