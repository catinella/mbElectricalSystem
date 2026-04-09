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

### 2.3 Cables


### 2.4 Connectors
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
