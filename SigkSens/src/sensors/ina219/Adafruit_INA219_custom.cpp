
/*!
 * @file Adafruit_INA219_custom.cpp
 *
 * @mainpage Adafruit INA219 current/power monitor IC
 *
 * @section intro_sec Introduction
 *
 *  Driver for the INA219 current sensor
 *
 *  This is a library for the Adafruit INA219 breakout
 *  ----> https://www.adafruit.com/product/904
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing
 *  products from Adafruit!
 *
 * @section author Author
 *
 * Written by Bryan Siepert and Kevin "KTOWN" Townsend for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Arduino.h"
#include "Adafruit_INA219_custom.h"
#include <Wire.h>


/*!
 *  @brief  Instantiates a new INA219 class
 *  @param addr the I2C address the device can be found on. Default is 0x40
 */
Adafruit_INA219_custom::Adafruit_INA219_custom(uint8_t addr) {
  ina219_i2caddr = addr;
  ina219_currentMultiplier_A = 0.0f;
  ina219_powerMultiplier_W = 0.0f;
}

/*!
 *  @brief  Sets up the HW (defaults to 32V and 2A for calibration values)
 *  @param theWire the TwoWire object to use
 *  @return true: success false: Failed to start I2C
 */
bool Adafruit_INA219_custom::begin(TwoWire *theWire) {
  i2c_dev = new Adafruit_I2CDevice(ina219_i2caddr, theWire);

  if (!i2c_dev->begin()) {
    return false;
  }
  init();
  return true;
}

/*!
 *  @brief  begin I2C and set up the hardware
 */
void Adafruit_INA219_custom::init() {
  // Set chip to large range config values to start
}

/*!
 *  @brief  Checks if conversion is ready
 *  @return return true if conversion is ready
 */
bool Adafruit_INA219_custom::conversionReady() {
  uint16_t value;

  Adafruit_BusIO_Register bus_voltage_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_BUSVOLTAGE, 2, MSBFIRST);
  bus_voltage_reg.read(&value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (value & INA219_CNVR_MASK) != 0;
}

/*!
 *  @brief  Gets the raw bus voltage (16-bit signed integer, so +-32767)
 *  @return the raw bus voltage reading
 */
int16_t Adafruit_INA219_custom::getBusVoltage_raw() {
  uint16_t value;

  Adafruit_BusIO_Register bus_voltage_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_BUSVOLTAGE, 2, MSBFIRST);
  bus_voltage_reg.read(&value);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  return (int16_t)((value >> 3) * 4);
}

/*!
 *  @brief  Gets the raw shunt voltage (16-bit signed integer, so +-32767)
 *  @return the raw shunt voltage reading
 */
int16_t Adafruit_INA219_custom::getShuntVoltage_raw() {
  uint16_t value;
  Adafruit_BusIO_Register shunt_voltage_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_SHUNTVOLTAGE, 2, MSBFIRST);
  shunt_voltage_reg.read(&value);
  return value;
}

/*!
 *  @brief  Gets the raw current value (16-bit signed integer, so +-32767)
 *  @return the raw current reading
 */
int16_t Adafruit_INA219_custom::getCurrent_raw() {
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  Adafruit_BusIO_Register calibration_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_CALIBRATION, 2, MSBFIRST);
  calibration_reg.write(ina219_calValue, 2);

  // Now we can safely read the CURRENT register!
  Adafruit_BusIO_Register current_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_CURRENT, 2, MSBFIRST);
  current_reg.read(&value);
  return value;
}

/*!
 *  @brief  Gets the raw power value (16-bit signed integer, so +-32767)
 *  @return raw power reading
 */
int16_t Adafruit_INA219_custom::getPower_raw() {
  uint16_t value;

  // Sometimes a sharp load will reset the INA219, which will
  // reset the cal register, meaning CURRENT and POWER will
  // not be available ... avoid this by always setting a cal
  // value even if it's an unfortunate extra step
  Adafruit_BusIO_Register calibration_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_CALIBRATION, 2, MSBFIRST);
  calibration_reg.write(ina219_calValue, 2);

  // Now we can safely read the POWER register!
  Adafruit_BusIO_Register power_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_POWER, 2, MSBFIRST);
  power_reg.read(&value);
  return value;
}

/*!
 *  @brief  Triggers conversion
 */
void Adafruit_INA219_custom::triggerMeasurement() {
  Adafruit_BusIO_Register config_reg = Adafruit_BusIO_Register(i2c_dev, INA219_REG_CONFIG, 2, MSBFIRST);
  Adafruit_BusIO_RegisterBits mode_bits = Adafruit_BusIO_RegisterBits(&config_reg, 3, 0);
  mode_bits.write(INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED);
}

