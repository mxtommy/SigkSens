os_timer_t  updateHz; // once request cycle starts, this timer set so we can send when ready
bool readyToUpdateHz = false;

uint32_t systemHzCount = 0, systemHzMs = 0;

void setupSystemHz() {
  os_timer_setfn(&updateHz, interuptSystemHz, NULL);
  os_timer_arm(&updateHz, 1000, true);
  systemHzMs = millis();
  SensorInfo *tmpSensorInfo;


  // Setup "sensor" if not already existing
  bool known = false;
  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (strcmp(tmpSensorInfo->address, "Local") == 0) {
      known = true;                
    }
  }    
  if (!known) {
    Serial.print("Setting up System info ");
    SensorInfo *newSensor = new SensorInfo();
    strcpy(newSensor->address, "Local");
    strcpy(newSensor->type,"Local");
    newSensor->attrName[0] = "systemHz";
    newSensor->attrName[1] = "freeMem";
    newSensor->signalKPath[0] = "sensor.systemHz";
    newSensor->signalKPath[1] = "sensor.freeMem";
    newSensor->valueJson[0] = "null";
    newSensor->valueJson[1] = "null";
    newSensor->isUpdated = false;
    sensorList.add(newSensor);         
    saveConfig();
  }    


  
}


void handleSystemHz() {
  if (readyToUpdateHz) {
    // reset interupt
    readyToUpdateHz = false;
    updateSystemHz();
  }
  systemHzCount++;
  
}


void interuptSystemHz(void *pArg) {
  readyToUpdateHz = true;
}


void updateSystemHz() {
  SensorInfo *thisSensorInfo;
  uint32_t elapsed = millis() - systemHzMs;
  systemHz = (systemHzCount*1000) / elapsed;
 // Serial.print ("System Hz :");
 // Serial.println (systemHz);

  for (uint8_t i=0; i < sensorList.size(); i++) {
    thisSensorInfo = sensorList.get(i);
    if (strcmp(thisSensorInfo->type, "Local") == 0) {
        
      thisSensorInfo->valueJson[0] = systemHz;
      thisSensorInfo->valueJson[1] = ESP.getFreeHeap();
      thisSensorInfo->isUpdated = true;
    }
  }

  systemHzCount = 0;
  systemHzMs = millis();
}

