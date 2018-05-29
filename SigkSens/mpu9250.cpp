extern "C" {
#include "user_interface.h"
}

#include <FS.h>

#include "config.h"

#include <Wire.h>

#include "arduino.h"

#include "sigksens.h"
#include "quaternionFilters.h"
#include "mpu9250.h"

MPU9250SensorInfo::MPU9250SensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = "";
  signalKPath[1] = "";
  signalKPath[2] = "";
  signalKPath[3] = "";
  signalKPath[4] = "";
  attrName[0] = "tempK";
  attrName[1] = "yaw";
  attrName[2] = "pitch";
  attrName[3] = "roll";
  attrName[4] = "filterRate";
  type = SensorType::mpu925x;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  valueJson[3] = "null";
  valueJson[4] = "null";

  offset[0] = 0;
  offset[1] = 0;
  offset[2] = 0;
  offset[3] = 0;
  offset[4] = 0;

  scale[0] = 1;
  scale[1] = 1;
  scale[2] = 1;
  scale[3] = 1;
  scale[4] = 1;

  isUpdated = false;
}

MPU9250SensorInfo::MPU9250SensorInfo(String addr, 
                                      String path1, String path2, String path3, String path4, String path5,
                                      float offset0, float offset1, float offset2, float offset3, float offset4,
                                      float scale0, float scale1, float scale2, float scale3, float scale4) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  signalKPath[2] = path3;
  signalKPath[3] = path4;
  signalKPath[4] = path5;
  attrName[0] = "tempK";
  attrName[1] = "yaw";
  attrName[2] = "pitch";
  attrName[3] = "roll";
  attrName[4] = "filterRate";
  type = SensorType::mpu925x;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";
  valueJson[3] = "null";
  valueJson[4] = "null";

  offset[0] = offset0;
  offset[1] = offset1;
  offset[2] = offset2;
  offset[3] = offset3;
  offset[4] = offset4;

  scale[0] = scale0;
  scale[1] = scale1;
  scale[2] = scale2;
  scale[3] = scale3;
  scale[4] = scale4;

  isUpdated = false;
}

MPU9250SensorInfo *MPU9250SensorInfo::fromJson(JsonObject &jsonSens) {
  return new MPU9250SensorInfo(
    jsonSens["address"],

    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][2]["signalKPath"],
    jsonSens["attrs"][3]["signalKPath"],
    jsonSens["attrs"][4]["signalKPath"],
    jsonSens["attrs"][0]["offset"],
    jsonSens["attrs"][1]["offset"],
    jsonSens["attrs"][2]["offset"],
    jsonSens["attrs"][3]["offset"],
    jsonSens["attrs"][4]["offset"],
    jsonSens["attrs"][0]["scale"],
    jsonSens["attrs"][1]["scale"],
    jsonSens["attrs"][2]["scale"],
    jsonSens["attrs"][3]["scale"],
    jsonSens["attrs"][4]["scale"]);

}

void MPU9250SensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::mpu925x;
  JsonArray& jsonAttrs = jsonSens.createNestedArray("attrs");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    JsonObject& attr = jsonAttrs.createNestedObject();
    attr["name"] = attrName[x];
    attr["signalKPath"] = signalKPath[x];
    attr["offset"] = offset[x];
    attr["scale"] = scale[x];
    attr["value"] = valueJson[x];
  }  
}




// Specify sensor full scale
uint8_t OSR = ADC_8192;     // set pressure amd temperature oversample rate
uint8_t Gscale = GFS_250DPS;
uint8_t Ascale = AFS_2G;
uint8_t Mscale = MFS_16BITS; // Choose either 14-bit or 16-bit magnetometer resolution
uint8_t Mmode = 0x06;        // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
float aRes, gRes, mRes;      // scale resolutions per LSB for the sensors
  
bool newMagData = false;



int16_t MPU9250Data[7]; // used to read all 14 bytes at once from the MPU9250 accel/gyro
int16_t accelCount[3];  // Stores the 16-bit signed accelerometer sensor output
int16_t gyroCount[3];   // Stores the 16-bit signed gyro sensor output
int16_t magCount[3];    // Stores the 16-bit signed magnetometer sensor output
float magCalibration[3] = {0, 0, 0};  // Factory mag calibration and mag bias
float magBias[3] = {0, 0, 0}, magScale[3]  = {1, 1, 1};      // Bias corrections for gyro and accelerometer
int32_t gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
int16_t tempCount;            // temperature raw count output
float   temperature;          // Stores the MPU9250 gyro internal chip temperature in degrees Celsius
float SelfTest[6];            // holds results of gyro and accelerometer self test

uint32_t delt_t = 0, count = 0;  // used to control display output rate
float pitch, yaw, roll;
float a12, a22, a31, a32, a33;            // rotation matrix coefficients for Euler angles and gravity components

uint32_t lastUpdate = 0, firstUpdate = 0; // used to calculate integration interval
uint32_t Now = 0;                         // used to calculate integration interval

float filterIntegrationInterval = 0.0f;
uint32_t filterIntegrationCount = 0;

float ax, ay, az, gx, gy, gz, mx, my, mz; // variables to hold latest sensor data values 
float lin_ax, lin_ay, lin_az;             // linear acceleration (acceleration with gravity component subtracted)

enum MPUVersion {
  MPU9250 = 0,
  MPU9255
};

uint8_t MPUVersion;
float myPI = 3.14159265359f;

// forward declarations

void MPU9250SelfTest(float * destination);
void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
uint8_t readByte(uint8_t address, uint8_t subAddress);
void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest);
void getMres();
void getGres();
void getAres();
void saveMPUCalibrationFS();
void loadMPUCalibrationFS();
void readMPU9250Data(int16_t * destination);
int16_t readTempData();
bool testMPU9250();
bool testAK8963();
void initAK8963(float * destination);
void initMPU9250();
void magCalMPU9250Start();
void magCalMPU9250Run();
void magcalMPU9250Stop();
//void magcalMPU9250(float * dest1, float * dest2);
void accelgyrocalMPU9250();
void loadAccelAndGyroBiases();
void readMagData(int16_t * destination);

