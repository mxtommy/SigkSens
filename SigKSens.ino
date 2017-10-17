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

#define ONE_WIRE_BUS 14   // D5 pin on ESP
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

ESP8266WebServer server(80);

char myHostname[16];


// memory to save sensor info
class SensorInfo {
  public:
    uint8_t sensorAddress[8];
    char signalKPath[50];
};

LinkedList<SensorInfo*> sensorList = LinkedList<SensorInfo*>();

//flag for saving data
bool shouldSaveConfig = false;

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

void setupHTTP() {
  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.on("/getSensors", htmlGetSensors);
  server.begin();
}

void setupDiscovery() {
//  if (!MDNS.begin(myHostname)) {             // Start the mDNS responder for esp8266.local
//    Serial.println("Error setting up MDNS responder!");
//  } else {
//    Serial.println("mDNS responder started");
//  }
//  Serial.printf("Starting SSDP...\n");
//    SSDP.setSchemaURL("description.xml");
//    SSDP.setHTTPPort(80);
//    SSDP.setName("WifiSensorNode");
//    SSDP.setSerialNumber("001788102201");
//    SSDP.setURL("index.html");
//    SSDP.setModelName("WifiSensorNode");
//    SSDP.setModelNumber("929000226503");
//    SSDP.setModelURL("http://www.meethue.com");
//    SSDP.setManufacturer("Royal Philips Electronics");
//    SSDP.setManufacturerURL("http://www.philips.com");
//    SSDP.begin();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output


  // TESTING STUFF
  //clean FS, for testing
  //SPIFFS.format();
  //reset saved settings
  //wifiManager.resetSettings();

  setupWifi();
  setupFS();
  loadConfig();
  setupOTA();
  setupHTTP();
  setupDiscovery();
  



  Serial.printf("Ready!\n");

  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// End Setup! /////////////////////////////////////////////////////////////////////////////////////////


void htmlGetSensors() {
  Serial.println(myHostname);
  Serial.println("aa");
}


// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
 for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    //if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    Serial.print("-");
  }
}




void handleWebRequest() {
  
}


void findNewSensors() {
  uint8_t tempDeviceAddress[8];
  int numberOfDevices = 0;
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  SensorInfo *tmpSensorInfo;
  for(int i=0;i<numberOfDevices; i++) {
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      //see if it's in sensorInfo
      bool known = false;
      for (int x=0;x<sensorList.size() ; x++) {
        tmpSensorInfo = sensorList.get(x);
        if (memcmp(tmpSensorInfo->sensorAddress, tempDeviceAddress, sizeof(tempDeviceAddress)) == 0) {
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
        sensorList.add(newSensor);
        saveConfig();
      }

    }
  }
}


void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  digitalWrite(LED_BUILTIN, LOW);  
  delay(500);                 
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  findNewSensors();

  handleWebRequest();
  Serial.println(sensorList.size());
/*
  sensors.begin();
  // locate devices on the bus
  Serial.print(myHostname);
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
*/
}
