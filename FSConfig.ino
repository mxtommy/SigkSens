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
          newSensor->tempK = 0;
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
