![Project's banner](../../images/mbElectricalSystem_banner.png)

## 1.0 Files

|        Files/Directories       | Description                          |
|--------------------------------|--------------------------------------|
| pcb-a.kicad_sch                | Main electrical scheme               |
| mbes_pcbA_connectors.kicad_sch | Connectors electrical scheme         |
| pcb-a.kicad_pcb                | PCB                                  |

(*) This project is covered by the GPL-3. Please, read that file for further information

## 2.0 Description:
This folder contains all files you need to modify/build the PCB that implements Motorbike Elecrical System's brain. It controls
the hand bar commands (buttons, switches...) and turn on/of the services considering its policies.

### 2.1 The Micro Controller Unit
In order to implement the motorbike'sservices management I have chosen the 
[ESP32-S2](https://documentation.espressif.com/esp32-s2_datasheet_en.pdf) MCU produced by Espressif. I select this chip for the
following reasons:

- It has **43 programmable GPIOs**. It allows me to manage all I/O lines without an external bus expander (es. MCP23016). 
- Its core is an Xtensa® **single-core 32-bit LX7 microprocessor (240 MHz)**. Its faster then what is needed
- ROM=128KB and **SRAM=320KB**. Adequate also for future implementations (eg. display management)
- I2C and SPI ports. It will be necessary for future **CAN BUS** implementation
- **ESP-IDF** framework. It is a fantastic big framework that allows you to develop also complicated firmware in easy way
- Embedded **WiFi** support. It will be needed for future implementations (cockpit on mobile)
- **AD Converter**. Required by the resistive key authentication
- It is a powerful but cheap device: less then 4 euro


### 2.2 MCU vs DevKit
The ESP32-S2, as all modern MCU is available in SMD but not in THT package. But, unfortunately, its SMD package (like every modern
MCU) is complicated to weld, too difficult for me. So, for the moment, I have used the official development kit 
[ESP32-S2-DevKitM-1](https://docs.espressif.com/projects/esp-idf/en/v5.2.1/esp32s2/hw-reference/esp32s2/user-guide-devkitm-1-v1.html)
produced by Espressiff. It is widely available on market and costs less then 10 euro


### 2.3 Cables and internal connectors
Every cable starting from this PCB, a PIN-Header 2.54mm socket is provided. This type of connector can support up to 2.5A electric
current. It means it can handle 1.5A without problem, that it is absolutely higher then what is needed here. 
On the PCB you can find the following sockets:

1) Two [10 PINs dual-lines connectors](https://www.molex.com/en-us/products/part-detail/702461004?display=pdf) used for the
  communication between the two PCBs in the motorbike's front-side.
2) Two single-lines connectors used to connect the left and right handle-bar controls (lights, direction indicators, horn,
  engine-on/off...)
3) One 8 PINs single-line connector to control the rear-side power stage (pcb-c)
4) Many 2 PINs single-line to connect some switch (PowerOff, addictionalLight, decompressor...) 
5) One 4 PINs single-line one for the resistive key socket (4 of 6 pins)

Unfortunately I have not found documentation of PIN-Header 2.54mm single line connectors, so, I took the following photo:

![PIN-Header 2.54mm single-line](../images/PinHeader254.jpeg)

The cables connected to the **#1** type socket are for box-internal communication only, and the electric current that flows inside
of them, is very log (<100mA). Thanks to this aspect, you don't need to waste time to weld the connector's pins, you can use 
[10-pins flat cable](https://www.we-online.com/components/products/datasheet/63911015521CAB.pdf) and
the [YTH214 Crimping Tool IDC](https://docs.rs-online.com/8d29/A700000009783991.pdf) to crimp the connectors.

All other connectors (except the one for resistive key-locker) are for external links, so, the cable must be enough resistent. For
this reason you can use AWG24 cable: it has a 0.22mmq size and supports up to 2A electric current, much more then needed.


### 2.4 External connectors
In ordet to connect the handlebars electric (buttons/switches) conrols, I selected 2.8mm tab-terminal-system connectors.
These connectors are widely used in automotive applications, for this reason you can find them as "2.88 Automotive Connectors", often.
You can easily buy them in kit version, on Amazone too.
Every (2.88mm) pin of this type can handle 10A without problem.


	Left connector pins-map:
	========================            +---+
	1 GND                         +-----+---+-----+
	2 Left turn indicator         | +-+  +-+  +-+ |
	3 Low-beam light              | |4|  |5|  |6| |
	4 High-beam light             | +-+  +-+  +-+ |
	5 Right turn indicator        | +-+  +-+  +-+ |
	6 Horn                        | |1|  |2|  |3| |
	                              | +-+  +-+  +-+ |
	                              +---------------+

	Right connector pins-map:
	=========================
	1) GND
	2) GND
	3) Brake control switch
	4) Engine START button
	5) Engine OFF
	6) Engine ON
