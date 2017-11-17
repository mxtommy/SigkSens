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

  //sensors
  JsonArray& jsonSensors = json.createNestedArray("sensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = jsonSensors.createNestedObject();
    tmpSens["address"] = tmpSensorInfo->address;
    tmpSens["type"] = tmpSensorInfo->type;

    // set number of attributes by sensor type
    if (strcmp(tmpSensorInfo->type, "oneWire") == 0) {
      numAttr = 1;
    } else if (strcmp(tmpSensorInfo->type, "sht30") == 0) {
      numAttr = 2;
    } else if (strcmp(tmpSensorInfo->type, "mpu925x") == 0) {
      numAttr = 4;      
    } else {
      //default to all...
      numAttr = MAX_SENSOR_ATTRIBUTES;
    }
    JsonArray& jsonPaths = tmpSens.createNestedArray("signalKPaths");
    for (int x=0;x<numAttr; x++) {
      jsonPaths.add(tmpSensorInfo->signalKPath[x]);
    }
  }

  //Timers
  json["oneWireReadDelay"] = oneWireReadDelay;
  json["sensorSHTReadDelay"] = sensorSHTReadDelay;


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
  char tempStr[MAX_SIGNALK_PATH_LEN];

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
        for (uint8_t i=0; i < json["sensors"].size(); i++) {

          SensorInfo *newSensor = new SensorInfo();
          //extract address array

          strcpy(newSensor->address, json["sensors"][i]["address"]);
          strcpy(newSensor->type, json["sensors"][i]["type"]);

          


          // load paths and set valueJson to null of that sensor type
          //should probably do this elsewhere to keep concerns seperate...
          if (strcmp(newSensor->type, "oneWire") == 0) {
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
          }
          
          sensorList.add(newSensor);
        }

        //Timers
    
        oneWireReadDelay = json["oneWireReadDelay"];
        sensorSHTReadDelay = json["sensorSHTReadDelay"];        

      } else {
        Serial.println("failed to load json config");
      }
    }
  }
}


