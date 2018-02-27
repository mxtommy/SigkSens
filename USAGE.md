# How to use this software:


On first bootup, Wifi credentials will need to be configured. This is done via the "WifiManager" library. The ESP will go into Access Point mode and create an unsecured AP named "Unconfigured Sensor". Connect to this AP with a phone/laptop etc, and you should be redirected to a webpage to configure the Wifi Settings. (If you are not automatically redirected, you can manually browse to http://192.168.4.1 )

You can either enter the Wifi credentials manually, or select "Scan" to have a list of detected access points shown. You also need to set a hostname for the sensor. (This is what it will show up as in mdns/network discovery)

Once Connected to a Wifi Network, you should be able to see the device on your network (Network center in windows). 

The firmware will scan i2c for any recognized sensors, as well as the oneWire bus for any tempurature probes. It also performs a mDNS discovery for the SignalK server, and connects to it if found. 

# SensorInfo

Each sensor discovered will have a type and Address. The type is a numeric value that identifies what type of sensor it is. (see the table after). The address is used to differenciate different sensors of the same type. This is usually the oneWire sensor address, or i2c address, etc

Each sensor will have one or more "attributes". An attribute is a data value that the sensor can send. For example a SHT30 sensor has two attributes: "tempK" and "humidity". Each attribute has 3 parameters, "path", "scale", and "offset". Path is the SignalK path that the value will be sent to. Scale and offset can be used to callibrate the data. (raw value is multiplied by scale, then offset is added). Data will only be sent to the SignalKServer if the path is set. Scale defaults to "1", offset to "0".

By default there is no path set to any attribute. That means that no deltas will be sent to the server. Setting the path for an attribute will "activate" it, and cause deltas to be sent to the server for that attribute. For testing purposes, there is a "Local" sensor that does not need any additional circuitry to use.

The following is a list of supported Sensor typess and their attributes. You can see in the hardware folder for some example circuit diagrams.

## Sensor Types
| Id  | SensorType | Description |
| --- | --- | --- |
| 0 | local | Local ESP info |
| 1 | digitalIn | Digital Inputs (state/hz) |
| 2 | oneWire | OneWire Tempurature probes |
| 3 | sht30 | Tempurature and Humidity |
| 4 | mpu925x | 9-Axis IMU |
| 5 | bmp280 | Barometric presure sensor |
| 6 | ads1115 | 16 bit 4 channel ADC |
| 7 | analogIn | ESP's 10 bit built-in ADC |
| 8 | digitalOut | Digital Output pins |

## Sensor Details

### Local

This sensor is useful for debugging, It gives some information on the state/performance of the ESP.

| AttributeName |  Description |
| --- | --- |
| systemHz | Number of times the main loop executed in 1 second |
| freeMem | Number of free Bytes on ESP |

### Digital In

This "sensor" is useful to see the status of digital pins. You can either see State (On/Off) or the frequency of state change. (for example for counting RPM). 
Note State sends 1 update per deltaTimer period, and also a delta immediately on state change. Setting State attribute on something like an rpm input would cause a delta to be sent on every pulse, probably not the best idea :) Frequency sends only 1 delta every deltaTimer period.

Input Pins can be defined in config.h  Defaults to "D5" and "D6" pins on Wemos D1 Mini (pins 14 and 12 of ESP)

| AttributeName |  Description |
| --- | --- |
| state | Reports whether pin is high/low.  |
| freq | Reports the number of pulses counted over a second |
| count | raw pulse counter. |

### One Wire Temp

Multiple One Wire sensors are supported, including MAX31850 thermocouple boards.

| AttributeName |  Description |
| --- | --- |
| tempK | Tempurature reported from the probe (in Kelvin) |


### sht30 Temp/Humid

Keep in mind that the ESP generates a bit of heat, enough to add a few degrees of heat to the reading of the sensor if it is too close to the board. It is best to put a bit of distance between the SHT30 and the ESP.

| AttributeName |  Description |
| --- | --- |
| tempK | Tempurature in kelvin |
| humidity | Humidity in percent (0-100) |

### Mpu925x 9-Axis IMU
This sketch will run a sensor fusing algorithm to try and get usable output out of the MPU925x chips. The onboard DSP is only a 6-axis fusion algorithm, so in order to fuse the magnometer data as well we use our own fusion. Note: Sensor MUST be calibrated in order to be useful!

| AttributeName |  Description |
| --- | --- |
| tempK | Tempurature in kelvin of the MPU chip |
| yaw | yaw referenced to magnetic north in radians |
| pitch | pitch referenced to "level" in radians |
| roll | roll referenced to "level" in radians |
| filterRate | number of time the sensor integration filter ran in one second. (useful for troublehooting) |

### bmp280 Barometric sensor

| AttributeName |  Description |
| --- | --- |
| tempK | Tempurature in kelvin |
| Pa | Air presure in Pascals |


### ads1115 4x16 bit ADC

In order to smooth data, many read a second are taken, and are smoothed with a simple exponent filter. (readings are only taken if path is set) Updates are sent on deltaTimer interval. Use scale/offset to convert millivolts to final value if needed.

