/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Config Save/Load/Reset
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

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


void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void saveConfig() {
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  SensorInfo *tmpSensorInfo;
  uint8_t numAttr;

  JsonObject& json = jsonBuffer.createObject();
  json["hostname"] = myHostname;
  
  json["signalKHost"] = signalKHost;
  json["signalKPort"] = signalKPort;
  json["signalKPath"] = signalKPath;



  //sensors
  JsonArray& jsonSensors = json.createNestedArray("sensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = jsonSensors.createNestedObject();
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

  uint32_t oneWireReadDelay = getOneWireReadDelay();
  
  //Timers
  json["oneWireReadDelay"] = oneWireReadDelay;
  json["sensorSHTReadDelay"] = sensorSHTReadDelay;
  json["updateMPUDelay"] = updateMPUDelay;
  json["updateDigitalInDelay"] = updateDigitalInDelay;

  //Digital
  json["d1Mode"] = d1Mode;
  json["d2Mode"] = d2Mode;

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
        strcpy(tempStr, json["signalKHost"]); signalKHost = tempStr;
        strcpy(tempStr, json["signalKPath"]); signalKPath = tempStr;
        signalKPort = json["signalKPort"];


        // load known sensors
        for (uint8_t i=0; i < json["sensors"].size(); i++) {

          SensorInfo *newSensor = new SensorInfo();
          //extract address array

          strcpy(newSensor->address, json["sensors"][i]["address"]);
          strcpy(newSensor->type, json["sensors"][i]["type"]);
          newSensor->isUpdated = false;
          


          // load paths and set valueJson to null of that sensor type
          //should probably do this elsewhere to keep concerns seperate...
          if (strcmp(newSensor->type, "Local") == 0) {
            // systemHz
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor->attrName[0] = "systemHz";
            newSensor->signalKPath[0] = tempStr;            
            newSensor->valueJson[0] = "null";
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][1]);
            newSensor->attrName[1] = "freeMem";
            newSensor->signalKPath[1] = tempStr;            
            newSensor->valueJson[1] = "null";
          
          } else if (strcmp(newSensor->type, "oneWire") == 0) {
            // tempK
            strcpy(tempStr, json["sensors"][i]["signalKPaths"][0]);
            newSensor->attrName[0] = "tempK";
            newSensor->signalKPath[0] = tempStr;            
            newSensor->valueJson[0] = "null";
          }
          else if (strcmp(newSensor->type, "sht30") == 0) {
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
          else if (strcmp(newSensor->type, "mpu925x") == 0) {
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
          } else if (strcmp(newSensor->type, "digitalIn") == 0) {
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

        sensorSHTReadDelay = json["sensorSHTReadDelay"];        
        updateMPUDelay = json["updateMPUDelay"];
        updateDigitalInDelay = json["updateDigitalInDelay"];

        //Digital
        d1Mode = json["d1Mode"];
        d2Mode = json["d2Mode"];

      } else {
        Serial.println("failed to load json config");
      }
    }
  }
}


