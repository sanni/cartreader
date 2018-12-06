# Cartridge Reader Shield for Arduino Mega 2560

![image](https://dl.dropboxusercontent.com/s/sfbfcb031cjlo1z/red.jpg?dl=1)   

The Arduino Cart Reader is aimed at people that either already have experience with Arduino or always wanted to buy an Arduino and are searching for an interesting project to accomplish. All the Arduino source code along with the PCB files and detailed build guides can be found in this github. Happy making. 🔧🔨😊
    

#### Features: 😮 
- Reads SNES roms and reads/writes save games from and to the SNES cartridge  
  Supported cartridge types so far: LoRom, HiRom, ExHiRom, SuperFX, SuperFX2, SDD1, CX4, SPC7110, SA1 (last two chips need Adafruit Clock Generator)     
- Reads and writes SNES Satellaview 8M Memory packs    
- Reads and writes Nintendo Power Super Famicom Memory Flash Cartridges (needs Adafruit Clock Generator for best result)  
- Reflashes some Chinese or custom-made SNES repros with 29F033 or 29LV320 flashroms     
- Reads N64 roms and reads/writes save games(4K/16K Eeprom + Sram + all 3 types of Flashram)   
- Reads and writes N64 controller paks and also can test a N64 controller's buttons and thumbstick   
- Reflashes some Chinese N64 repros with Spansion S29GL256N, Fujitsu MSP55LV512, Fujitsu MSP55LV100S or Intel 4400L0ZDQ0 flashroms    
- Reads Game Boy (Color) roms and reads/writes save games   
- Reads and writes Nintendo Power Game Boy Memory Flash Cartridges   
- Programs custom-made Game Boy (Color) flashcarts with 29F016/29F032/29F033 flashrom   
- Reads Game Boy Advance roms and reads/writes most of the save games(4K Eeprom, 64K Eeprom, Sram/Fram, SST39VF512 512K flash + MX29L010 1M flash)  
- Reflashes some Chinese GBA repros with Intel 4000L0YBQ0, Macronix MX29GL128E or Fujitsu MSP55LV128 flashroms    
- Reads Sega Mega Drive roms and reads/writes save games(Sram/Fram)    
- Reads PC engine/TG16 (custom-made adapter needed)   
- Programs Flashroms like 29F016, 29F032 and 29F033, 29F1601, 29F1610, 29L3211, 29LV160, 29LV320 and S29GL032M   
- Programs 27C322 Eproms   

![image](https://dl.dropboxusercontent.com/s/nc5lblrrf49sgxb/v17_slots.png?dl=1)   

#### Be sure to check the guides in the [Wiki](https://github.com/sanni/cartreader/wiki) too. 📚

![image](https://dl.dropboxusercontent.com/s/ptswm9c7nhi6pa3/v17_back.png?dl=1)   

#### Thanks to: 😍  
   MichlK - ROM-Reader for Super Nintendo    
   Jeff Saltzman - 4-Way Button    
   Wayne and Layne - Video-Game-Shield menu    
   skaman - SNES enhancements, SA1 sram support and GB flash fix    
   nocash - Nintendo Power and GBA Eeprom commands and lots of other info    
   crazynation - N64 bus timing     
   hkz/themanbehindthecurtain - N64 flashram commands    
   jago85 - help with N64 stuff    
   Andrew Brown/Peter Den Hartog - N64 controller protocol   
   bryc - mempak   
   Shaun Taylor - N64 controller CRC functions   
   Angus Gratton - CRC32   
   Tamanegi_taro - SA1 fix, Satellaview and PCE support   
   Snes9x - SuperFX sram fix    
   zzattack - multigame pcb fix   
   Pickle - SDD1 fix   
   insidegadgets - GBCartRead   
   RobinTheHood - GameboyAdvanceRomDumper   
   YamaArashi - GBA flashrom bank switch command   
   infinest - GB Memory Binary Maker   
   moldov - SF Memory Binary Maker   
   vogelfreiheit - N64 flashram fix    
     
   ![image](https://dl.dropboxusercontent.com/s/t0igdyghvagbrwq/v17_switches.png?dl=1)   

#### Please join the discussion at: http://forum.arduino.cc/index.php?topic=158974.0 💬      
