#ifndef _sensorType_H_
#define _sensorType_H_

// max amount of attributes for any sensor
#define MAX_SENSOR_ATTRIBUTES 10

enum class SensorType {
  local,
  digitalIn,
  oneWire,
  sht30,
  mpu925x,
  bmp280,
  ads1115,
  analogIn,
  digitalOut,
  SensorType_MAX = digitalOut  // update this if you add items!
};

#endif