bool MPUisValid = false;
bool AK8963isValid = false;
bool mpuUpdateReady = false;
volatile bool newData = false;
MpuRunMode mpuRunMode = MpuRunMode::mpuOff;

//variables for Mag Calibration
int16_t magCalMax[3] = {-32767, -32767, -32767};
int16_t magCalMin[3] = {32767, 32767, 32767};

/* ---------------------------------------------------------------------------------------------
   ---------------------------------------------------------------------------------------------
   ---------------------------------------------------------------------------------------------
   --------------------------------------------------------------------------------------------- */

void ICACHE_RAM_ATTR interruptMPUNewData() {
  newData = true;
}

void setupMPU9250() {
  MPUisValid = testMPU9250();
  

  if (MPUisValid) {
    loadMPUCalibrationFS(); //load calibration from FS
    loadAccelAndGyroBiases(); // load calibration into MPU

    initMPU9250(); 
    Serial.println("MPU925X initialized...");

    AK8963isValid = testAK8963();
    if (AK8963isValid) {
      initAK8963(magCalibration); 
      Serial.println("AK8963 initialized... Mag hardware cablibration");
      Serial.print("X-Axis sensitivity adjustment value "); Serial.println(magCalibration[0], 2);
      Serial.print("Y-Axis sensitivity adjustment value "); Serial.println(magCalibration[1], 2);
      Serial.print("Z-Axis sensitivity adjustment value "); Serial.println(magCalibration[2], 2);
      // define interrupt for INT pin output of MPU9250
      attachInterrupt(MPU_INTERRUPT_PIN, interruptMPUNewData, RISING); 
      Serial.println("Interrupts setup");
    }
    if (MPUisValid && AK8963isValid) {
      mpuRunMode = MpuRunMode::mpuRun;
    }
  }
  app.onTick(&handleMPU9250);
  app.repeat(SLOW_LOOP_DELAY, &updateMPUSensorInfo);
}

void handleMPU9250() {
  switch(mpuRunMode) {
    case MpuRunMode::mpuRun:
      if(newData) { //newData is from pin Interrupt
        newData = false; // reset newData flag
        processMPU9250();
      }

      updateQuaternion();
      break;

    case MpuRunMode::calAccelGyro:
      accelgyrocalMPU9250();
      loadAccelAndGyroBiases();
      saveMPUCalibrationFS();
      initMPU9250(); 
      mpuRunMode = MpuRunMode::mpuRun;
      break;

    case MpuRunMode::calMagStart:
      magCalMPU9250Start();
      mpuRunMode = MpuRunMode::calMagRun;
      break;

    case MpuRunMode::calMagRun:
      magCalMPU9250Run();
      mpuRunMode = MpuRunMode::calMagRun;
      break;

    case MpuRunMode::calMagStop:
      magcalMPU9250Stop();
      saveMPUCalibrationFS();
      mpuRunMode = MpuRunMode::mpuRun;
      break;
  }
}

void saveMPUCalibrationFS(){
  Serial.println("Saving MPU Calibrations");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  JsonArray& jsonCalGryoBias = json.createNestedArray("gyroBias");
  JsonArray& jsonCalAccelBias = json.createNestedArray("accelBias");
  JsonArray& jsonCalMagBias = json.createNestedArray("magBias");
  JsonArray& jsonCalMagScale = json.createNestedArray("magScale");
  jsonCalGryoBias.add(gyroBias[0]); jsonCalGryoBias.add(gyroBias[1]); jsonCalGryoBias.add(gyroBias[2]);
  jsonCalAccelBias.add(accelBias[0]); jsonCalAccelBias.add(accelBias[1]); jsonCalAccelBias.add(accelBias[2]);
  jsonCalMagBias.add(magBias[0]); jsonCalMagBias.add(magBias[1]); jsonCalMagBias.add(magBias[2]);
  jsonCalMagScale.add(magScale[0]); jsonCalMagScale.add(magScale[1]); jsonCalMagScale.add(magScale[2]);

  File configFile = SPIFFS.open("/mpuCal.json", "w");
  if (!configFile) {
    Serial.println("failed to open MPU Calibration file for writing");
  }
  json.prettyPrintTo(Serial);
  json.printTo(configFile);
  configFile.close();
}


void loadMPUCalibrationFS() {
  Serial.println("Loading MPU Calibration.");
  if (!SPIFFS.exists("/mpuCal.json")) {
    return; // no calibrations to load
    Serial.println("No Calibration found");
  }

  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/mpuCal.json", "r");
  if (configFile) {
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
      
    JsonObject& json = jsonBuffer.parseObject(buf.get());

    if (json.success()) {
      gyroBias[0] = json["gyroBias"][0];
      gyroBias[1] = json["gyroBias"][1];
      gyroBias[2] = json["gyroBias"][2];

      accelBias[0] = json["accelBias"][0];
      accelBias[1] = json["accelBias"][1];
      accelBias[2] = json["accelBias"][2];

      magBias[0] = json["magBias"][0];
      magBias[1] = json["magBias"][1];
      magBias[2] = json["magBias"][2];
    
      magScale[0] = json["magScale"][0];
      magScale[1] = json["magScale"][1];
      magScale[2] = json["magScale"][2];
    }
  }
  Serial.print("accel biases (ug) : "); 
  Serial.print(accelBias[0]); Serial.print(", "); 
  Serial.print(accelBias[1]); Serial.print(", "); 
  Serial.println(accelBias[2]);
  
  Serial.print("gyro biases (dps) : "); 
  Serial.print(gyroBias[0]); Serial.print(", ");  
  Serial.print(gyroBias[1]); Serial.print(", "); 
  Serial.println(gyroBias[2]);

  Serial.print("mag biases (mG) : "); 
  Serial.print(magBias[0]); Serial.print(", ");
  Serial.print(magBias[1]); Serial.print(", ");
  Serial.println(magBias[2]); 
  
  Serial.print("mag scale (mG) : "); 
  Serial.print(magScale[0]); Serial.print(", "); 
  Serial.print(magScale[1]); Serial.print(", "); 
  Serial.println(magScale[2]); 
 

}

