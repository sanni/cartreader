![image](https://dl.dropboxusercontent.com/s/v2ds2zfj1bxuqr9/logos.png?dl=1)   

# Open Source Cartridge Reader
This project represents a community-driven effort to provide an easy to build and easy to modify cartridge dumper.     
Happy making. 🔧🔨😊     

For any questions or issues please also visit the accompanying thread in the [Arduino Forum](http://forum.arduino.cc/index.php?topic=158974.9001).    
And be sure to check the guides in the [Wiki](https://github.com/sanni/cartreader/wiki) too.    

#### Features:  
- Completely stand-alone, does not need a PC to operate (unless for updating firmware)    
- Easy to modify open-source code, write your own extensions and share them with others      
- Portable thanks to a battery  
- Modular design using mostly off-the-shelf components    

![image](https://dl.dropboxusercontent.com/s/70zaxs9lyyuxvqe/IMG_0799.jpg?dl=1)   

#### Supported Systems:    
- Reads NES, Famicom and Family Basic cartridges including save    
- Supports Mapper 30/NESmaker and flashes INL NM30 boards        
- Reads SNES roms and reads/writes save games from and to the SNES cartridge  
  Supported cartridge types so far: LoRom, HiRom, ExHiRom, DSP, SuperFX, SuperFX2, SDD1, CX4, SPC7110, SA1 (last two chips need Adafruit Clock Generator)     
- Reads and writes SNES Satellaview 8M Memory packs (BS-X cartridge and Adafruit Clock Generator needed)   
- Reads and writes Nintendo Power Super Famicom Memory Flash Cartridges (needs Adafruit Clock Generator for best result)  
- Reflashes some Chinese or custom-made SNES repros with AM29F032, MBM29F033, MX29LV320 or MX29LV640 flashroms     
- Reads N64 roms and reads/writes save games(4K/16K Eeprom + Sram + all 3 types of Flashram), Eeprom needs Adafruit Clockgen by default, Proto carts are not supported yet    
- Reads and writes N64 controller paks and also can test a N64 controller's buttons and thumbstick   
- Reflashes some Chinese N64 repros with S29GL type flashroms   
- Reflashes N64 Gamesharks with SST 29LE010 eeproms     
- Reads Game Boy (Color) roms and reads/writes save games   
- Reads and writes Nintendo Power Game Boy Memory Flash Cartridges   
- Programs custom-made Game Boy (Color) flashcarts with AM29F016, AM29F032, MBM29F033 flashrom   
- Programs EMS GB Smart 32M flash carts    
- Reads Game Boy Advance roms and reads/writes most of the save games(4K Eeprom, 64K Eeprom, Sram/Fram, SST39VF512 512K flash + MX29L010 1M flash)  
- Reflashes some Chinese GBA repros with i4000L0YBQ0, i4400L0ZDQ0, MX29GL128E, MSP55LV128, PC28F256M29 or M29W128GH flashroms    
- Reads Sega Mega Drive roms and reads/writes save games(Sram/Fram, Eeprom)    
- Reads Sega Master System roms and saves (using Retrode or Raphnet adapter)       
- Reads Sega Game Gear roms and saves(using Retrode adapter)    
- Reads PC engine/TG16 cartridges (using Retrode TG16 adapter)   
- Reads WonderSwan cartridges (using V3 adapters)   
- Reads NeoGeo Pocket cartridges (using V3 adapters)   
- Programs flashrom chips like AM29F016B/D, AM29F032B, MBM29F033C, MX29F1601, MX29F1610, MX29L3211, MX29LV160, MX29LV320, S29GL032M, MX26L6420, MBM29F800BA, AM29F800BB, LH28F016SUT, AM29F400AB, E28FXXXJ3A and AM29LV033C (using V3 adapters)      
- Programs M27C322 eproms (using V3 adapters)      
