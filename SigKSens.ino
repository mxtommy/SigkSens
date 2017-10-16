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
    DeviceAddress sensorAddress;
     signalKPath[50];
};

LinkedList<SensorInfo*> sensorList = LinkedList<SensorInfo*>();

//flag for saving data
bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  WiFiManager wifiManager;

  // TESTING STUFF
  //clean FS, for testing
  //SPIFFS.format();
  //reset saved settings
  //wifiManager.resetSettings();


  /*************************************************
  * FS Mount
  */

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");

    String str = "";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      str += dir.fileName();
      str += " / ";
      str += dir.fileSize();
      str += "\r\n";
    }
    Serial.print(str);

    
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
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(myHostname, json["hostname"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end FS load

/***************************************************************
 * WIFI 
 */
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_hostname("myHostname", "Set Hostname", myHostname, 16);

  wifiManager.addParameter(&custom_hostname);
  
  wifiManager.autoConnect("Unconfigured Sensor");
  Serial.println("Connected to Wifi!");

/**************************************************************
 * Save config
 */

  if (shouldSaveConfig) {
    strcpy(myHostname, custom_hostname.getValue());
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["hostname"] = myHostname;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

//  if (!MDNS.begin(myHostname)) {             // Start the mDNS responder for esp8266.local
//    Serial.println("Error setting up MDNS responder!");
//  } else {
//    Serial.println("mDNS responder started");
//  }
/*
  Serial.printf("Starting SSDP...\n");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("WifiSensorNode");
    SSDP.setSerialNumber("001788102201");
    SSDP.setURL("index.html");
    SSDP.setModelName("WifiSensorNode");
    SSDP.setModelNumber("929000226503");
    SSDP.setModelURL("http://www.meethue.com");
    SSDP.setManufacturer("Royal Philips Electronics");
    SSDP.setManufacturerURL("http://www.philips.com");
    SSDP.begin();
*/
  Serial.printf("Ready!\n");

  
  /*************************************
   * Over the Air upgrade
   */
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


  /*********************
   * Web
   */
  server.serveStatic("/", SPIFFS, "/web/index.html");
  server.on("/getSensors", htmlGetSensors);
  server.begin();
  
}

void htmlGetSensors() {
  Serial.println(myHostname);
  Serial.println("aa");
}


// function to print a device address
//void printAddress(DeviceAddress deviceAddress)
//{
// for (uint8_t i = 0; i < 8; i++)
//  {
//    // zero pad the address if necessary
//    if (deviceAddress[i] < 16) Serial.print("0");
//    Serial.print(deviceAddress[i], HEX);
//  }
//}




void handleWebRequest() {
  
}


void findSensors() {
  DeviceAddress tempDeviceAddress;
  int numberOfDevices = 0;
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
  SensorInfo *tmpSensorInfo;
  for(int i=0;i<numberOfDevices; i++) {
    Serial.print("Checking num: ");
    Serial.println(i);
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      //see if it's in sensorInfo
      bool known = false;
      for (int x=0;x<sensorList.size() ; x++) {
        Serial.print(x);
        tmpSensorInfo = sensorList.get(x);
        if (tmpSensorInfo->sensorAddress == tempDeviceAddress) {
          known = true;
        }
      }
      if (!known) {
        Serial.println("New Sensorfound!");
        SensorInfo *newSensor = new sensorInfo();
        newSensor->sensorAddress = tempDeviceAddress;
        newSensor->signalKPath = "";
        sensorList.add(newSensor);
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

  findSensors();

  handleWebRequest();
  
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
