// confusion that it's inverted...

//Timers
os_timer_t  digitalInTimer; // timer to scan for new devices
bool readyToUpdateDigitalIn1 = false;
bool readyToUpdateDigitalIn2 = false;


bool d1ValueLast = false;
bool d2ValueLast = false;
bool d1Value = false;
bool d2Value = false;

void setupDigitalIn() {
  SensorInfo *tmpSensorInfo;

  pinMode(D1_PIN, INPUT); 
  pinMode(D2_PIN, INPUT); 

  if (digitalRead(D1_PIN) == LOW) {
    d1Value = true;
    d1ValueLast = true;
  }
  if (digitalRead(D2_PIN) == LOW) {
    d2Value = true;
    d2ValueLast = true;
  }
  
  // Setup "sensors" if not already existing
  bool known1 = false;
  bool known2 = false;
  for (int x=0;x<sensorList.size() ; x++) {
    tmpSensorInfo = sensorList.get(x);
    if (strcmp(tmpSensorInfo->type, "digitalIn") == 0) {
      if (strcmp(tmpSensorInfo->address, "1") == 0) {
        known1 = true;                
      }
      if (strcmp(tmpSensorInfo->address, "2") == 0) {
        known2 = true;                
      }      
    }

  }    
  if (!known1) {
    Serial.print("Setting up Digital Input 1");
    SensorInfo *newSensor = new SensorInfo();
    strcpy(newSensor->address, "1");
    strcpy(newSensor->type,"digitalIn");
    newSensor->attrName[0] = "state";
    newSensor->attrName[1] = "freq";
    newSensor->signalKPath[0] = "";
    newSensor->signalKPath[1] = "";
    newSensor->valueJson[0] = "null";
    newSensor->valueJson[1] = "null";
    newSensor->isUpdated = false;
    sensorList.add(newSensor);         
    saveConfig();
  }    
  if (!known2) {
    Serial.print("Setting up Digital Input 2");
    SensorInfo *newSensor = new SensorInfo();
    strcpy(newSensor->address, "2");
    strcpy(newSensor->type,"digitalIn");
    newSensor->attrName[0] = "state";
    newSensor->attrName[1] = "freq";
    newSensor->signalKPath[0] = "";
    newSensor->signalKPath[1] = "";
    newSensor->valueJson[0] = "null";
    newSensor->valueJson[1] = "null";
    newSensor->isUpdated = false;
    sensorList.add(newSensor);         
    saveConfig();
  }    



  os_timer_setfn(&digitalInTimer, interuptUpdateDigitalIn, NULL);
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);

  
}


void handleDigitalIn() {


  if (d1Mode == 1) {
    checkD1State();
  }

  if (d2Mode == 1) {
    checkD2State();
  }

  if (readyToUpdateDigitalIn1 || readyToUpdateDigitalIn2) {
    updateDigitalIn();
  }
  
}

void interuptUpdateDigitalIn(void *pArg) {
  if (d1Mode != 0) { readyToUpdateDigitalIn1 = true; }
  if (d2Mode != 0) { readyToUpdateDigitalIn2 = true; }
}

void setDigitalInUpdateDelay(uint32_t newDelay) {
  os_timer_disarm(&digitalInTimer);
  Serial.print("Restarting DigitalIn polling timer at: ");
  Serial.print(newDelay);  
  Serial.println("ms");
  updateDigitalInDelay = newDelay;
  os_timer_arm(&digitalInTimer, updateDigitalInDelay, true);
}

void checkD1State() {
  if (digitalRead(D1_PIN) == LOW) {
    d1Value = true;
  } else {
    d1Value = false; 
  }
  if (d1Value && !d1ValueLast) { 
    //just activated!
    d1ValueLast = d1Value;
    readyToUpdateDigitalIn1 = true;
  }
  if (!d1Value && d1ValueLast) { 
    //just deactivated!
    d1ValueLast = d1Value;
    readyToUpdateDigitalIn1 = true;
  }
}

void checkD2State() {
  if (digitalRead(D2_PIN) == LOW) {
    d2Value = true;
  } else {
    d2Value = false; 
  }
  if (d2Value && !d2ValueLast) { 
    //just activated!
    d2ValueLast = d2Value;
    readyToUpdateDigitalIn2 = true;
  }
  if (!d2Value && d2ValueLast) { 
    //just deactivated!
    d2ValueLast = d2Value;
    readyToUpdateDigitalIn2 = true;
  }
}


void updateDigitalIn() {
  SensorInfo *thisSensorInfo;
  for (uint8_t i=0; i < sensorList.size(); i++) {
    thisSensorInfo = sensorList.get(i);
    
    if (strcmp(thisSensorInfo->type, "digitalIn") == 0) {
      if (strcmp(thisSensorInfo->address, "1") == 0 && readyToUpdateDigitalIn1) {
        if (d1Value) {
          thisSensorInfo->valueJson[0] = "true";
        } else {
          thisSensorInfo->valueJson[0] = "false";
        }
        thisSensorInfo->valueJson[1] = "null";
        thisSensorInfo->isUpdated = true;
      }
      if (strcmp(thisSensorInfo->address, "2") == 0 && readyToUpdateDigitalIn2) {
        if (d2Value) {
          thisSensorInfo->valueJson[0] = "true";
        } else {
          thisSensorInfo->valueJson[0] = "false";
        }        
        thisSensorInfo->valueJson[1] = "null";
        thisSensorInfo->isUpdated = true;
      }
    }   
  }

  readyToUpdateDigitalIn1 = false;
  readyToUpdateDigitalIn2 = false;
}


