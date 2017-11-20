/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
One Wire
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

os_timer_t  oneWireRequestTimer; // repeating timer that fires ever X/time to start temp request cycle
os_timer_t  oneWireReadyTimer; // once request cycle starts, this timer set so we can send when ready
os_timer_t  oneWireScanTimer; // timer to scan for new devices
bool readyToRequest1Wire = false;
bool readyToRead1Wire = false;
bool readyToScan1Wire = false;


void setup1Wire() {
  sensors.begin();
  
  sensors.setWaitForConversion(false);

  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  Serial.print("1Wire Device precision currently: ");
  Serial.print(sensors.getResolution());
  Serial.print(" setting to ");
  Serial.print(TEMPERATURE_PRECISION);
  sensors.setResolution(TEMPERATURE_PRECISION);
  Serial.println(" Done!");

  Serial.print("Starting oneWire polling timer at: ");
  Serial.print(oneWireReadDelay);
  Serial.println("ms");

  Serial.print("Starting oneWire scanning timer at: ");
  Serial.print(oneWireScanDelay);
  Serial.println("ms");

  Serial.println("Scanning OneWire Bus");

  oneWireScanBus();

  os_timer_setfn(&oneWireRequestTimer, interuptRequest1WSensors, NULL);
  os_timer_setfn(&oneWireReadyTimer, interuptReady1WSensors, NULL);
  os_timer_setfn(&oneWireScanTimer, interuptScan1WSensors, NULL);



  os_timer_arm(&oneWireRequestTimer, oneWireReadDelay, true);
  os_timer_arm(&oneWireScanTimer, oneWireScanDelay, true);



}

//called once every loop()
void handle1Wire() {

  // If it's time to request temps, well request it...
  if (readyToRequest1Wire) {
    digitalWrite(LED_BUILTIN, LOW);
    request1WSensors();
    digitalWrite(LED_BUILTIN, HIGH);  
  }

  //ready to send temps! 
  if (readyToRead1Wire) {
    digitalWrite(LED_BUILTIN, LOW);
    read1WSensors();
    digitalWrite(LED_BUILTIN, HIGH);      
  }

  if (readyToScan1Wire) {
    digitalWrite(LED_BUILTIN, LOW);
    oneWireScanBus();
    digitalWrite(LED_BUILTIN, HIGH);       
  }

}

void setOneWireReadDelay(uint32_t newDelay) {
  os_timer_disarm(&oneWireRequestTimer);
  Serial.print("Restarting oneWire polling timer at: ");
  Serial.print(oneWireReadDelay);  
  Serial.println("ms");
  oneWireReadDelay = newDelay;
  os_timer_arm(&oneWireRequestTimer, oneWireReadDelay, true);
}

// debug function to print a device address
void printAddress(DeviceAddress deviceAddress) {
 for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) {
      Serial.print(":");
    }
  }
}

void interuptRequest1WSensors(void *pArg) {
  readyToRequest1Wire = true;
}

void interuptReady1WSensors(void *pArg) {
  readyToRead1Wire = true;
} 

void interuptScan1WSensors(void *pArg) {
  readyToScan1Wire = true;
} 

void request1WSensors() {
  readyToRequest1Wire = false; // reset interupt

  sensors.requestTemperatures();

  // start ready timer
  os_timer_arm(&oneWireReadyTimer, ONEWIRE_READ_DELAY, false); // false = no loop
  
}

void read1WSensors() {
  readyToRead1Wire = false; // reset interupt
  SensorInfo *thisSensorInfo;
  DeviceAddress address;
  float tempK;
  float tempC;

  for (uint8_t i=0; i < sensorList.size(); i++) {
    thisSensorInfo = sensorList.get(i);
    if (strcmp(thisSensorInfo->type, "oneWire") == 0) {
      parseBytes(thisSensorInfo->address, ':', address,  8, 16); // convert string address to uint_8 array
    
      tempC = sensors.getTempC(address);
      if (tempC == DEVICE_DISCONNECTED) {
        thisSensorInfo->valueJson[0] = "null";
      } else {
        tempK = tempC + 273.15;
        thisSensorInfo->valueJson[0] = tempK;
      } 
      thisSensorInfo->isUpdated = true;
    }

  }
}


void oneWireScanBus() {
  readyToScan1Wire = false; // reset Interupt

  uint8_t tempDeviceAddress[8];
  char strAddress[32];

  int numberOfDevices = 0;

  sensors.begin(); //needed so the library searches for new sensors that came up since boot

  numberOfDevices = sensors.getDeviceCount();
  SensorInfo *tmpSensorInfo;

  for(int i=0;i<numberOfDevices; i++) {
    if(sensors.getAddress(tempDeviceAddress, i))
    {

      // convert to string
        sprintf(strAddress, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
        tempDeviceAddress[0], 
        tempDeviceAddress[1], 
        tempDeviceAddress[2],
        tempDeviceAddress[3], 
        tempDeviceAddress[4], 
        tempDeviceAddress[5], 
        tempDeviceAddress[6], 
        tempDeviceAddress[7]  );      
      
      //see if it's in sensorInfo
      bool known = false;
      for (int x=0;x<sensorList.size() ; x++) {
        tmpSensorInfo = sensorList.get(x);
        if (strcmp(tmpSensorInfo->address, strAddress) == 0) {
          known = true;                
        }
        
      }
      if (!known) {
        Serial.print("New Sensor found: ");
        Serial.print(strAddress);
        Serial.println("");        
        SensorInfo *newSensor = new SensorInfo();
        strcpy(newSensor->address, strAddress);
        newSensor->signalKPath[0] = "";
        newSensor->attrName[0] = "tempK";
        strcpy(newSensor->type,"oneWire");
        newSensor->valueJson[0] = "{ \"tempK\": null }";
        sensorList.add(newSensor);
        saveConfig();
      }

    }
  }
  
}