void runAccelGyroCal() {
  Serial.println("Starting Accel and Gyro ");
  mpuRunMode = MpuRunMode::calAccelGyro;
    
}

void runMagCalStart() {
  Serial.println("Starting Magnometer Calibration...");
  mpuRunMode = MpuRunMode::calMagStart;
}

void runMagCalStop() {
  Serial.println("Stoping Magnometer Calibration...");
  mpuRunMode = MpuRunMode::calMagStop;
}


/* ---------------------------------------------------------------------------------------------
   ---------------------------------------------------------------------------------------------
   ---------------------------------------------------------------------------------------------
   --------------------------------------------------------------------------------------------- */

bool testMPU9250() {
 // Read the WHO_AM_I register, this is a good test of communication
  Serial.println("Testing MPU9250 9-axis motion sensor...");
  byte c = readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);  // Read WHO_AM_I register for MPU-9250

  if (c == 0x71) {
    MPUVersion = MPU9250;
  } else if (c == 0x73) {
    MPUVersion = MPU9255;
  } else {
    MPUVersion = MPU9250; //default?
  }

  if ( (c == 0x71) || (c == 0x73) ) //
  {  
    Serial.println("MPU925X is online...");

    MPU9250SelfTest(SelfTest); // Start by performing self test and reporting values
    Serial.print("x-axis self test: acceleration trim within : "); Serial.print(SelfTest[0],1); Serial.println("% of factory value");
    Serial.print("y-axis self test: acceleration trim within : "); Serial.print(SelfTest[1],1); Serial.println("% of factory value");
    Serial.print("z-axis self test: acceleration trim within : "); Serial.print(SelfTest[2],1); Serial.println("% of factory value");
    Serial.print("x-axis self test: gyration trim within : "); Serial.print(SelfTest[3],1); Serial.println("% of factory value");
    Serial.print("y-axis self test: gyration trim within : "); Serial.print(SelfTest[4],1); Serial.println("% of factory value");
    Serial.print("z-axis self test: gyration trim within : "); Serial.print(SelfTest[5],1); Serial.println("% of factory value");
    
    // get sensor resolutions, only need to do this once
    getAres();
    getGres();
    getMres();
    return true;
  } else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);
    return false;
  }

}

bool testAK8963() {
  byte d = readByte(AK8963_ADDRESS, AK8963_WHO_AM_I);  // Read WHO_AM_I register for AK8963
  if (d == 0x48) {
    return true;
  } else {
    Serial.print("AK8963 WHOAMI Register"); Serial.print("I AM "); Serial.print(d, HEX); Serial.print(" I should be "); Serial.println(0x48, HEX);
    return false;
  }
}

void processMPU9250() {  
  readMPU9250Data(MPU9250Data); // INT cleared on any read
    
  // Now we'll calculate the accleration value into actual g's
  ax = (float)MPU9250Data[0]*aRes; // - accelBias[0];  // get actual g value, this depends on scale being set
  ay = (float)MPU9250Data[1]*aRes; // - accelBias[1];   
  az = (float)MPU9250Data[2]*aRes; // - accelBias[2];  
   
  // Calculate the gyro value into actual degrees per second
  gx = (float)MPU9250Data[4]*gRes;  // get actual gyro value, this depends on scale being set
  gy = (float)MPU9250Data[5]*gRes;  
  gz = (float)MPU9250Data[6]*gRes;   
  
  readMagData(magCount);  // Read the x/y/z adc values
   
  // Calculate the magnetometer values in milliGauss
  // Include factory calibration per data sheet and user environmental corrections
  if(newMagData == true) {
    newMagData = false; // reset newMagData flag

    mx = (float)magCount[0]*mRes*magCalibration[0] - magBias[0];  // get actual magnetometer value, this depends on scale being set
    my = (float)magCount[1]*mRes*magCalibration[1] - magBias[1];  
    mz = (float)magCount[2]*mRes*magCalibration[2] - magBias[2];  
    mx *= magScale[0];
    my *= magScale[1];
    mz *= magScale[2]; 
  } 
}

void updateQuaternion() {
  Now = micros();
  deltat = ((Now - lastUpdate)/1000000.0f); // set integration time by time elapsed since last filter update
  lastUpdate = Now;

  filterIntegrationInterval += deltat; // sum for averaging filter update rate
  filterIntegrationCount++;
  
  // Sensors x (y)-axis of the accelerometer/gyro is aligned with the y (x)-axis of the magnetometer;
  // the magnetometer z-axis (+ down) is misaligned with z-axis (+ up) of accelerometer and gyro!
  // We have to make some allowance for this orientation mismatch in feeding the output to the quaternion filter.
  // For the MPU9250+MS5637 Mini breakout the +x accel/gyro is North, then -y accel/gyro is East. So if we want te quaternions properly aligned
  // we need to feed into the Madgwick function Ax, -Ay, -Az, Gx, -Gy, -Gz, My, -Mx, and Mz. But because gravity is by convention
  // positive down, we need to invert the accel data, so we pass -Ax, Ay, Az, Gx, -Gy, -Gz, My, -Mx, and Mz into the Madgwick
  // function to get North along the accel +x-axis, East along the accel -y-axis, and Down along the accel -z-axis.
  // This orientation choice can be modified to allow any convenient (non-NED) orientation convention.
  // Pass gyro rate as rad/s
  //MadgwickQuaternionUpdate(-ay, -ax, -az, gx*myPI/180.0f, -gy*myPI/180.0f, -gz*myPI/180.0f,  my,  -mx, mz);

   // MadgwickQuaternionUpdate(-ax, ay, az, gx*myPI/180.0f, -gy*myPI/180.0f, -gz*myPI/180.0f,  my,  -mx, mz);
   MahonyQuaternionUpdate(-ax, ay, az, gx*myPI/180.0f, -gy*myPI/180.0f, -gz*myPI/180.0f,  my,  -mx, mz);
}


