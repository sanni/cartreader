To order adapter PCBs go to https://jlcpcb.com/ and click the "Quote now" button then upload the *_adapter.zip file of the adapter you want by clicking the "Add your Gerber file" button. It will take a while. Then in the settings below you will find "PCB Thickness" which is set to 1.6, you need to set it to 1.2 for most of the adapters or the adapter will not fit inside the SNES/N64 cart slot.    
The other settings are fine by default. Next, click the "Save to cart" button and if you want to order another adapter type click on "Add new item" button and upload the next zip file, set the thickness to 1.2mm again if needed, add to cart, and so on.     
You should get a free shipping coupon if this is your first order and you sign up on this page: https://jlcpcb.com/cheapest-PCB-way      

You can also order from [Oshpark](https://oshpark.com/) but you need to either [rename *_adapter.GML to *_adapter.GKO](https://www.dropbox.com/s/0rcvhalgeu11sf8/rename.jpg?dl=0) inside the *_adapter.zip or upload the *_adapter.brd from the [eagle design files directory](https://github.com/sanni/cartreader/tree/master/pcb/eagle%20design%20files) instead or the Oshpark website will display an error. Oshpark is great for ordering the smaller PCBs but very expensive for larger boards.   

#### nes_adapter.zip is an add-on for reading NES carts, [PCB thickness needs to be changed to 1.2mm](https://dl.dropboxusercontent.com/s/va1c72073cqfy90/pcb12.jpg?dl=1), this is very important or else it won't fit into the SNES slot. You can order a 2.5mm 72pin NES slot [here](https://www.aliexpress.com/item/32827561164.html).     

![image](https://dl.dropboxusercontent.com/s/z2atlcly642sewj/nes_adapter.png?dl=1)   

#### famicom_adapter.zip is an add-on for reading Famicom carts, [PCB thickness needs to be changed to 1.2mm](https://dl.dropboxusercontent.com/s/va1c72073cqfy90/pcb12.jpg?dl=1), this is very important or else it won't fit into the SNES slot. You can order a 2.54mm 60pin Famicom slot [here](https://www.aliexpress.com/item/32827561249.html).     

![image](https://dl.dropboxusercontent.com/s/w89ivzvuzk6hf5b/famicom_adapter.png?dl=1)   

#### sms_adapter.zip is an add-on for reading Sega Master System carts. You can order a 2.54mm 50pin SMS slot [here](https://www.aliexpress.com/item/32818469880.html). The adapter is based on the [design by Raphnet](https://www.raphnet.net/electronique/sms_to_smd/index_en.php). For use with the Cart Reader ignore the SMD footprints on the PCB, the adapter does not need any components. I only bridged R5 to connect the reset line, although I'm not sure if this is even needed. Unlike the other adapters this one plugs into the MD slot.   

![image](https://dl.dropboxusercontent.com/s/r6lavgoaccjtrz7/sms_adapter.png?dl=1)   

#### wonderswan_adapter.zip is an add-on for reading WonderSwan carts. [PCB thickness needs to be changed to 1.2mm](https://www.dropbox.com/s/va1c72073cqfy90/pcb12.jpg?dl=0), this is very important or else it won't fit into the SNES slot. (Optional) Install C1 and C2 with 10uF/16v 1210 package tantalum capacitor.

![image](https://dl.dropboxusercontent.com/s/755249v8smcuoft/wonderswan_adapter.png?dl=1)   

#### ngp_adapter.zip is an add-on for reading NeoGeo Pocket carts. [PCB thickness needs to be changed to 1.2mm](https://www.dropbox.com/s/va1c72073cqfy90/pcb12.jpg?dl=0), this is very important or else it won't fit into the SNES slot. 

![image](https://dl.dropboxusercontent.com/s/yvutwme8n7d4tiy/ngp_adapter.png?dl=1)   

#### flash_adapter.zip is an add-on for writing flashroms like the 29F032, 29L3211, 29LV160, [PCB thickness needs to be changed to 1.2mm](https://dl.dropboxusercontent.com/s/va1c72073cqfy90/pcb12.jpg?dl=1), this is very important or else it won't fit into the SNES slot.   
   
![image](https://dl.dropboxusercontent.com/s/afrfmiuwvmvg9px/flash_adapter.png?dl=1)   

#### eprom_adapter.zip is an add-on for writing an 27C322 eprom, [PCB thickness needs to be changed to 1.2mm](https://www.dropbox.com/s/va1c72073cqfy90/pcb12.jpg?dl=0), this is very important or else it won't fit into the SNES slot.     
   
![image](https://dl.dropboxusercontent.com/s/ldmtkjv7xsgtwyg/27c322_adapter.png?dl=1)   

gba_adapter.zip and md_adapter.zip are only needed if you got an older Cart Reader version that only has a SNES and N64 slot and is missing the GBA and Mega Drive/Genesis slots.    
