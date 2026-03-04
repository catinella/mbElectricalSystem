![Project's banner](../images/mbElectricalSystem_banner.png)

## 1.0 Files

|       Files/Directories        | Description                          |
|--------------------------------|--------------------------------------|
| Connectors.txt                 | Cables's connectors pin-out          |
| create_fabric_files.sh         | Script to create the fabric files    |
| kicad_symbols                  | Non standard KiCAD symbols libraries |
| LICENSE.txt                    | The project's licence file (\*)      |
| pcb-a                          | Controller stage's PCB               |
| pcb-b                          | Power stage's PCB                    |
| starter_key_plug               | The schema of the resistive key      |
| images                         | The images used by this file         |

(*) This project is covered by the GPL-3. Please, read that file for further information

## 2.0 Description:
This folder contains all files you need to get and/or modify your own PCBs.
To perform these actions you need to install [KiCAD](https://www.kicad.org) software (ver >= 9.0), a free software suite for
electronic design automation (EDA).

## 2.1 Internals:
For the **power stage** I have use the [TPCA8120](https://static.chipdip.ru/lib/624/DOC060624975.pdf) P-Channel MOS. Its
low Drain-source on-resistance (4mOhm) allows me to avoid to waste electric power with high current too. For eample
with a 60W (5A) light it wase just 20mW!! It can also support 10A current and it is very small.

To reduce the number of fuses, the direction indicator LEDs use a [MSMF050](https://www.bourns.com/docs/product-datasheets/mf-msmf.pdf)
PTC Resettable Fuse.

As in all motorbikes, in order to avoid the engine can be started when the motorbike is locked, the proper CDI pin must be
connected to the GND. To achiueve this result and to protect the circuit from unknown electric voltage from the CDI, I have 
used the relay [HK19-DC12V](https://grobotronics.com/images/datasheets/HK19F-12V.pdf) instead of a modern N-Channel MOS.

## 3.0 Devices building:
The following images shows you the PCB shapes they should be at the end
![PCB](./images/pcb-a.png)
![PCB](./images/pcb-b.png)

