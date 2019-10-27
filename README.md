# DIY Cartridge Reader for Arduino Mega 2560
This Cart Reader is aimed at people that either already have experience with Arduino or always wanted to buy an Arduino and are searching for an interesting project to accomplish. All the Arduino source code along with the PCB files and detailed build guides can be found in this github. Happy making. 🔧🔨😊

For any questions or issues please join the discussion on the [Arduino Forum](http://forum.arduino.cc/index.php?topic=158974.9001).  
Be sure to check the guides in the [Wiki](https://github.com/sanni/cartreader/wiki) too. 

![image](https://dl.dropboxusercontent.com/s/nk0eo146bpljk6s/v17_carts.png?dl=1)     

#### Features:  
- Completely stand-alone, does not need a PC to operate (unless for updating firmware)    
- Portable if fitted with battery add-on or suitable power bank  
- Easy to modify open-source code, write your own extensions and share them with others      

#### Supported Systems:    
- Reads NES and Famicom cartridges    
- Reads SNES roms and reads/writes save games from and to the SNES cartridge  
  Supported cartridge types so far: LoRom, HiRom, ExHiRom, SuperFX, SuperFX2, SDD1, CX4, SPC7110, SA1 (last two chips need Adafruit Clock Generator)     
- Reads and writes SNES Satellaview 8M Memory packs    
- Reads and writes Nintendo Power Super Famicom Memory Flash Cartridges (needs Adafruit Clock Generator for best result)  
- Reflashes some Chinese or custom-made SNES repros with AM29F032, MBM29F033 or MX29LV320 flashroms     
- Reads N64 roms and reads/writes save games(4K/16K Eeprom + Sram + all 3 types of Flashram), Proto carts are not supported yet  
- Reads and writes N64 controller paks and also can test a N64 controller's buttons and thumbstick   
- Reflashes some Chinese N64 repros with S29GL128N, S29GL256N, MSP55LV512, MSP55LV100S, MX29LV640, i512M29EW or i4400L0ZDQ0 flashroms   
- Reflashes N64 Gamesharks with SST 29LE010 eeproms     
- Reads Game Boy (Color) roms and reads/writes save games   
- Reads and writes Nintendo Power Game Boy Memory Flash Cartridges   
- Programs custom-made Game Boy (Color) flashcarts with AM29F016, AM29F032, MBM29F033 flashrom   
- Programs EMS GB Smart 32M flash carts    
- Reads Game Boy Advance roms and reads/writes most of the save games(4K Eeprom, 64K Eeprom, Sram/Fram, SST39VF512 512K flash + MX29L010 1M flash)  
- Reflashes some Chinese GBA repros with i4000L0YBQ0, i4400L0ZDQ0, MX29GL128E, MSP55LV128 or PC28F256M29 flashroms    
- Reads Sega Mega Drive roms and reads/writes save games(Sram/Fram, Eeprom)    
- Reads Sega Master System roms (only default mapper and no SRAM supported so far)     
- Reads PC engine/TG16 cartridges (user-made adapter needed, see pinout.xls)   
- Reads WonderSwan cartridges (user-made adapter needed, see pinout.xls)   
- Programs flashrom chips like AM29F016B/D, AM29F032B, MBM29F033C, MX29F1601, MX29F1610, MX29L3211, MX29LV160, MX29LV320, S29GL032M, MX26L6420, MBM29F800BA, AM29F800BB, LH28F016SUT, AM29F400AB, E28FXXXJ3A and AM29LV033C    
- Programs M27C322 eproms    


[![](https://dl.dropboxusercontent.com/s/h2e08skmn9pbi2y/savegameyouprev.jpg?dl=1)](https://www.youtube.com/watch?v=r0J9Dplejjg)   
