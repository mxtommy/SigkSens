




void setupI2C() {
  Wire.begin();
  
  scanI2C();

  setupSHT30();

  
}

void handleI2C() {

  handleSHT30();
  
}



void scanI2C() {
  SensorInfo *tmpSensorInfo;

  
  Serial.println("Scanning for i2c Sensors...");

  //SHT30
  if (scanI2CAddress(0x45)) { //Sht on D1 sheild
    bool known = false;
    for (int x=0;x<sensorList.size() ; x++) {
      tmpSensorInfo = sensorList.get(x);
      if (strcmp(tmpSensorInfo->address, "45") == 0) {
        known = true;                
      }
    }
    if (!known) {
      Serial.print("New SHT Sensor found at: 0x45 ");
      SensorInfo *newSensor = new SensorInfo();
      strcpy(newSensor->address, "45");
      strcpy(newSensor->type,"sht30");
      newSensor->attrName[0] = "tempK";
      newSensor->attrName[1] = "humidity";
      newSensor->signalKPath[0] = "";
      newSensor->signalKPath[1] = "";
      newSensor->valueJson[0] = "null";
      newSensor->valueJson[1] = "null";
      sensorList.add(newSensor);         
      saveConfig();
    }    
  }


}


bool scanI2CAddress(uint8_t address) {

  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();

  if (errorCode == 0) {
    return true;
  }
  return false;
}





