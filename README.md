![Project's banner](images/mbElectricalSystem_banner.png)
> Modular electrical control unit for pre-injection motorbikes  
> based on ESP32 and distributed power management

## 1.0 Files

|    Files/Dirs    | Description                          |
|------------------|--------------------------------------|
| firmware-esp32   | Firmware for the device's brain      |
| images           | Images used for README.md file       |
| mechanical_parts | The CAD files for the device's box   |
| PCB              | All the PCB used by the device       |

## 2.0 Description:
This project (MBES) aims to realize a generic motorbike electric system you can use on any old motorbike, or customized ones.

Pre-injection motorbikes typically leave the factory with a bulky and highly distributed wiring harness. All power and control
lines are routed through a single large cable bundle, creating what can be described as an “electrical snake”.

Opening this harness often feels like exposing the internal organs of that snake: a dense mass of wires embedded in adhesive
tape and protective compounds. Identifying a single faulty line inside this structure can quickly become a nightmare.

No more chaotic situations with this modern motorbike electrical system: the (bigger) power bales will run just from the device
to the user (light, horn, indicators....) and all commands' cables (light switch, start button, turn indicators switches...)
will be smaller because very small electric current will flow inside them.

### 2.1 Managed devices:
- High beam and down beam headlights
- Left and right turn flashing indicators
- Additional (high power) light
- Horn
- Neutral, left/right indicators, high/down light... LEDs
- STOP light
- Engine start button
- Motorbike (and engine) OFF switch
- Clutch switch
- Decompressor command management
- External cockpit power-connector
- Cockpit's leds

### 2.2 Provided features
- Checking for decompressor command
  The system does not allow the engine to run, before the decompressor command has been activated.
  If the veichle does not have a decompressor system, or if the level control switch is broken, then this feature can be
  disabled easily.

- Direction indicator blinking
  Common blinking for the yellow indicator. The system has been planed for 12V LED indicators (with resistors), but because
  for these ports 2A current can be supported, you can also use older lights

- Resistive authentication key
  In order to turn-on the motorbike, the driver must insert a valid resistor key. After the autentication the key
  can be also removed. It is very usefull for off-road using. 

- Parking mode
  To set the motorbike turned-off but with the normal-light-on and the indicators blinking, you have to put the
  motorbile stopped on its lateral stand, set the engine swith to off, and turn the normal-light on. In this mode the engine
  cannot be started. To restart the veichle you have to turn-off it with the proper switch
	
### 2.3 Security rules:
- Byke stand
  if the motorcycle stand is downn when the engine is connected to the wheels, the the engine will be stopped

- Decompressor command
  If the motorcycle is running and the driver will push the decompressor command, then the engine will be stopped

## 3.0 Info about this version:
This branch is based on ESP32 MCU. This chip is a powerful and very low-price device and it allowed me to reduce the PCB
size. At the moment, I am using the ESP32 devkit device because welder this chip is a bit difficult operation, for my skills.
In this branch the Electrical System front part, is composed by two PCBs: one is the logic controller (all input are connected to
this PCB), the other is the electric power stage (all the high current outputs lines are provided by this one).
Depending by the device's box shape, the dual-pcb solution can requires a bit bigger space.