void updateMPUSensorInfo() {
  SensorInfo *thisSensorInfo;
  uint8_t address;
  
  if (mpuRunMode != MpuRunMode::mpuRun) {
    return;
  }

  tempCount = readTempData();  // Read the gyro adc values
  temperature = ((float) tempCount) / 333.87 + 21.0; // Gyro chip temperature in degrees Centigrade
  // Print temperature in degrees Centigrade      
 

  
  // Define output variables from updated quaternion---these are Tait-Bryan angles, commonly used in aircraft orientation.
  // In this coordinate system, the positive z-axis is down toward Earth. 
  // Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
  // Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
  // Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
  // These arise from the definition of the homogeneous rotation matrix constructed from quaternions.
  // Tait-Bryan angles as well as Euler angles are non-commutative; that is, the get the correct orientation the rotations must be
  // applied in the correct order which for this configuration is yaw, pitch, and then roll.
  // For more see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles which has additional links.
  //Software AHRS:
  //   yaw   = atan2f(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);   
  //   pitch = -asinf(2.0f * (q[1] * q[3] - q[0] * q[2]));
  //   roll  = atan2f(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);
  //   pitch *= 180.0f / PI;
  //   yaw   *= 180.0f / PI; 
  //   yaw   += 13.8f; // Declination at Danville, California is 13 degrees 48 minutes and 47 seconds on 2014-04-04
  //   if(yaw < 0) yaw   += 360.0f; // Ensure yaw stays between 0 and 360
  //   roll  *= 180.0f / PI;
  a12 =   2.0f * (q[1] * q[2] + q[0] * q[3]);
  a22 =   q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3];
  a31 =   2.0f * (q[0] * q[1] + q[2] * q[3]);
  a32 =   2.0f * (q[1] * q[3] - q[0] * q[2]);
  a33 =   q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
  pitch = -asinf(a32);
  roll  = atan2f(a31, a33);
  yaw   = atan2f(a12, a22);
  pitch *= 180.0f / myPI;
  yaw   *= 180.0f / myPI; 
  //    yaw   += 13.8f; // Declination at Danville, California is 13 degrees 48 minutes and 47 seconds on 2014-04-04
  if(yaw < 0) yaw   += 360.0f; // Ensure yaw stays between 0 and 360
  roll  *= 180.0f / myPI;
  lin_ax = ax + a31;
  lin_ay = ay + a32;
  lin_az = az - a33;
  sensorStorage[(int)SensorType::mpu925x].forEach([&](SensorInfo* si) {
    si->valueJson[0] = temperature + 273.15;
    si->valueJson[1] = String(yaw * (myPI/180.0f),4);
    si->valueJson[2] = String(pitch * (myPI/180.0f),4);
    si->valueJson[3] = String(roll * (myPI/180.0f),4);
    si->valueJson[4] = String((int)filterIntegrationCount/filterIntegrationInterval);
    si->isUpdated = true;    
  });

  //if(SerialDebug) {
    //Serial.print("ax = "); Serial.print((int)1000*ax);  
    //Serial.print(" ay = "); Serial.print((int)1000*ay); 
    //Serial.print(" az = "); Serial.print((int)1000*az); Serial.println(" mg");
    //Serial.print("gx = "); Serial.print( gx, 2); 
    //Serial.print(" gy = "); Serial.print( gy, 2); 
    //Serial.print(" gz = "); Serial.print( gz, 2); Serial.println(" deg/s");
    //Serial.print("mx = "); Serial.print( (int)mx ); 
    //Serial.print(" my = "); Serial.print( (int)my ); 
    //Serial.print(" mz = "); Serial.print( (int)mz ); Serial.println(" mG");
    
    //Serial.print("q0 = "); Serial.print(q[0]);
    //Serial.print(" qx = "); Serial.print(q[1]); 
    //Serial.print(" qy = "); Serial.print(q[2]); 
    //Serial.print(" qz = "); Serial.println(q[3]); 
    //Serial.print("Gyro temperature is ");  Serial.print(temperature, 1);  Serial.println(" degrees C"); // Print T values to tenths of s degree C
    
    //Serial.print("Yaw, Pitch, Roll: ");
    //Serial.print(yaw, 2);
    //Serial.print(", ");
    //Serial.print(pitch, 2);
    //Serial.print(", ");
    //Serial.print(roll, 2);
    //Serial.print(" rate = "); Serial.print((float)sumCount/sum, 2); Serial.println(" Hz");
    //Serial.print("Grav_x, Grav_y, Grav_z: ");
    //Serial.print(-a31*1000, 2);
    //Serial.print(", ");
    //Serial.print(-a32*1000, 2);
    //Serial.print(", ");
    //Serial.print(a33*1000, 2);  Serial.println(" mg");
    //Serial.print("Lin_ax, Lin_ay, Lin_az: ");
    //Serial.print(lin_ax*1000, 2);
    //Serial.print(", ");
    //Serial.print(lin_ay*1000, 2);
    //Serial.print(", ");
    //Serial.print(lin_az*1000, 2);  Serial.println(" mg");
    
  //}
  count = millis(); 
  filterIntegrationCount = 0;
  filterIntegrationInterval = 0;    
}



//===================================================================================================================
//====== Set of useful function to access acceleration. gyroscope, magnetometer, and temperature data
//===================================================================================================================

void getMres() {
  switch (Mscale)
  {
  // Possible magnetometer scales (and their register bit settings) are:
  // 14 bit resolution (0) and 16 bit resolution (1)
    case MFS_14BITS:
          mRes = 10.*4912./8190.; // Proper scale to return milliGauss
          break;
    case MFS_16BITS:
          mRes = 10.*4912./32760.0; // Proper scale to return milliGauss
          break;
  }
}

void getGres() {
  switch (Gscale)
  {
  // Possible gyro scales (and their register bit settings) are:
  // 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11). 
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case GFS_250DPS:
          gRes = 250.0/32768.0;
          break;
    case GFS_500DPS:
          gRes = 500.0/32768.0;
          break;
    case GFS_1000DPS:
          gRes = 1000.0/32768.0;
          break;
    case GFS_2000DPS:
          gRes = 2000.0/32768.0;
          break;
  }
}

