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

  JsonObject& json = jsonBuffer.createObject();
  json["hostname"] = myHostname;

  //oneWire sensors
  JsonArray& jsonSensors = json.createNestedArray("sensors");
  for (uint8_t i=0; i < sensorList.size(); i++) {
    tmpSensorInfo = sensorList.get(i);
    JsonObject& tmpSens = jsonSensors.createNestedObject();
    tmpSens["address"] = tmpSensorInfo->address;
    tmpSens.set<String>("signalKPath", tmpSensorInfo->signalKPath );
    tmpSens["type"] = tmpSensorInfo->type;
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
        for (uint8_t i=0; i < json["sensors"].size(); i++) {

          SensorInfo *newSensor = new SensorInfo();
          //extract address array

          strcpy(newSensor->address, json["sensors"][i]["address"]);
          strcpy(newSensor->signalKPath,json["sensors"][i]["signalKPath"]);
          strcpy(newSensor->type, json["sensors"][i]["type"]);

          // set valueJson to null of that sensor type
          //should probably do this elsewhere to keep concerns seperate...
          if (strcmp(newSensor->type, "oneWire") == 0) {
            strcpy(newSensor->valueJson, "{ tempK: null }");
          }
          else if (strcmp(newSensor->type, "sht30") == 0) {
            strcpy(newSensor->valueJson, "{ tempK: null, humidity: null }");
          }

          
          sensorList.add(newSensor);
        }



      } else {
        Serial.println("failed to load json config");
      }
    }
  }
}


