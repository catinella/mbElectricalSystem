![Project's banner](../../../images/mbElectricalSystem_banner.png)
> Modular electrical control unit for pre-injection motorbikes  
> based on ESP32 and distributed power management

## 1.0 Files

|  Files/Dirs                          | Description                                       |
|--------------------------------------|---------------------------------------------------|
| electricSystemBox.dxf                | Electric system main-box                          |
| Koso_instrumentation_attachment.dxf  | Mechanical part used to mount the Koso's cocpit   |
| electricSystemBox_squareBars.dxf     | System used to close the box's lid                |
| motorbikeFrame_attachment_system.dxf | Small plate used to link the box to the framework |

## 2.0 Description:

### 2.1 The Electric System (main) Box
This box can contain the PCB-A and PCB-B circuits. The CAD file shows you all the 2D parts you should create starting from a
3mm aluminium sheet. Mounting the parts to be welled in correct way, can be a difficult task, but you can use the square bars
support to get an advantage.

#### 2.1.1 How to close the box?
In order to close the box's lid, you have to lock the (**electricSystemBox_squareBars.dxf**) square bars inside the box with
3mm screws, then use 5mm screws to lock the lid of the box. For the 3mm screws mounting, I suggest youto use the
[Loctite Threadlocker Red](https://www.loctiteproducts.com/products/central-pdp.html/loctite-threadlocker-red/SAP_0201OHL029W5/variation/209741.html)
substance. You should not need to remove these screws, in the future.

**[IMPORTANT]** Because the bars are used as screw nut, it SHOULD NOT be made with aluminium, it SHOULD be produced using a
stainless steel metal.

#### 2.1.2 How to attach the box to the motorbyke framework?
In order to link the box to the motorbike's framework you should weld four 6MA bolts to the (2mm) plate described in the 
**motorbikeFrame_attachment_system.dxf** drawing. The plate should be placed inside the lid, on the box's bottom side. the
protruding bolts will be used to link the device to the franework in stable way.
The plate and the bolts should be built in stainless steel metal.

### 2.2 Koso's Cokpit
As motorbike cockpit I use a [KOSO's](https://www.koso.com.tw/en) product. To link this part to the upside of the box, you need
two copy of the mechanical single part described in the **Koso_instrumentation_attachment.dxf** CAD file. The file shows you two
sides of the object (from lateral and top point of view). It should be built with aluminium and it does not require to be welded.