/*!
 *  @brief  Gets the shunt voltage in mV (so +-327mV)
 *  @return the shunt voltage converted to millivolts
 */
float Adafruit_INA219_custom::getShuntVoltage_mV() {
  int16_t value;
  value = getShuntVoltage_raw();
  return value * 0.01;
}

/*!
 *  @brief  Gets the shunt voltage in volts
 *  @return the bus voltage converted to volts
 */
float Adafruit_INA219_custom::getBusVoltage_V() {
  int16_t value = getBusVoltage_raw();
  return value * 0.001;
}

/*!
 *  @brief  Gets the current value in mA, taking into account the
 *          config settings and current LSB
 *  @return the current reading convereted to milliamps
 */
float Adafruit_INA219_custom::getCurrent_A() {
  float valueDec = getCurrent_raw();
  valueDec *= ina219_currentMultiplier_A;
  return valueDec;
}

/*!
 *  @brief  Gets the power value in mW, taking into account the
 *          config settings and current LSB
 *  @return power reading converted to milliwatts
 */
float Adafruit_INA219_custom::getPower_W() {
  float valueDec = getPower_raw();
  valueDec *= ina219_powerMultiplier_W;
  return valueDec;
}

/*!
 *  @brief  Set power save mode according to parameters
 *  @param  on
 *          boolean value
 */
void Adafruit_INA219_custom::powerSave(bool on) {
  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_CONFIG, 2, MSBFIRST);

  Adafruit_BusIO_RegisterBits mode_bits =
      Adafruit_BusIO_RegisterBits(&config_reg, 3, 0);
  if (on) {
    mode_bits.write(INA219_CONFIG_MODE_POWERDOWN);
  } else {
    mode_bits.write(INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED);
  }
}

/*!
 *  @brief set device to alibration which uses the highest precision for
 *     current measurement (0.1mA), at the expense of
 *     only supporting 16V at 400mA max.
 */
bool Adafruit_INA219_custom::setCalibration(float shunt_A, float shunt_mV, float max_V, float max_A) {
  // scale shunt values to working range
  float scaled_mV = max_A/shunt_A * shunt_mV;

  // set optimal PGA gain
  uint16_t config_gain;
  if (scaled_mV <= 40)
    config_gain = INA219_CONFIG_GAIN_1_40MV; // Gain 1, 40mV Range
  else if (scaled_mV <= 80)
    config_gain = INA219_CONFIG_GAIN_2_80MV;  // Gain 2, 80mV Range
  else if (scaled_mV <= 160)
    config_gain = INA219_CONFIG_GAIN_4_160MV; // Gain 4, 160mV Range
  else if (scaled_mV <= 320)
    config_gain = INA219_CONFIG_GAIN_8_320MV; // Gain 8, 320mV Range
  else
    return false;

  // set ADC range
  uint16_t config_range;
  if (max_V <= 16)
    config_range = INA219_CONFIG_BVOLTAGERANGE_16V; // 0-16V Range
  else if (max_V <= 32)
    config_range = INA219_CONFIG_BVOLTAGERANGE_32V; // 0-32V Range
  else
    return false; // out of range

  // calculate minimum LSB for adc
  float shunt_r = shunt_mV/shunt_A/1000.;
  float LSB_min = max_A/32768;
  
  // round LSB to nearest factor 5
  float LSB_factor = pow(10.0, 1-ceil(log10(LSB_min/5)))/5;
  float LSB_round = ceil(LSB_min * LSB_factor) / LSB_factor;
  
  // Set shunt calibration value
  ina219_calValue = truncf(0.04096/(LSB_round*shunt_r));
  
  // Set multipliers to convert raw current/power values
  ina219_currentMultiplier_A = LSB_round;
  ina219_powerMultiplier_W = 20. * LSB_round;

  // Set Calibration register to 'Cal' calculated above
  Adafruit_BusIO_Register calibration_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_CALIBRATION, 2, MSBFIRST);
  calibration_reg.write(ina219_calValue, 2);
  // Set Config register to take into account the settings above
  uint16_t config = config_range |
                    config_gain | INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_BADCRES_12BIT_128S_69MS;

  Adafruit_BusIO_Register config_reg =
      Adafruit_BusIO_Register(i2c_dev, INA219_REG_CONFIG, 2, MSBFIRST);
  config_reg.write(config, 2);

  Serial.print("INA219: ");
  Serial.print(shunt_A,0);
  Serial.print("A, ");
  Serial.print(shunt_mV,0);
  Serial.print("mV, ");
  Serial.print(shunt_r*1000,3);
  Serial.print("mOhm, ");
  Serial.print(LSB_round*1000,2);
  Serial.print("mA/bit, ");
  Serial.print(ina219_calValue);
  Serial.println("cal value");
  return true;
}