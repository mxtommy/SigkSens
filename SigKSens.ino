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

#include <OneWire.h>
#include <DallasTemperature.h>


/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Defines
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/


#define RESET_CONFIG_PIN 0
#define ONE_WIRE_BUS 13   // D7 pin on ESP
#define TEMPERATURE_PRECISION 10 // 10 is 0.25C resolution in 187ms. (11 is .125C Resolution in 375ms, 12 .0625C in 750ms)
#define MAX_SIGNALK_PATH_LEN 100

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Global Variables
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

ESP8266WebServer server(80);

char myHostname[16];




// memory to save sensor info
class SensorInfo {
  public:
    uint8_t sensorAddress[8];
    char signalKPath[MAX_SIGNALK_PATH_LEN];
    float tempK;
};

LinkedList<SensorInfo*> sensorList = LinkedList<SensorInfo*>();

//flag for saving data
bool shouldSaveConfig = false;

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Config Save/Load/Reset
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/



void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void saveConfig() {
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;

  JsonObject& json = jsonBuffer.createObject();
  json["hostname"] = myHostname;

  //sensors
  JsonArray& oneWSensors = json.createNestedArray("1wSensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = oneWSensors.createNestedObject();
    JsonArray& tmpAddress = tmpSens.createNestedArray("address");
    for (uint8_t x = 0; x < 8; x++)
    {
      tmpAddress.add(tmpSensorInfo->sensorAddress[x]);
    }
    tmpSens.set<String>("signalKPath", tmpSensorInfo->signalKPath );

  }

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.prettyPrintTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

void loadConfig() {
  SensorInfo *tmpSensorInfo;
  uint8_t tempDeviceAddress[8];

  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      Serial.println("Current Configuration:");
      json.prettyPrintTo(Serial);
      if (json.success()) {
        Serial.println("\nparsed json");
        // load hostname
        strcpy(myHostname, json["hostname"]);
        // load known sensors
        for (uint8_t i=0; i < json["1wSensors"].size(); i++) {

          SensorInfo *newSensor = new SensorInfo();
          //extract address array
          for (uint8_t x=0; x<8; x++) {
            tempDeviceAddress[x] = json["1wSensors"][i]["address"][x];
          }

          memcpy(newSensor->sensorAddress, tempDeviceAddress,8);
          strcpy(newSensor->signalKPath,json["1wSensors"][i]["signalKPath"]);
          sensorList.add(newSensor);
        }

      } else {
        Serial.println("failed to load json config");
      }
    }
  }
}

void resetConfig() {
  WiFiManager wifiManager;
  Serial.println("Resetting Config!");
  wifiManager.resetSettings();
  SPIFFS.remove("/config.json");
  for (uint8_t x=0; x<50; x++) { // need delay as resetting right away 
    digitalWrite(LED_BUILTIN, LOW);  // to cause wifi settings to not erase
    delay(20);                 
    digitalWrite(LED_BUILTIN, HIGH);
    delay(20);
  }

  ESP.reset();
}


/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
One Wire
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

os_timer_t  oneWireRequestTimer; // repeating timer that fires ever X/time to start temp request cycle
os_timer_t  oneWireReadyTimer; // once request cycle starts, this timer set so we can send when ready
bool readyToRequest1Wire = false;
bool readyToRead1Wire = false;
uint16_t readingTempDelay = 5000; //ms between reading

void setup1Wire() {
  sensors.begin();
  
  sensors.setWaitForConversion(false);

  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  Serial.print("1Wire Device precision currently: ");
  Serial.print(sensors.getResolution());
  Serial.print(" setting to ");
  Serial.print(TEMPERATURE_PRECISION);
  sensors.setResolution(TEMPERATURE_PRECISION);
  Serial.println(" Done!");

  Serial.print("Starting temperature polling timer at: ");
  Serial.print(readingTempDelay);
  Serial.println("ms");

  os_timer_setfn(&oneWireRequestTimer, interuptRequest1WSensors, NULL);
  os_timer_setfn(&oneWireReadyTimer, interuptReady1WSensors, NULL);

  os_timer_arm(&oneWireRequestTimer, readingTempDelay, true);
}

//called once every loop()
void handle1Wire() {

  // If it's time to request temps, well request it...
  if (readyToRequest1Wire) {
    digitalWrite(LED_BUILTIN, LOW);
    request1WSensors();
    digitalWrite(LED_BUILTIN, HIGH);  
  }

  //ready to send temps! 
  if (readyToRead1Wire) {
    digitalWrite(LED_BUILTIN, LOW);
    read1WSensors();
    digitalWrite(LED_BUILTIN, HIGH);      
  }

}


// debug function to print a device address
void printAddress(DeviceAddress deviceAddress) {
 for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) {
      Serial.print(":");
    }
  }
}

void interuptRequest1WSensors(void *pArg) {
  readyToRequest1Wire = true;
}

void interuptReady1WSensors(void *pArg) {
  readyToRead1Wire = true;
} 

