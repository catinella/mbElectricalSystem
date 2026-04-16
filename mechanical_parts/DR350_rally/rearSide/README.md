![Project's banner](../../../images/mbElectricalSystem_banner.png)
> Modular electrical control unit for pre-injection motorbikes  
> based on ESP32 and distributed power management

## 1.0 Files

|            Files/Dirs                     |                 Description                  |
|-------------------------------------------|----------------------------------------------|
| electricSystemBox.dxf                     | The box that will contain the PCB-C          |
| electricSystemBox_internalSupports_a.dxf  | External support for the box's cap           |
| electricSystemBox_internalSupports_b.dxf  | Internal support to mount and close the cap  |

## 2.0 Description:
The CAD drawings belong to this folder are what you need to produce a strange shape box for PCB-C that fits in a Suzuki
DR350 motorcycle. 
The "electricSystemBox.dxf" file contains all the 2D parts and they must be realized starting from a 3mm aluminium sheet.
The "electricSystemBox_internalSupports_a.dxf" describe a single 3D object. It must be produced starting from an aluminium
square bar. Also the "electricSystemBox_internalSupports_b.dxf" contains a 3D object, but you need to produce 4 copy of
the object. 

### 2.1 Welding
In order to create the box you have to weld all the 2D parts (except the cap). The internal supports can help you to keep
the parts together in order to be welded. Use 4MA screws and nuts to assembly the lateral box's sides. Then weld the box's 
bottom side. Now, you can weld the internal supports to the box's sides and remove the screws and nuts. You have created
the box successfylly.

Now, to complete the cap, (5MA) thread the vertical holes of the internal supports. Unfortunately, these hole cannot be used
directly, because their holding power have been compromised by the side holes. So screw a 5MA screw from the box's bottom side
ad lock it usig the
[Loctite Threadlocker Red](https://www.loctiteproducts.com/products/central-pdp.html/loctite-threadlocker-red/SAP_0201OHL029W5/variation/209741.html)
Now, you can locking the cap using 4 5MA self-locking nuts

