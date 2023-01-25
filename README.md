![image](https://dl.dropboxusercontent.com/s/ioc5oewzcuvs8nz/logos.png?dl=1)   

# Open Source Cartridge Reader
This project represents a community-driven effort to provide an easy to build and easy to modify cartridge dumper.     
Happy making. 🔧🔨😊     

For any questions you can use the [Discussions](https://github.com/sanni/cartreader/discussions) and/or [Issues](https://github.com/sanni/cartreader/issues) section here on Github or visit the accompanying thread in the [Arduino Forum](http://forum.arduino.cc/index.php?topic=158974.9001).    
And be sure to check the guides in the [Wiki](https://github.com/sanni/cartreader/wiki) too.    

![image](https://dl.dropboxusercontent.com/s/3lrn7xh3f7h6jre/HW5_front.png?dl=1)   

#### Features:  
- Completely stand-alone, does not need a PC to operate (unless for updating firmware)    
- Easy to modify open-source code, write your own extensions and share them with others      
- Portable thanks to a battery/power bank  
- Modular design using mostly off-the-shelf components    

![image](https://dl.dropboxusercontent.com/s/w99hewh6ors3awb/HW5_side.png?dl=1)   

#### Supported Systems:    
- Reads official NES, Famicom and Family Basic cartridges including save    
- Supports Mapper 30/NESmaker and flashes INL NM30 boards        
- Reads SNES roms and reads/writes save games from and to the SNES cartridge  
  Supported cartridge types so far: LoRom, HiRom, ExHiRom, DSP, SuperFX, SuperFX2, SDD1, CX4, SPC7110, SA1 (last two chips need Adafruit Clock Generator)     
- Reads and writes SNES Satellaview 8M Memory packs (BS-X cartridge and Adafruit Clock Generator needed)   
- Reads and writes Nintendo Power Super Famicom Memory Flash Cartridges (needs Adafruit Clock Generator for best result)  
- Reflashes some Chinese or custom-made SNES repros with AM29F032, MBM29F033, MX29LV320 or MX29LV640 flashroms     
- Reads N64 roms and reads/writes save games(4K/16K Eeprom + Sram + all 3 types of Flashram), Eeprom needs Adafruit Clockgen, Proto carts are not supported yet    
- Reads and writes N64 controller paks and also can test a N64 controller's buttons and thumbstick   
- Reflashes some Chinese N64 repros with S29GL type flashroms   
- Reflashes N64 Gamesharks with SST 29LE010(and similar) eeproms     
- Reads Game Boy (Color) roms and reads/writes save games   
- Reads and writes Nintendo Power Game Boy Memory Flash Cartridges   
- Programs custom-made Game Boy (Color) flashcarts with AM29F016, AM29F032, MBM29F033 flashrom   
- Programs EMS GB Smart 32M flash carts    
- Programs Gameboy Camera Flashcart      
- Reads Game Boy Advance roms and reads/writes the save games(4K Eeprom, 64K Eeprom, Sram/Fram, 512K flash, 1M flash)  
- Reflashes some Chinese GBA repros with i4000L0YBQ0, i4400L0ZDQ0, MX29GL128E, MSP55LV128, PC28F256M29 or M29W128GH flashroms    
- Reads Virtual Boy cartridges (using custom adapter)    
- Reads Sega Mega Drive roms and reads/writes save games(Sram/Fram, Eeprom)    
- Reads Sega Master System roms and saves           
- Reads Sega Game Gear roms and saves (using Retrode adapter)    
- Reads some Sega Mark III cartridges (using Raphnet adapter)  
- Reads some Sega SG-1000 cartridges (using Raphnet adapter)      
- Reads some Sega Cards (using Card Catcher and Raphnet Mark III Adapter)   
- Reads PC engine/TG16 cartridges (using Retrode or custom adapter)   
- Reads WonderSwan cartridges (using custom adapter)   
- Reads NeoGeo Pocket cartridges (using custom adapter)    
- Reads Intellivision cartridges (using custom adapter)    
- Reads ColecoVision cartridges (using custom adapter)    
- Reads Benesse Pocket Challenge W cartridges (using custom adapter)    
- Reads Watara Supervision cartridges (using custom adapter)    
- Reads Atari 2600 cartridges (using custom adapter)    
- Reads Emerson Arcadia 2001 cartridges (using custom adapter)    
- Reads Fairchild Channel F cartridges (using custom adapter)    
- Reads Magnavox Odyssey 2 cartridges (using custom adapter)    
- Programs flashrom chips like AM29F016B/D, AM29F032B, MBM29F033C, MX29F1601, MX29F1610, MX29L3211, MX29LV160, MX29LV320, S29GL032M, MBM29F800BA, AM29F800BB, LH28F016SUT, AM29F400AB, E28FXXXJ3A and AM29LV033C (using custom adapter)       

![image](https://dl.dropboxusercontent.com/s/oi7c2radgblylyz/HW5_slots.png?dl=1)  

#### Open Source Licenses:    
- Software(everything in Cart_Reader folder) = GPL v3   
- Hardware(everything in hardware folder) = CC-BY-4.0   