| AttributeName |  Description |
| --- | --- |
| diff0_1 | Differential voltage between inputs 0 and 1, in Millivolts. (1000mV = 1V) |
| diff2_3 | Differential voltage between inputs 2 and 3, in Millivolts. (1000mV = 1V) |
| chan0 | Voltage (referenced to GND) on input 0, in Millivolts |
| chan1 | Voltage (referenced to GND) on input 1, in Millivolts |
| chan2 | Voltage (referenced to GND) on input 2, in Millivolts |
| chan3 | Voltage (referenced to GND) on input 3, in Millivolts |

### Analog 10Bit ADC

Similat to ADS1115 with an exponential filter to smooth data a bit. Reads from the build-in 10bit ADC in the ESP8266 (a0). Use scale/offset to change to get proper range.

| AttributeName |  Description |
| --- | --- |
| a0 | Value of the A0 10bit ADC (between 0 and 1024) |

### Digital Output 

Use to control the world! Connect to relay etc to power more than a LED.
Note this is a WIP. It also requires PUT support in the signalK server to work, which has not been released into NPM at this time. The ESP will report the current state of the output to the path set. It will also listen for PUT requests to that signalk path. 

Output Pins can be defined in config.h  Defaults to "D0" and "D8" pins on Wemos D1 Mini (pins 16 and 15 of ESP)

| AttributeName |  Description |
| --- | --- |
| state | Current state (on=true, off=false) of the output. Defaults to off on startup |
 

# API

There's a very simple API used to get information or set parameters. All requests are GET for simplicity.

### /getSensorInfo
Returns a list of all global config and sensors and their configuration.

Ex: 
http://A.B.C.D/getSensorInfo

### /setSensorAttr
Sets the parameters of a sensor. 

Required HTTP parameters: 
- address
  - Address of the sensor. (ex: "0x42" for an I2C)
- attrName
  - name of attribute to set parameters for (ex: "tempK")
  
Optional HTTP parameters: (though not setting any of these seems a bit pointless :D)
- path
  - Sets the signalk Path of the attribute (ex: "environment.outside.temp")
- scale
  - Sets the scale of the attribute. Raw value is multiplied by this (Ex: "1.03")
- offset
  - Sets the offset for the attribute. The amount added to the scaled value. For substraction use a negative number (Ex: "-3.345")

Example:
http://A.B.C.D/setSensorAttr?address=0x42&attrName=tempK&path=environment.outside.temp

### /setTimerDelay
Changes the amount of time between SignalK deltas (in ms).

Required HTTP parameters:
- timer
  - Name of the timer to change (see below)
- delay
  - New value in ms (1 second = 1000ms)

Timer can be one of the following:
- deltaDelay
- ads1115Read


Ex:
http://A.B.C.D/setTimerDelay?timer=deltaDelay&delay=500

### /setNewHostname
This command sets a new hostname for the sensor. Note that is causes the sensor to reboot!

Required HTTP parameters: 
- hostname
  - New hostname. max 16 charcters.
  
Ex:
http://A.B.C.D/setNewHostname?hostname=mySensor



### /setSignalKHost
Used to manually set the hostname of the signalk server. Default: ""

Not needed if server is detected via mDNS.

Required HTTP parameters:
- host
  - IP address of SignalK server

http://A.B.C.D/setSignalKHost?host=1.2.3.4

### /setSignalKPort
Used to manually set the port of the signalk server. Default: "80"

Not needed if server is detected via mDNS.

Required HTTP parameters:
- port
  - port number of SignalK Server

http://A.B.C.D/setSignalKPort?port=3000

### /setSignalKPath

Used to manually set the URL of the signalk server. Default: "/signalk/v1/stream" 

(Not needed if server is detected via mDNS. Should also never really change?)

Required HTTP parameters:
- path
  - URL (path) of SignalK Server

http://A.B.C.D/setSignalKPath?path=/signalk/v1/stream





# MPU925X usage
Note, make sure the interupt pin is connected as well :)

If there is a MPU925X 9-axis IMU connected, it will need to be calibrated before it can be used. There are two steps for calibration.

- Accelerometer and Gyroscope calibration
  - Keep the sensor completely still and level. (this basically tells the sensor what "level" is.
  - Perform the callibration with a request to "/mpuCalAccelGyro" (ex: http://A.B.C.D/mpuCalAccelGyro). No parameters required. After a couple of seconds calibration will be done.
- Magnometer calibration
  - Start Magnetic calibration mode with a request to "/mpuCalMagStart" (ex: http://A.B.C.D/mpuCalMagStart). This puts the program into magnetic calibration mode. Durring this time no MPU data will be setn to the signalK server.
  - Rotate the sensor in every direction through all three axis. Make sure the location you do this in is free of magnetic interference. (Lost a few days of troubleshooting only to realize being to close to a computer can affect it!)
  - Finish the calibration by making a request to "/mpuCalMagStop" (ex: http:/A.B.C.D/mpuCalMagStop). This saves the generated calibration and puts the MPU back into run mode.
  
 