void getAres() {
  switch (Ascale)
  {
  // Possible accelerometer scales (and their register bit settings) are:
  // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11). 
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case AFS_2G:
          aRes = 2.0/32768.0;
          break;
    case AFS_4G:
          aRes = 4.0/32768.0;
          break;
    case AFS_8G:
          aRes = 8.0/32768.0;
          break;
    case AFS_16G:
          aRes = 16.0/32768.0;
          break;
  }
}

void readMPU9250Data(int16_t * destination) { // reads both accel and gyro
  uint8_t rawData[14];  // x/y/z accel register data stored here
  readBytes(MPU9250_ADDRESS, ACCEL_XOUT_H, 14, &rawData[0]);  // Read the 14 raw data registers into data array
  destination[0] = ((int16_t)rawData[0] << 8) | rawData[1] ;  // Turn the MSB and LSB into a signed 16-bit value
  destination[1] = ((int16_t)rawData[2] << 8) | rawData[3] ;  
  destination[2] = ((int16_t)rawData[4] << 8) | rawData[5] ; 
  destination[3] = ((int16_t)rawData[6] << 8) | rawData[7] ;   
  destination[4] = ((int16_t)rawData[8] << 8) | rawData[9] ;  
  destination[5] = ((int16_t)rawData[10] << 8) | rawData[11] ;  
  destination[6] = ((int16_t)rawData[12] << 8) | rawData[13] ; 
}

void readMagData(int16_t * destination) {
  uint8_t rawData[7];  // x/y/z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition
  newMagData = (readByte(AK8963_ADDRESS, AK8963_ST1) & 0x01);
  if(newMagData == true) { // wait for magnetometer data ready bit to be set
  readBytes(AK8963_ADDRESS, AK8963_XOUT_L, 7, &rawData[0]);  // Read the six raw data and ST2 registers sequentially into data array
  uint8_t c = rawData[6]; // End data read by reading ST2 register
    if(!(c & 0x08)) { // Check if magnetic sensor overflow set, if not then report data
    destination[0] = ((int16_t)rawData[1] << 8) | rawData[0] ;  // Turn the MSB and LSB into a signed 16-bit value
    destination[1] = ((int16_t)rawData[3] << 8) | rawData[2] ;  // Data stored as little Endian
    destination[2] = ((int16_t)rawData[5] << 8) | rawData[4] ; 
   }
  }
}

int16_t readTempData() {
  uint8_t rawData[2];  // x/y/z gyro register data stored here
  readBytes(MPU9250_ADDRESS, TEMP_OUT_H, 2, &rawData[0]);  // Read the two raw data registers sequentially into data array 
  return ((int16_t)rawData[0] << 8) | rawData[1] ;  // Turn the MSB and LSB into a 16-bit value
}
       
void initAK8963(float * destination) {
  // First extract the factory calibration for each magnetometer axis
  uint8_t rawData[3];  // x/y/z gyro calibration data stored here
  writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer  
  delay(10);
  writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
  delay(10);
  readBytes(AK8963_ADDRESS, AK8963_ASAX, 3, &rawData[0]);  // Read the x-, y-, and z-axis calibration values
  destination[0] =  (float)(rawData[0] - 128)/256. + 1.;   // Return x-axis sensitivity adjustment values, etc.
  destination[1] =  (float)(rawData[1] - 128)/256. + 1.;  
  destination[2] =  (float)(rawData[2] - 128)/256. + 1.; 
  writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer  
  delay(10);
  // Configure the magnetometer for continuous read and highest resolution
  // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
  // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
  writeByte(AK8963_ADDRESS, AK8963_CNTL, Mscale << 4 | Mmode); // Set magnetometer data resolution and sample ODR
  delay(10);
}


void initMPU9250() {  
  // wake up device
  writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00); // Clear sleep mode bit (6), enable all sensors 
  delay(100); // Wait for all registers to reset 

  // get stable time source
  writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);  // Auto select clock source to be PLL gyroscope reference if ready else
  delay(200); 
  
  // Configure Gyro and Thermometer
  // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz, respectively; 
  // minimum delay time for this setting is 5.9 ms, which means sensor fusion update rates cannot
  // be higher than 1 / 0.0059 = 170 Hz
  // DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
  // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!), 8 kHz, or 1 kHz
  writeByte(MPU9250_ADDRESS, CONFIG, 0x03);  

  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  writeByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x04);  // Use a 200 Hz rate; a rate consistent with the filter update rate 
                                    // determined inset in CONFIG above
 
  // Set gyroscope full scale range
  // Range selects FS_SEL and GFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
  uint8_t c = readByte(MPU9250_ADDRESS, GYRO_CONFIG); // get current GYRO_CONFIG register value
  // c = c & ~0xE0; // Clear self-test bits [7:5] 
  c = c & ~0x03; // Clear Fchoice bits [1:0] 
  c = c & ~0x18; // Clear GFS bits [4:3]
  c = c | Gscale << 3; // Set full scale range for the gyro
  // c =| 0x00; // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of GYRO_CONFIG
  writeByte(MPU9250_ADDRESS, GYRO_CONFIG, c ); // Write new GYRO_CONFIG value to register
  
  // Set accelerometer full-scale range configuration
  c = readByte(MPU9250_ADDRESS, ACCEL_CONFIG); // get current ACCEL_CONFIG register value
  // c = c & ~0xE0; // Clear self-test bits [7:5] 
  c = c & ~0x18;  // Clear AFS bits [4:3]
  c = c | Ascale << 3; // Set full scale range for the accelerometer 
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, c); // Write new ACCEL_CONFIG register value

  // Set accelerometer sample rate configuration
  // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
  // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
  c = readByte(MPU9250_ADDRESS, ACCEL_CONFIG2); // get current ACCEL_CONFIG2 register value
  c = c & ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])  
  c = c | 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG2, c); // Write new ACCEL_CONFIG2 register value
  
  // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates, 
  // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

  // Configure Interrupts and Bypass Enable
  // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH until interrupt cleared,
  // clear on read of INT_STATUS, and enable I2C_BYPASS_EN so additional chips 
  // can join the I2C bus and all can be controlled by the Arduino as master
  //   writeByte(MPU9250_ADDRESS, INT_PIN_CFG, 0x22);    
  writeByte(MPU9250_ADDRESS, INT_PIN_CFG, 0x12);  // INT is 50 microsecond pulse and any read to clear  
  writeByte(MPU9250_ADDRESS, INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
  delay(100);
}