void request1WSensors() {
  readyToRequest1Wire = false; // reset interupt

  sensors.begin(); //needed so the library searches for new sensors that came up since boot
  sensors.requestTemperatures();

  // start ready timer
  uint16_t msWait;
  switch(TEMPERATURE_PRECISION) {
    case 9:
      msWait = 94;
    case 10:
      msWait =  188;
    case 11:
      msWait =  375;
    default:
      msWait =  750;
  }
  os_timer_arm(&oneWireReadyTimer, msWait, false); // false = no loop
  
}




void read1WSensors() {
  readyToRead1Wire = false; // reset interupt
 
  uint8_t tempDeviceAddress[8];
  int numberOfDevices = 0;

  numberOfDevices = sensors.getDeviceCount();
  SensorInfo *tmpSensorInfo;

  for(int i=0;i<numberOfDevices; i++) {
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      
      float tempC = sensors.getTempC(tempDeviceAddress);
      float tempK = tempC + 273.15;

      //see if it's in sensorInfo
      bool known = false;
      for (int x=0;x<sensorList.size() ; x++) {
        tmpSensorInfo = sensorList.get(x);
        if (memcmp(tmpSensorInfo->sensorAddress, tempDeviceAddress, sizeof(tempDeviceAddress)) == 0) {
          tmpSensorInfo->tempK = tempK;
          known = true;
        }
      }
      if (!known) {
        Serial.print("New Sensor found: ");
        printAddress(tempDeviceAddress);
        Serial.println("");        
        SensorInfo *newSensor = new SensorInfo();
        memcpy(newSensor->sensorAddress, tempDeviceAddress,8);
        strcpy(newSensor->signalKPath,"");
        newSensor->tempK = tempK;
        sensorList.add(newSensor);
        saveConfig();
      }

    }
  }
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

void setupFS() {
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    Serial.println("FS Contents:");
    String str = "";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      str += dir.fileName();
      str += " / ";
      str += dir.fileSize();
      str += "\r\n";
    }
    Serial.print(str);
  } else {
    Serial.println("failed to mount FS");
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

void setupHTTP() {
  Serial.println("starting webserver");
  server.onNotFound(handleNotFound);
  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.serveStatic("/index.html", SPIFFS, "/web/index.html");
  server.on("/getSensors", HTTP_GET, htmlGetSensors);
  server.on("/set1wSensorPath", HTTP_GET, htmlSet1WPath);
  server.on("/description.xml", HTTP_GET, [](){  SSDP.schema(server.client()); });
  server.begin();
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

  setup1Wire();



  Serial.printf("Ready!\n");

  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// End Setup! /////////////////////////////////////////////////////////////////////////////////////////
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void htmlGetSensors() {
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  char response[1024];
  JsonObject& json = jsonBuffer.createObject();
  char strAddress[32];
  //sensors
  JsonArray& oneWSensors = json.createNestedArray("1wSensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = oneWSensors.createNestedObject();
    sprintf(strAddress, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
        tmpSensorInfo->sensorAddress[0], 
        tmpSensorInfo->sensorAddress[1], 
        tmpSensorInfo->sensorAddress[2],
        tmpSensorInfo->sensorAddress[3], 
        tmpSensorInfo->sensorAddress[4], 
        tmpSensorInfo->sensorAddress[5], 
        tmpSensorInfo->sensorAddress[6], 
        tmpSensorInfo->sensorAddress[7]  );
    tmpSens.set<String>("address", strAddress);
    tmpSens.set<String>("signalKPath", tmpSensorInfo->signalKPath );
    tmpSens.set<float>("tempK", tmpSensorInfo->tempK);
  }
  json.printTo(response);
  server.send ( 200, "application/json", response);
}


void htmlSet1WPath() {
  
  DeviceAddress address;
  SensorInfo *tmpSensorInfo;
  char pathStr[MAX_SIGNALK_PATH_LEN];
  bool found = false;

  Serial.print("Setting path for 1W Sensor: ");
  if(!server.hasArg("address")) {server.send(500, "text/plain", "missing arg 'address'"); return;}
  if(!server.hasArg("path")) {server.send(500, "text/plain", "missing arg 'path'"); return;}
  
  String addressStr = server.arg("address");
  server.arg("path").toCharArray(pathStr, MAX_SIGNALK_PATH_LEN);

  parseBytes(addressStr.c_str(), ':', address,  8, 16);

  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (memcmp(tmpSensorInfo->sensorAddress, address, sizeof(address)) == 0) {
      memcpy(tmpSensorInfo->signalKPath, pathStr, MAX_SIGNALK_PATH_LEN);
      found = true;
      break; //no need to check others if we found it
    }
  }

  if (found) {
    saveConfig();
    server.send(200, "application/json", "{ \"success\": true }");
  } else {
    server.send(500, "application/json", "{ \"success\": false }");
  }
  
}




void loop() {
  //device mgmt
  ArduinoOTA.handle();
  server.handleClient();

  //our stuff
  handle1Wire();
  

  yield();           

  

  
  //Reset Config!
  if (digitalRead(RESET_CONFIG_PIN) == LOW) {
    //resetConfig();
  } 

}


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
