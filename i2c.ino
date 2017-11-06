
int8_t sensorSHT = DEVICE_DISCONNECTED;


os_timer_t  sensorSHTPollTimer; // repeating timer that fires ever X/time to start poll cycle
os_timer_t  sensorSHTReadTimer; // once request cycle starts, this timer set so we can send when ready

uint16_t sensorSHTReadDelay = 5000; //ms between reading
bool readytoPollSHT = false;
bool readytoReadSHT = false;

void setupI2C() {
  scanI2C();

  if (sensorSHT != DEVICE_DISCONNECTED) {
    os_timer_setfn(&sensorSHTPollTimer, interuptSHTPoll, NULL);
    os_timer_setfn(&sensorSHTReadTimer, interuptSHTRead, NULL);
    
    os_timer_arm(&sensorSHTPollTimer, sensorSHTReadDelay, true);
  }
  
}

void handleI2C() {
  if (readytoPollSHT){
    digitalWrite(LED_BUILTIN, LOW);
    pollSHT();
    digitalWrite(LED_BUILTIN, HIGH);  
  }
  if (readytoReadSHT){
    digitalWrite(LED_BUILTIN, LOW);
    readSHT();
    digitalWrite(LED_BUILTIN, HIGH);  
  }

  
}

void interuptSHTPoll(void *pArg) {
  readytoPollSHT = true;
}

void interuptSHTRead(void *pArg) {
  readytoReadSHT = true;
}





void pollSHT() {
  readytoPollSHT = false; //reset interupt
  Wire.beginTransmission(sensorSHT);
  Wire.write(0x2C);
  Wire.write(0x06);
  // Stop I2C transmission
  Wire.endTransmission();

  os_timer_arm(&sensorSHTReadTimer, 100, false); // prepare to read
}

void readSHT() {
  uint8_t data[6];
  
  readytoReadSHT = false; //reset interupt
  Wire.requestFrom(sensorSHT, 6);
// Read 6 bytes of data
  // cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc
  if (Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }

  // Convert the data
  float cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
  float fTemp = (cTemp * 1.8) + 32;
  float humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

  Serial.print("Relative Humidity : ");
  Serial.print(humidity);
  Serial.println(" %RH");
  Serial.print("Temperature in Celsius : ");
  Serial.print(cTemp);
  Serial.println(" C");
  Serial.print("Temperature in Fahrenheit : ");
  Serial.print(fTemp);
  Serial.println(" F");  
  
}











void scanI2C() {
  Serial.println("Scanning for i2c Sensors...");

  if (scanI2CAddress(0x45)) { //Sht on D1 sheild
    Serial.println("Found SHT30 at 0x45");
    sensorSHT = 0x45;
  }


}


bool scanI2CAddress(uint8_t address) {
  Wire.begin();

  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();

  if (errorCode == 0) {
    return true;
  }
  return false;
}