// Function which accumulates gyro and accelerometer data after device initialization. It calculates the average
// of the at-rest readings and then loads the resulting offsets into accelerometer and gyro bias registers.
void accelgyrocalMPU9250() {  
 
  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii, packet_count, fifo_count;
  int32_t gyro_bias[3]  = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
  
  // reset device
  writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
  delay(100);
   
  // get stable time source; Auto select clock source to be PLL gyroscope reference if ready 
  // else use the internal oscillator, bits 2:0 = 001
  writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);  
  writeByte(MPU9250_ADDRESS, PWR_MGMT_2, 0x00);
  delay(200);                                    

  // Configure device for bias calculation
  writeByte(MPU9250_ADDRESS, INT_ENABLE, 0x00);   // Disable all interrupts
  writeByte(MPU9250_ADDRESS, FIFO_EN, 0x00);      // Disable FIFO
  writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);   // Turn on internal clock source
  writeByte(MPU9250_ADDRESS, I2C_MST_CTRL, 0x00); // Disable I2C master
  writeByte(MPU9250_ADDRESS, USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
  writeByte(MPU9250_ADDRESS, USER_CTRL, 0x0C);    // Reset FIFO and DMP
  delay(15);
  
  // Configure MPU6050 gyro and accelerometer for bias calculation
  writeByte(MPU9250_ADDRESS, CONFIG, 0x01);      // Set low-pass filter to 188 Hz
  writeByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
  writeByte(MPU9250_ADDRESS, GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity
 
  uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
  uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

  // Configure FIFO to capture accelerometer and gyro data for bias calculation
  writeByte(MPU9250_ADDRESS, USER_CTRL, 0x40);   // Enable FIFO  
  writeByte(MPU9250_ADDRESS, FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
  delay(40); // accumulate 40 samples in 40 milliseconds = 480 bytes

  // At end of sample accumulation, turn off FIFO sensor read
  writeByte(MPU9250_ADDRESS, FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
  readBytes(MPU9250_ADDRESS, FIFO_COUNTH, 2, &data[0]); // read FIFO sample count
  fifo_count = ((uint16_t)data[0] << 8) | data[1];
  packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging
  Serial.print("Datapoints for bias: "); Serial.println(packet_count);
  
  for (ii = 0; ii < packet_count; ii++) {
    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
    readBytes(MPU9250_ADDRESS, FIFO_R_W, 12, &data[0]); // read data for averaging
    accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
    accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
    accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;    
    gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
    gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
    gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;
    
    accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias[1] += (int32_t) accel_temp[1];
    accel_bias[2] += (int32_t) accel_temp[2];
    gyro_bias[0]  += (int32_t) gyro_temp[0];
    gyro_bias[1]  += (int32_t) gyro_temp[1];
    gyro_bias[2]  += (int32_t) gyro_temp[2];
            
  }
  accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
  accel_bias[1] /= (int32_t) packet_count;
  accel_bias[2] /= (int32_t) packet_count;
  gyro_bias[0]  /= (int32_t) packet_count;
  gyro_bias[1]  /= (int32_t) packet_count;
  gyro_bias[2]  /= (int32_t) packet_count;
    
  if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
  else {accel_bias[2] += (int32_t) accelsensitivity;}


  gyroBias[0] = gyro_bias[0];
  gyroBias[1] = gyro_bias[1];
  gyroBias[2] = gyro_bias[2];
  accelBias[0] = accel_bias[0];
  accelBias[1] = accel_bias[1];
  accelBias[2] = accel_bias[2];

}


void loadAccelAndGyroBiases() {

  int32_t gyro_bias[3], accel_bias[3];	
  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii;

  gyro_bias[0] = gyroBias[0];
  gyro_bias[1] = gyroBias[1];
  gyro_bias[2] = gyroBias[2];
  accel_bias[0] = accelBias[0];
  accel_bias[1] = accelBias[1];
  accel_bias[2] = accelBias[2];
  // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
  data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
  data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
  data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
  data[3] = (-gyro_bias[1]/4)       & 0xFF;
  data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
  data[5] = (-gyro_bias[2]/4)       & 0xFF;
  
  // Push gyro biases to hardware registers
  writeByte(MPU9250_ADDRESS, XG_OFFSET_H, data[0]);
  writeByte(MPU9250_ADDRESS, XG_OFFSET_L, data[1]);
  writeByte(MPU9250_ADDRESS, YG_OFFSET_H, data[2]);
  writeByte(MPU9250_ADDRESS, YG_OFFSET_L, data[3]);
  writeByte(MPU9250_ADDRESS, ZG_OFFSET_H, data[4]);
  writeByte(MPU9250_ADDRESS, ZG_OFFSET_L, data[5]);
  


  // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
  // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
  // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
  // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
  // the accelerometer biases calculated above must be divided by 8.

  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
  readBytes(MPU9250_ADDRESS, XA_OFFSET_H, 2, &data[0]); // Read factory accelerometer trim values
  accel_bias_reg[0] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
  readBytes(MPU9250_ADDRESS, YA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[1] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
  readBytes(MPU9250_ADDRESS, ZA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[2] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
  
  uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis
  
  for(ii = 0; ii < 3; ii++) {
    if((accel_bias_reg[ii] & mask)) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
  }
  
  // Construct total accelerometer bias, including calculated average accelerometer bias from above
  accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
  accel_bias_reg[1] -= (accel_bias[1]/8);
  accel_bias_reg[2] -= (accel_bias[2]/8);
  
  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
  data[1] = (accel_bias_reg[0])      & 0xFF;
  data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
  data[3] = (accel_bias_reg[1])      & 0xFF;
  data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
  data[5] = (accel_bias_reg[2])      & 0xFF;
  data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers
 
  // Apparently this is not working for the acceleration biases in the MPU-9250
  // Are we handling the temperature correction bit properly?
  // Push accelerometer biases to hardware registers
  writeByte(MPU9250_ADDRESS, XA_OFFSET_H, data[0]);
  writeByte(MPU9250_ADDRESS, XA_OFFSET_L, data[1]);
  writeByte(MPU9250_ADDRESS, YA_OFFSET_H, data[2]);
  writeByte(MPU9250_ADDRESS, YA_OFFSET_L, data[3]);
  writeByte(MPU9250_ADDRESS, ZA_OFFSET_H, data[4]);
  writeByte(MPU9250_ADDRESS, ZA_OFFSET_L, data[5]);


}



void magCalMPU9250Start() {
  for (int jj = 0; jj < 3; jj++) {
    magCalMax[jj] = -32767;
    magCalMin[jj] = 32767;
  }
}


void magCalMPU9250Run() {
  int16_t mag_temp[3] = {0, 0, 0};
  readMagData(mag_temp);  // Read the mag data   
  for (int jj = 0; jj < 3; jj++) {
    if(mag_temp[jj] > magCalMax[jj]) magCalMax[jj] = mag_temp[jj];
    if(mag_temp[jj] < magCalMin[jj]) magCalMin[jj] = mag_temp[jj];
  }
}


void magcalMPU9250Stop() {
  int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
 
  // Get hard iron correction
  mag_bias[0]  = (magCalMax[0] + magCalMin[0])/2;  // get average x mag bias in counts
  mag_bias[1]  = (magCalMax[1] + magCalMin[1])/2;  // get average y mag bias in counts
  mag_bias[2]  = (magCalMax[2] + magCalMin[2])/2;  // get average z mag bias in counts
  
  magBias[0] = (float) mag_bias[0]*mRes*magCalibration[0];  // save mag biases in G for main program
  magBias[1] = (float) mag_bias[1]*mRes*magCalibration[1];   
  magBias[2] = (float) mag_bias[2]*mRes*magCalibration[2];  
      
  // Get soft iron correction estimate
  mag_scale[0]  = (magCalMax[0] - magCalMin[0])/2;  // get average x axis max chord length in counts
  mag_scale[1]  = (magCalMax[1] - magCalMin[1])/2;  // get average y axis max chord length in counts
  mag_scale[2]  = (magCalMax[2] - magCalMin[2])/2;  // get average z axis max chord length in counts

  float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
  avg_rad /= 3.0;

  magScale[0] = avg_rad/((float)mag_scale[0]);
  magScale[1] = avg_rad/((float)mag_scale[1]);
  magScale[2] = avg_rad/((float)mag_scale[2]);

  Serial.println("Mag Calibration done!");
  
}

/*  replaced with 3 functions above
void magcalMPU9250(float * dest1, float * dest2) {
  uint16_t ii = 0, sample_count = 0;
  int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
  int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};

  Serial.println("Mag Calibration: Wave device in a figure eight until done!");
  //delay(4000);
  
    // shoot for ~fifteen seconds of mag data
    if(Mmode == 0x02) sample_count = 128;  // at 8 Hz ODR, new mag data is available every 125 ms
    if(Mmode == 0x06) sample_count = 1500;  // at 100 Hz ODR, new mag data is available every 10 ms
   for(ii = 0; ii < sample_count; ii++) {
    readMagData(mag_temp);  // Read the mag data   
    for (int jj = 0; jj < 3; jj++) {
      if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
      if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
    }
    if(Mmode == 0x02) delay(135);  // at 8 Hz ODR, new mag data is available every 125 ms
    if(Mmode == 0x06) delay(12);  // at 100 Hz ODR, new mag data is available every 10 ms
    }

  //    Serial.println("mag x min/max:"); Serial.println(mag_max[0]); Serial.println(mag_min[0]);
  //    Serial.println("mag y min/max:"); Serial.println(mag_max[1]); Serial.println(mag_min[1]);
  //    Serial.println("mag z min/max:"); Serial.println(mag_max[2]); Serial.println(mag_min[2]);

    // Get hard iron correction
    mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
    mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
    mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts
    
    dest1[0] = (float) mag_bias[0]*mRes*magCalibration[0];  // save mag biases in G for main program
    dest1[1] = (float) mag_bias[1]*mRes*magCalibration[1];   
    dest1[2] = (float) mag_bias[2]*mRes*magCalibration[2];  
       
    // Get soft iron correction estimate
    mag_scale[0]  = (mag_max[0] - mag_min[0])/2;  // get average x axis max chord length in counts
    mag_scale[1]  = (mag_max[1] - mag_min[1])/2;  // get average y axis max chord length in counts
    mag_scale[2]  = (mag_max[2] - mag_min[2])/2;  // get average z axis max chord length in counts

    float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
    avg_rad /= 3.0;

    dest2[0] = avg_rad/((float)mag_scale[0]);
    dest2[1] = avg_rad/((float)mag_scale[1]);
    dest2[2] = avg_rad/((float)mag_scale[2]);
  
   Serial.println("Mag Calibration done!");
}
*/


// Accelerometer and gyroscope self test; check calibration wrt factory settings
void MPU9250SelfTest(float * destination) { // Should return percent deviation from factory trim values, +/- 14 or less deviation is a pass

  uint8_t rawData[6] = {0, 0, 0, 0, 0, 0};
  uint8_t selfTest[6];
  int32_t gAvg[3] = {0}, aAvg[3] = {0}, aSTAvg[3] = {0}, gSTAvg[3] = {0};
  float factoryTrim[6];
  uint8_t FS = 0;
   
  writeByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x00);    // Set gyro sample rate to 1 kHz
  writeByte(MPU9250_ADDRESS, CONFIG, 0x02);        // Set gyro sample rate to 1 kHz and DLPF to 92 Hz
  writeByte(MPU9250_ADDRESS, GYRO_CONFIG, FS<<3);  // Set full scale range for the gyro to 250 dps
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG2, 0x02); // Set accelerometer rate to 1 kHz and bandwidth to 92 Hz
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, FS<<3); // Set full scale range for the accelerometer to 2 g

  for( int ii = 0; ii < 200; ii++) {  // get average current values of gyro and acclerometer
  
    readBytes(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, &rawData[0]);        // Read the six raw data registers into data array
    aAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
    aAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;  
    aAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ; 
  
    readBytes(MPU9250_ADDRESS, GYRO_XOUT_H, 6, &rawData[0]);       // Read the six raw data registers sequentially into data array
    gAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
    gAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;  
    gAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ; 
  }
  
  for (int ii =0; ii < 3; ii++) {  // Get average of 200 values and store as average current readings
    aAvg[ii] /= 200;
    gAvg[ii] /= 200;
  }
  
  // Configure the accelerometer for self-test
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, 0xE0); // Enable self test on all three axes and set accelerometer range to +/- 2 g
  writeByte(MPU9250_ADDRESS, GYRO_CONFIG,  0xE0); // Enable self test on all three axes and set gyro range to +/- 250 degrees/s
  delay(25);  // Delay a while to let the device stabilize

  for( int ii = 0; ii < 200; ii++) {  // get average self-test values of gyro and acclerometer
  
    readBytes(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, &rawData[0]);  // Read the six raw data registers into data array
    aSTAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
    aSTAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;  
    aSTAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ; 
  
    readBytes(MPU9250_ADDRESS, GYRO_XOUT_H, 6, &rawData[0]);  // Read the six raw data registers sequentially into data array
    gSTAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]) ;  // Turn the MSB and LSB into a signed 16-bit value
    gSTAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]) ;  
    gSTAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]) ; 
  }
  
  for (int ii =0; ii < 3; ii++) {  // Get average of 200 values and store as average self-test readings
    aSTAvg[ii] /= 200;
    gSTAvg[ii] /= 200;
  }   
  
  // Configure the gyro and accelerometer for normal operation
  writeByte(MPU9250_ADDRESS, ACCEL_CONFIG, 0x00);  
  writeByte(MPU9250_ADDRESS, GYRO_CONFIG,  0x00);  
  delay(25);  // Delay a while to let the device stabilize
   
  // Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
  selfTest[0] = readByte(MPU9250_ADDRESS, SELF_TEST_X_ACCEL); // X-axis accel self-test results
  selfTest[1] = readByte(MPU9250_ADDRESS, SELF_TEST_Y_ACCEL); // Y-axis accel self-test results
  selfTest[2] = readByte(MPU9250_ADDRESS, SELF_TEST_Z_ACCEL); // Z-axis accel self-test results
  selfTest[3] = readByte(MPU9250_ADDRESS, SELF_TEST_X_GYRO);  // X-axis gyro self-test results
  selfTest[4] = readByte(MPU9250_ADDRESS, SELF_TEST_Y_GYRO);  // Y-axis gyro self-test results
  selfTest[5] = readByte(MPU9250_ADDRESS, SELF_TEST_Z_GYRO);  // Z-axis gyro self-test results

  // Retrieve factory self-test value from self-test code reads
  factoryTrim[0] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[0] - 1.0) )); // FT[Xa] factory trim calculation
  factoryTrim[1] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[1] - 1.0) )); // FT[Ya] factory trim calculation
  factoryTrim[2] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[2] - 1.0) )); // FT[Za] factory trim calculation
  factoryTrim[3] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[3] - 1.0) )); // FT[Xg] factory trim calculation
  factoryTrim[4] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[4] - 1.0) )); // FT[Yg] factory trim calculation
  factoryTrim[5] = (float)(2620/1<<FS)*(pow( 1.01 , ((float)selfTest[5] - 1.0) )); // FT[Zg] factory trim calculation
 
  // Report results as a ratio of (STR - FT)/FT; the change from Factory Trim of the Self-Test Response
  // To get percent, must multiply by 100
  for (int i = 0; i < 3; i++) {
    destination[i]   = 100.0*((float)(aSTAvg[i] - aAvg[i]))/factoryTrim[i] - 100.;   // Report percent differences
    destination[i+3] = 100.0*((float)(gSTAvg[i] - gAvg[i]))/factoryTrim[i+3] - 100.; // Report percent differences
  }
   
}



