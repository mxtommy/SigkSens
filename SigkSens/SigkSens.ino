#include <ReactESP.h>

#include "config.h"

#ifdef ENABLE_I2C
#include "src/services/i2c.h"
#endif
#ifdef ENABLE_MPU
  #include "src/sensors/mpu9250/mpu9250.h"
#endif
#ifdef ENABLE_SHT30
  #include "src/sensors/sht30/sht30.h"
#endif
#ifdef ENABLE_BMP280
  #include "src/sensors/bmp280/bmp280.h"
#endif
#ifdef ENABLE_BME280
  #include "src/sensors/bme280/bme280.h"
#endif
#ifdef ENABLE_ADS1115
#include "src/sensors/ads1115/ads1115.h"
#endif
#ifdef ENABLE_ONEWIRE
  #include "src/sensors/oneWire/oneWire.h"
#endif
#ifdef ENABLE_DIGITALIN
  #include "src/sensors/digitalIn/digitalIn.h"
#endif
#ifdef ENABLE_DIGITALOUT
  #include "src/sensors/digitalOut/digitalOut.h"
#endif
#ifdef ENABLE_ANALOGIN
  #include "src/sensors/analogIn/analogIn.h"
#endif
#ifdef ENABLE_SYSTEMHZ
  #include "src/sensors/systemHz/systemHz.h"
#endif
#ifdef ENABLE_INA219
  #include "src/sensors/ina219/ina219.h"
#endif

#include "FSConfig.h"

#include "src/net/discovery.h"
#include "src/net/webSocket.h"
#include "src/net/httpd.h"
#include "src/net/sigKWifi.h"
#include "src/net/ota.h"

#include "src/services/configReset.h"
#include "src/services/signalK.h"


#include "sigksens.h"
#include "src/sensors/sensorStorage.h"

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Global Variables
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

char myHostname[16];

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
General Setup
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

void setupFromJson() {
  #ifdef ENABLE_SYSTEMHZ
  fromJson[(int)SensorType::local] =
    (fromJsonFunc)&(SystemHzSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_DIGITALIN
  fromJson[(int)SensorType::digitalIn] =
    (fromJsonFunc)&(DigitalInSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_DIGITALOUT
  fromJson[(int)SensorType::digitalOut] =
    (fromJsonFunc)&(DigitalOutSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_ANALOGIN
  fromJson[(int)SensorType::analogIn] =
    (fromJsonFunc)&(AinSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_ONEWIRE
  fromJson[(int)SensorType::oneWire] =
    (fromJsonFunc)&(OneWireSensorInfo::fromJson);
  #endif

  #ifdef ENABLE_SHT30
  fromJson[(int)SensorType::sht30] =
    (fromJsonFunc)&(SHT30SensorInfo::fromJson);
  #endif

  #ifdef ENABLE_MPU
  fromJson[(int)SensorType::mpu925x] =
    (fromJsonFunc)&(MPU9250SensorInfo::fromJson);
  #endif    

  #ifdef ENABLE_ADS1115
  fromJson[(int)SensorType::ads1115] =
    (fromJsonFunc)&(ADSSensorInfo::fromJson);    
  #endif

  #ifdef ENABLE_BMP280
  fromJson[(int)SensorType::bmp280] =
    (fromJsonFunc)&(BMP280SensorInfo::fromJson);
  #endif

  #ifdef ENABLE_BME280
  fromJson[(int)SensorType::bme280] =
    (fromJsonFunc)&(BME280SensorInfo::fromJson);
  #endif

  #ifdef ENABLE_INA219
  fromJson[(int)SensorType::ina219] =
    (fromJsonFunc)&(INA219SensorInfo::fromJson);
  #endif
}

// forward declarations
void loop_();
void slow_loop();

ReactESP app([] () {
    bool need_save = false;
  // put your setup code here, to run once:
  Serial.begin(115200);

  ledBlinker.setWifiDisconnected();

  setupFromJson();

  setupFS();

  setupWifi();
  loadConfig();
  setupDiscovery();
  setupWebSocket();
  setupSignalK();
  setupOTA();

  setupConfigReset();
  #ifdef ENABLE_ONEWIRE
  setup1Wire(need_save);
  #endif
  #ifdef ENABLE_I2C
  setupI2C(need_save);
  #endif
  #ifdef ENABLE_DIGITALIN
  setupDigitalIn(need_save);
  #endif
  #ifdef ENABLE_DIGITALOUT
  setupDigitalOut(need_save);
  #endif
  #ifdef ENABLE_ANALOGIN
  setupAnalogIn(need_save);
  #endif
  #ifdef ENABLE_SYSTEMHZ
  setupSystemHz(need_save);
  #endif
  
  if (need_save) {
    saveConfig();
  }
  
  // call http last so that we can call any needed callbacks.
  setupHTTP();


  Serial.printf("Ready.\n");

  app.onRepeat(SLOW_LOOP_DELAY, slow_loop);

  
});

/*---------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------
Main Loop!
-----------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------*/

void slow_loop() {
  // Check Wifi...
  if (WiFi.status() != WL_CONNECTED) {
    // if connection is lost, simply restart
    ESP.restart();
  }
  
  handleConfigReset(); 
}

