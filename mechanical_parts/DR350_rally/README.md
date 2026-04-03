![Project's banner](../../images/mbElectricalSystem_banner.png)
> Modular electrical control unit for pre-injection motorbikes  
> based on ESP32 and distributed power management

## 1.0 Files

|  Files/Dirs  | Description                                     |
|--------------|-------------------------------------------------|
| frontSide    | Circuits container for the motorbike front side |
| rearSide     | PCB container for the motorbike rear side       |

## 2.0 Description:
In the original DR350's electric system, as every other with the same age, the electric current flows in (1mmq) big wires from
the commands to the services (lights, horn, relay...). But the commands are located on the hand-bar, in the front
side of the motorbike, while some services (turn-indicators, stop-light, emgine-start-relay...) is located in the rear one.
So, to reduce the number and the size of the wires that runs in the electric system, I have placed one power stage in the front
side and another in the rear side. Every one of them has its own metal box container.
