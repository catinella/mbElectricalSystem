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
This firmware is based on Espressif's MCU ESP32-S2 and, in order to build it, you have to get and configure the official vendor's
building environment. To complete the task, please, read the
[ESP-IDF ver 5.3  documentation](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/get-started/index.html)
	
### 2.2 Device Configuration
Because the JTAG pins are used as normal GPIOs you need to disable the JTAG support on your ESP32-S2 MCU with the following
command and the proper serial port character device (eg. /dev/ttyUSB0):

	espefuse.py --port <serial-port> burn_efuse SOFT_DIS_JTAG

### 2.3 Firmware Configuration
From the 5.3 version, esp-idf uses CMAKE as software building infrastructure (project.mk is no more available).
For this reason all firmware's customization has been set in the CMakeLists.conf file. This file is parsed by the CMakeLists.txt
file using the configUtils.cmake library. In order to get the options list, please, read the documentation lines in the
[CMakeLists.txt](./CMakeLists.txt) file header.

#### 2.3.1 Authentication resistive key setting
In the original version the resistive key accepted value was set using two multiround-trimmer. The setting process was fast and easy.
But because this electric-system has been planned mainly for off-road motorcycles that solution was not adequate. In fact frequent
and hard vibrations can change the trimmer's output value. It means, the right key could become not accepted by the device.
So, at the end, I moved to a new solution: saving key's value in the NVS (Non Volatile Storage) area of the ESP32 MCU.
This solution makes the key authentication process more reliable and more secure too, because a thief cannot use the trimmers to
set on-fly a new key. In this case his project is a much much more difficult one.

To set a new accepted key's value, you should follow the next steps:
1) Creating the key you will use with the motorbike
2) Reading the new key's value
3) Creating a standard CSV file with the retrieved key's value expressed in the correct syntax
4) Converting the CSV in a blob file. 
5) Writing the blob in the ESP32's NVS partition

**[1]**
The key is a 8-pin connector with two resistors, mainly. In order to build a new one, consider the electric diagram and the
README.md file stored in the **<PROJECT-HOME>/PCB/starter_key_plug**

**[2]**
The two resistors in the key create two voltage reference values. These values are evaluated in the authentication
process. For this reason it is necessary to read these reference values and to make this step easier I wrote a dedicated
firmware <.....>. So, build it and upload it in your device. At this point plugging the key, and using the ESP-IDF
monitor (idf.py monitor) you will be able to read the two voltage values on your display.

**[3]**
You have to create a CSV file respecting the following syntax, and with the values you got at step 2

	key,type,encoding,value
	refVal_A,u16,decimal,<first-value>
	refVal_B,u16,decimal,<second-value>

**[4]**
Use the proper tool provided by the ESP-IDF framework, respecting the following syntax:

	nvs_partition_gen.py generate <your-file.csv> nvs.bin 0x6000

**[5]**
Use the proper tool provided by the ESP-IDF framework, respecting the following syntax:

	esptool.py write_flash 0x9000 nvs.bin

**[!]**
The following command erase the memory content: the installed firmware and the key's reference values too.

	idf.py erase-flash

### 2.4 Building and firmware updating process
In order to build the firmware use the following idf standard command:

	idf.py build flash

### 2.5 Debug console
In order to monitor the pins' status interactively, you can use the **debugConsole** software in the
<PROJECT-HOME>/firmware-esp32/tool folder
