![Project's banner](../images/mbElectricalSystem_banner.png)
> Modular electrical control unit for pre-injection motorbikes  
> based on ESP32 and distributed power management

## 1.0 Files

|     Files/Dirs      | Description                                                       |
|---------------------|-------------------------------------------------------------------|
| [build]             | Temporary folder used by firmware building process                |
| CMakeLists.txt      | The CMAKE configuration file used to create the required Makefile |
| [CMakeLists.conf]   | Building process' configuration file                              |
| configUtils.cmake   | CMAKE functions library                                           |
| main                | The folder that contains the main firmware source code            |
| sdkconfig           | Project and device configuration (used by ESP-IDF framework)      |
| components          | Libraries used by the firmware                                    |
| tools               | Tools for debugging and other purposes                            |


## 2.0 Description
This folder contains all needed files to build the project firmware

### 2.1 Environment Configuration
This firmware is based on Espressif's MCU ESP32-S2 and, in order to built it, you have to get and configure the official vendor's
building environment. To complete the task, please, read the
[ESP-IDF ver 5.3  documentation](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/get-started/index.html)
	
### 2.2 Device Configuration
Because the JTAG's pins has used as normal GPIOs you need to disable the JTAG support on your ESP32-S2 MCU with the following
command with the proper serial port character device (eg. /dev/ttyUSB0):

	espefuse.py --port <serial-port> burn_efuse SOFT_DIS_JTAG

### 2.3 Firmware Configuration
From the 5.3 version, esp-idf uses CMAKE as software building infrastructure (project.mk is no more available).
For this rerason all firmware's customization has been set in the CmakeLists.conf file. This file is parsed by the CmakeLists.txt
file using the configUtils.cmake library. In order to get the options list, please, read the documentation lines in the
[CMakeLists.txt](./CMakeLists.txt) file's head.

### 2.4 Building process
In order to build the firmware use the following idf standard command:
	idf.py build

### 2.5 Debug console
In order to monitor the pin's status in interactively, you can use the **debugConsole** software in the <PROJECT HOME>/tool folder
