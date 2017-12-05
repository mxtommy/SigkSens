#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ESP8266SSDP.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ArduinoOTA.h>
#include <LinkedList.h>

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson

#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>


/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Defines
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/


#define RESET_CONFIG_PIN 0

#define ONE_WIRE_BUS 13   // D7 pin on ESP

// set these together! Precision for OneWire
// 9  is 0.5C in 94ms
// 10 is 0.25C in 187ms
// 11 is 0.125C in 375ms
// 12 is 0.0625C in 750ms
#define TEMPERATURE_PRECISION 9
#define ONEWIRE_READ_DELAY 94

#define MAX_SIGNALK_PATH_LEN 150
#define MAX_SENSOR_ATTRIBUTES 10

#define SHORT_BUTTON_PRESS_MS 1000
#define LONG_BUTTON_PRESS_MS 5000

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Global Variables
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

ESP8266WebServer server(80);



//Websocet
WebSocketsServer webSocketServer = WebSocketsServer(81);
WebSocketsClient webSocketClient;

char myHostname[16];

float systemHz = 0;


//flag for saving data in FSConfig
bool shouldSaveConfig = false;


// memory to save sensor info
class SensorInfo {
  public:
    char address[32];
    String attrName[MAX_SENSOR_ATTRIBUTES];
    String signalKPath[MAX_SENSOR_ATTRIBUTES];
    String valueJson[MAX_SENSOR_ATTRIBUTES];
    char type[10];
    bool isUpdated;
};


LinkedList<SensorInfo*> sensorList = LinkedList<SensorInfo*>();

// Sensors present
bool sensorSHT30Present = false;
bool sensorMPU925XPresent = false;
bool sensorOneWirePresent = false;

// some timers 
uint32_t oneWireReadDelay = 5000; //ms between reading
uint32_t oneWireScanDelay = 30000; //ms between scan
uint32_t sensorSHTReadDelay = 5000; //ms between reading
uint32_t updateMPUDelay = 1000;

// SignalK stuff
String signalKHost = "";
uint16_t signalKPort = 80;
String signalKPath = "/signalk/v1/stream";

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
General Setup
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/


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



void setupOTA() {
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
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


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(RESET_CONFIG_PIN, INPUT_PULLUP);

  setupFS();

  setupWifi();
  loadConfig();
  setupOTA();
  setupDiscovery();
  setupHTTP();
  setupWebSocket();
  setupSignalK();

  setupConfigReset();
  setup1Wire();
  setupI2C();

  setupSystemHz();
  Serial.printf("Ready!\n");

  
}

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Main Loop!
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/


void loop() {
  //device mgmt
  ArduinoOTA.handle();
  yield();           
  

  //our stuff
  handleSystemHz();
  handle1Wire();
  handleI2C();
  handleWebSocket();
  handleSignalK();
  server.handleClient();
  
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
