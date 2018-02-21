# How to use this software:


On first bootup, Wifi credentials will need to be configured. This is done via the "WifiManager" library. The ESP will go into Access Point mode and create an unsecured AP named "Unconfigured Sensor". Connect to this AP with a phone/laptop etc, and you should be redirected to a webpage to configure the Wifi Settings. (If you are not automatically redirected, you can manually browse to http://192.168.4.1 )

You can either enter the Wifi credentials manually, or select "Scan" to have a list of detected access points shown. You also need to set a hostname for the sensor. (This is what it will show up as in mdns/network discovery)

Once Connected to a Wifi Network, you should be able to see the device on your network (Network center in windows). 

The firmware will scan i2c for any recognized sensors, as well as the oneWire bus for any tempurature probes. It also performs a mDNS discovery for the SignalK server, and connects to it if found. 

# SensorInfo

Each sensor discovered will have an Address. This is usually the oneWire sensor address, or i2c address.
Each sensor will have one or more "attributes". An attribute is a data value that the sensor can send. For example a SHT30 sensor has two attributes: "tempK" and "humidity". Each attribute has 3 parameters, "path", "scale", and "offset". Path is the SignalK path that the value will be sent to. Scale and offset can be used to callibrate the data. (raw value is multiplied by scale, then offset is added). Data will only be sent to the SignalKServer if the path is set. Scale defaults to "1", offset to "0".

# API

There's a very simple API used to get information or set parameters. All requests are GET for simplicity.

### /getSensorInfo
Returns a list of all global config and sensors and their configuration. (Currently WIP, needs updating to handle large responses)

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
http://A.B.C.D/getSensorInfo?address=0x42&attr=tempK&path=environment.outside.temp

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

http://A.B.C.D/setSignalKPort?path=/signalk/v1/stream


# Supported Sensors and their attributes

- oneWire
  - tempK
    - Tempurature in kelvin
- mpu925x
  - tempK
    - Tempurature in kelvin of the MPU chip.
  - yaw
    - yaw referenced to magnetic north in radians.
  - pitch
    - pitch referenced to "level" in radians.
  - roll
    - roll referenced to "level" in radians.
  - filterRate
    - number of time the sensor integration filter ran in one second. (useful for troublehooting)
- sht30
  - tempK
    - Tempurature in kelvin
  - humidity
    - Humidity in percent (0-100)
- digitalIn
  - state
    - Boolean state of the input. True = High, False = low.
  - freq
    - Number of pulses/second
  - count
    - Number of pulses (will wrap around after 32bit).
- ads1115
  - diff0_1
    - Differential voltage between inputs 0 and 1, in Millivolts. (1000mV = 1V)
  - diff2_3
    - Differential voltage between inputs 2 and 3, in Millivolts. (1000mV = 1V)
  - chan0
    - Voltage (referenced to GND) on input 0, in Millivolts.
  - chan1
    - Voltage (referenced to GND) on input 1, in Millivolts.    
  - chan2
    - Voltage (referenced to GND) on input 2, in Millivolts.    
  - chan3
    - Voltage (referenced to GND) on input 3, in Millivolts.    
- bmp280
  - tempK
    - Tempurature in kelvin
  - Pa
    - Air presure in Pascals.
- local (useful in debuging, information obout the ESP itself )
  - systemHz
    - number of times the main loop ran in 1 second.
  - freeMem
    - number of free bytes of ram.



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
  
 
