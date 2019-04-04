#include <Arduino.h>
#include <ReactESP.h>

#include "config.h"
#include "src/services/filesystem.h"
#include "src/services/configStore.h"
#include "src/components/componentSensor.h"



#include "src/net/discovery.h"
#include "src/net/webSocket.h"
#include "src/net/httpd.h"
#include "src/net/sigKWifi.h"
#include "src/net/ota.h"
#include "src/services/ledBlinker.h"

#include "src/services/configReset.h"
#include "src/services/signalK.h"
#include "src/services/i2c.h"

#include "sigksens.h"


/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
General Setup
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

// forward declarations
void loop_();
void slow_loop();

I2cBus* i2cBus;


ReactESP app([] () {

  // put your setup code here, to run once:
  Serial.begin(115200);

  ledBlinker.setWifiDisconnected();

  setupFilesystem();
  configStore.begin("/globalConfig.json");

  setupWifi();
  setupDiscovery();
  setupWebSocket();
  setupSignalK();
  setupOTA();

  setupConfigReset();

  // start i2c
  i2cBus = new I2cBus();
  i2cBus->start();

  //Start all components
  forEachComponent([](ComponentSensor* ComponentSensor) {
    ComponentSensor->begin();
  });


  // call http last so that we can call any needed callbacks.
  setupHTTP();

  Serial.printf("Ready.\n");

  app.onRepeat(SLOW_LOOP_DELAY, slow_loop);
  
});

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Main Loop!
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

void slow_loop() {
  // Check Wifi...
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    ESP.restart();
  }
  configStore.handle(); // will save any config changes if needed
  handleConfigReset(); 
}

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Helper functions
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/


void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
  }
}

String uuidv4() {
  char *hexvalues = "0123456789abcdef";
  String uuid = "";
  for (int i = 0 ; i < 32; i++) {
    if (i == 9) { uuid = uuid + "-"; continue; }
    if (i == 14) { uuid = uuid + "-"; continue; }
    if (i == 19) { uuid = uuid + "-"; continue; }
    if (i == 24) { uuid = uuid + "-"; continue; }
    uuid = uuid + String(hexvalues[random(0,15)]);
  }
  return uuid;
}