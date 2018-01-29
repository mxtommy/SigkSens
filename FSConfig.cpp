extern "C" {
#include "user_interface.h"
}

#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson

#include "FSConfig.h"
#include "webSocket.h"
#include "oneWire.h"
#include "sht30.h"
#include "mpu.h"
#include "digitalIn.h"
#include "sigksens.h"

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Config Save/Load/Reset
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/

//flag for saving data in FSConfig
bool shouldSaveConfig = false;

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


void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void saveConfig() {
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  uint8_t numAttr;
  char tmpPinStr[NUMBER_DIGITAL_INPUT][10];

  JsonObject& json = jsonBuffer.createObject();
  json["hostname"] = myHostname;
  
  json["signalKHost"] = signalKClientInfo.host;
  json["signalKPort"] = signalKClientInfo.port;
  json["signalKPath"] = signalKClientInfo.path;

  //sensors
  JsonArray& jsonSensors = json.createNestedArray("sensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = jsonSensors.createNestedObject();
    if (tmpSensorInfo->isSerializable()) {
      tmpSensorInfo->toJson(tmpSens);
    } else {
      tmpSens["address"] = tmpSensorInfo->address;
      tmpSens["type"] = tmpSensorInfo->type;

      JsonArray& jsonPaths = tmpSens.createNestedArray("signalKPaths");
      for (int x=0;x<MAX_SENSOR_ATTRIBUTES; x++) {
        if (strcmp(tmpSensorInfo->attrName[x].c_str(), "") == 0 ) {
          break; //no more attributes
        }
        jsonPaths.add(tmpSensorInfo->signalKPath[x]);
      }
    }
  }

  uint32_t oneWireReadDelay = getOneWireReadDelay();
  
  //Timers
  json["oneWireReadDelay"] = oneWireReadDelay;
  json["sensorSHTReadDelay"] = getSensorSHTReadDelay();
  json["updateMPUDelay"] = getUpdateMPUDelay();
  json["updateDigitalInDelay"] = getUpdateDigitalInDelay();


  //Digital Pins
  JsonObject& digitalPins = json.createNestedObject("digitalPinModes");
  for (uint8_t x=0; x < NUMBER_DIGITAL_INPUT; x++) {
    ;
    getDigitalPinName(x, tmpPinStr[x]); // sets tmpPinStr to the name of pin (array of char)
    Serial.print(tmpPinStr[x]);
    digitalPins.set(tmpPinStr[x], getDigitalMode(x));
    Serial.println(x);
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
  char tempStr[255];

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

        //signalk
        strcpy(tempStr, json["signalKHost"]); signalKClientInfo.host = tempStr;
        strcpy(tempStr, json["signalKPath"]); signalKClientInfo.path = tempStr;
        signalKClientInfo.port = json["signalKPort"];

        // load known sensors
        for (uint8_t i=0; i < json["sensors"].size(); i++) {
          String type = json["sensors"][i]["type"];
          SensorInfo *newSensor;
          newSensor = new SensorInfo();
          strcpy(newSensor->address, json["sensors"][i]["address"]);
          strcpy(newSensor->type, type.c_str());
          newSensor->isUpdated = false;
          
          // load paths and set valueJson to null of that sensor type
          //should probably do this elsewhere to keep concerns seperate...
          if (type == "Local") {
            newSensor = new SensorInfo();
            strcpy(newSensor->address, json["sensors"][i]["address"]);
            strcpy(newSensor->type, type.c_str());
            newSensor->isUpdated = false;

            // systemHz
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor->attrName[0] = "systemHz";
            newSensor->signalKPath[0] = tempStr;            
            newSensor->valueJson[0] = "null";
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][1]);
            newSensor->attrName[1] = "freeMem";
            newSensor->signalKPath[1] = tempStr;            
            newSensor->valueJson[1] = "null";
          
          } else if (type == "oneWire") {
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor = new OneWireSensorInfo(
              json["sensors"][i]["address"],
              json["sensors"][i]["signalKPaths"][0]);
          }
          else if (type == "sht30") {
            newSensor = new SensorInfo();
            strcpy(newSensor->address, json["sensors"][i]["address"]);
            strcpy(newSensor->type, type.c_str());
            newSensor->isUpdated = false;

            //tempK            
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor->attrName[0] = "tempK";
            newSensor->signalKPath[0] = tempStr;                
            newSensor->valueJson[0] = "null";

            //humidity
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][1]);
            newSensor->attrName[1] = "humidity";
            newSensor->signalKPath[1] = tempStr;              
            newSensor->valueJson[1] = "null";
          }
          else if (type == "mpu925x") {
            newSensor = new SensorInfo();
            strcpy(newSensor->address, json["sensors"][i]["address"]);
            strcpy(newSensor->type, type.c_str());
            newSensor->isUpdated = false;

            //tempK (of gyro sensor)           
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor->attrName[0] = "tempK";
            newSensor->signalKPath[0] = tempStr;                
            newSensor->valueJson[0] = "null";
                        
            //yaw            
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][1]);
            newSensor->attrName[1] = "yaw";
            newSensor->signalKPath[1] = tempStr;                
            newSensor->valueJson[1] = "null";

            //pitch
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][2]);
            newSensor->attrName[2] = "pitch";
            newSensor->signalKPath[2] = tempStr;              
            newSensor->valueJson[2] = "null";

            //roll
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][3]);
            newSensor->attrName[3] = "roll";
            newSensor->signalKPath[3] = tempStr;              
            newSensor->valueJson[3] = "null";            
          } else if (type == "digitalIn") {
            newSensor = new SensorInfo();
            strcpy(newSensor->address, json["sensors"][i]["address"]);
            strcpy(newSensor->type, type.c_str());
            newSensor->isUpdated = false;

            // state
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor->attrName[0] = "state";
            newSensor->signalKPath[0] = tempStr;            
            newSensor->valueJson[0] = "null";
            // freq
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][1]);
            newSensor->attrName[1] = "freq";
            newSensor->signalKPath[1] = tempStr;            
            newSensor->valueJson[1] = "null";
          }
          
          sensorList.add(newSensor);
        }

        //Timers

        uint32_t oneWireReadDelay = json["oneWireReadDelay"];
        setOneWireReadDelay(oneWireReadDelay);

        setSHTReadDelay(json["sensorSHTReadDelay"]);
        setMPUUpdateDelay(json["updateMPUDelay"]);
        setDigitalInUpdateDelay(json["updateDigitalInDelay"]);

        //Digital
        JsonObject& digitalPins = json["digitalPinModes"];
        for (auto keyValue : digitalPins) {
          setDigitalMode(keyValue.key, keyValue.value.as<uint8_t>());
        }

      } else {
        Serial.println("failed to load json config");
      }
    }
  }
}