// I2C read/write functions for the MPU9250 and AK8963 sensors

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
  Wire.beginTransmission(address);  // Initialize the Tx buffer
  Wire.write(subAddress);           // Put slave register address in Tx buffer
  Wire.write(data);                 // Put data in Tx buffer
  Wire.endTransmission();           // Send the Tx buffer
}

uint8_t readByte(uint8_t address, uint8_t subAddress) {
  uint8_t data; // `data` will store the register data   
  Wire.beginTransmission(address);         // Initialize the Tx buffer
  Wire.write(subAddress);                  // Put slave register address in Tx buffer
  Wire.endTransmission(I2C_NOSTOP);        // Send the Tx buffer, but send a restart to keep connection alive
  //  Wire.endTransmission(false);             // Send the Tx buffer, but send a restart to keep connection alive
  //  Wire.requestFrom(address, 1);  // Read one byte from slave register address 
  Wire.requestFrom(address, (size_t) 1);   // Read one byte from slave register address 
  data = Wire.read();                      // Fill Rx buffer with result
  return data;                             // Return data read from slave register
}

void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest) {  
  Wire.beginTransmission(address);   // Initialize the Tx buffer
  Wire.write(subAddress);            // Put slave register address in Tx buffer
  Wire.endTransmission(I2C_NOSTOP);  // Send the Tx buffer, but send a restart to keep connection alive
  //  Wire.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
  uint8_t i = 0;
  //        Wire.requestFrom(address, count);  // Read bytes from slave register address 
  Wire.requestFrom(address, (size_t) count);  // Read bytes from slave register address 
  while (Wire.available()) {
    dest[i++] = Wire.read();         // Put read results in the Rx buffer
  }
}
