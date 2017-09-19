#### Copy these files to the root of your SD card.  

#### gba.txt  
This file stores the GBA database which is needed because the save type and rom size are not stored inside the rom. 

Example:  
A22J,08,3   
game id, size in MByte, savetype  

Savetypes:   
0 = Unknown or no save   
1 = 4k Eeprom   
2 = 64K Eeprom   
3 = 256K Sram   
4 = 512K Flash   
5 = 1024K Flash   
6 = 512K Sram   

#### n64.txt  
This file stores the N64 database which is needed because the save type and rom size are not stored inside the rom.  
The CRC32 checksum is used to verify a good dump.  

Example:  
002c3b2a,NO7P,32,0  
CRC32, game id, size in MByte, savetype  

Savetypes:  
0 = no save chip  
1 = SRAM  
4 = Flashram  
5 = 4K Eeprom  
6 = 16K Eeprom  

#### snes.txt  
This file is needed for odd sized SNES games like Final Fantasy (JAP), Super Metroid(US/JAP) or Tales of Symphonia. Without this file you will get overdumps and the checksum calculation will fail. There are still a lot games missing from this list.    

Example:  
A172,24,48  
checksum, size in Mbit, number of banks (lorom needs twice as many banks as hirom for the same rom size)  

#### hirom64.map  
This is a Nintendo Power mapping file that changes the mapping to a single 4MB HiRom game with 64Kbit/8KByte save. The first byte is used to specify the mapping.  

Bit0-1 SRAM Size (0=2K, 1=8K, 2=32K, 3=None) ;ie. 2K SHL (N*2)  
Bit2-4 ROM Size (0=512K, 2=1.5M, 5=3M, 7=4M) ;ie. 512K*(N+1)  
Bit5 Zero (maybe MSB of ROM Size for carts with three FLASH chips) (set for HIROM:ALL)  
Bit6-7 Mode (0=Lorom, 1=Hirom, 2=Forced HIROM:MENU, 3=Forced HIROM:ALL)  
More info: http://problemkaputt.de/fullsnes.htm#snescartnintendopowerflashcard  

Example:  
0x5d = 0b 01 0 111 01  
01 -> Hirom  
0  
111 -> 4M  
01 -> 8K  

#### lorom256.map  
This is a Nintendo Power mapping file that changes the mapping to a single 4MB LoRom game with 256Kbit/32 KByte save.  

Example:  
0x1e = 0b 00 0 111 10  
00 -> Lorom  
0  
0b111 -> 7 -> 4M  
0x10 -> 2 ->32k  
