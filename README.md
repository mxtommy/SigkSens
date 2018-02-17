# SignalK Wifi Controllers
## Design Goals
---------------------
Simple low cost Wifi based IoT device. Should be simple enough to setup/use that it could be manufactured and sold to people with no microcontroller/circuit design knowledge. Support configuration via a Web application, and network discovery for ease of use.

Assuming it will be installed close to the object being monitored/controlled, it should not need to support many IO by default. Aim to cover 80% of use cases with single device, assuming other 20% can be solved by adding a additional stand alone devices. If 80% of the time you only need 2 Inputs, the 20% of the time you need 4 inputs just put another device. IE prefer multiple cheap devices vs one expensive do everything one.

Hardware should be easy to assemble on breadboard, as well as be able to be made into a PCB.

Device should send data to SignalK Server over Wifi.
Device should accept commands from Wifi. (WIP: how?)

## Use Case and Inputs required 
---------------------

- Engine Monitor
    - 1Wire Bus (multiple Temps)
    - 1 Digital Input (rpm pulse)
    - SHT30 (Engine room temp/humidity)
    - 1 Analog Inputs (Oil Presure)
- Bilge pump Monitor:
    - 2 Digital Input (Bilge pump on/off, High water Alarm)
- Fridge Monitor:
    - 1Wire Bus (Temps)
    - 1 Digital Input (Compressor on/off)
    - SHT30, Temp/Humidity
- Tank Monitor:
    - 1 Analog Input (Tank level)
- DC Monitor:
    - 1Wire Bus (Battery Temps)
    - 2 Analog Inputs - Current/Voltage 
- AC Monitor:
    - 2 Analog inputs - Current/Voltage
- GPS
    - UART
- Environment
    - Presure Sensor
    - IMU
    - SHT30 (temp/humidity)
        
## Final list of Inputs (and current status):
- 1Wire Bus (supporting MAX31850 Thermocouple breakout)
	- Done!
- MPU925X 9 axis IMU
	- Done!
- 2 Digital in (Opto isolated)
    - Done!
- 2 Digital out (Opto isolated)
	- Planned.
- SHT30 temp/humidity sensor
	- Done
- ADS1115 ADC (2 input 16 bits)
	- Done!
- Pressure sensor
	- Done!

		
## Requirement List

### Hardware
---------------------
- Circuits should follow best practices, over-voltage protection, isolated where possible, etc
	- Need to make friendly circuit diagrams (currentl in kicad, need to make images)
- Core Wifi Hardware module
    - 12V External Power
    - IO 
        - OneWire - Covers most Temp situations and can support many temp sensors in same bus. Only requires single resistor to support...
        - Config Reset button.
- Rest handled by adding to core. 
    - 2 opto Isolated Digital I/O
    - ADS1115 (16bit ADC)
    - INA219 (Current sensing)
    - IMU (Due to timing/computations needed, recommended to not run other sensors at same time as IMU)
    - SHT30 (Temp/Humidity)
    - Presure sensor

- Case/Physical connections.
    - 3d printable cases. 
    - Terminal blocks for connecting external wires.
    - Mounting of Core module and any expansion modules

     
### Software
----------------------

#### Features (and current status if not done)
- One binary to rule them all. (Should not need to recompile to support different sensors/functions) Saves supporting multiple versions of software.
- Wifi AP configuration. (Credentials not hard coded in source)
- Web API to support configuration via the webApp.
- No hardcoded configurations where possible. All config configurable via Webapp
- Configurations saved to filesystem.
- Supports discovery services (mDNS/ssdp)
- Non blocking interupts/timer mechanisms should be used instead of delay() whenever possible.
- Web based update of firmware.
	- Planned, not done yet.
- Configurable user/pass for HTTP basic auth. 
	- Planned, not done yet.
    
      

#### Signalk
- Hosts a simple websocket only (no rest interface) signalk server. Responds to endpoint URL with URL of websocket, and hosts websocket server. Can handle one or two clients.
- Auto-discovers SignalK Server via mDNS. Can also be manually set. Connects to existing signalk server to push deltas there.
- Sends delta messages at regular (configurable) interval, combining multiple updates into one message when needed.



