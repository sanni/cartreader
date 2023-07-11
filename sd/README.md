### Copy these files to the root of your SD card. If you're on Linux or MAC make sure the Windows style line endings(CRLF) don't get removed.      
Hint: You can select all the databases, right-click, properties, mark checkbox Hidden and now they won't show up in the Cart Reader's file browser.    

## gb.txt / gg.txt / md.txt / pce.txt / sms.txt / vb.txt    
These files store the ROM names and the CRC32 checksums of the complete ROM and are used only for verification at the end of the dumping process.    

Example:    
007 - The World Is Not Enough (USA, Europe).gbc    
E038E666    

Name    
CRC32   

## colv.txt / wsv.txt      
Stores the ROM name, CRC32 of the complete ROM, CRC32 of the first 512 bytes and size in KB, since the size is not stored in the ROM it is needed for dumping.    

Example:    
Artillery Duel (USA).col    
6F88FCF0,48474D52,16     

Name    
CRC32 (complete ROM), CRC32 (512 bytes), size in KB   

## gba.txt     
This file stores the GBA database which is needed because the save type and rom size are not stored inside the rom.     

Example:     
007 - Everything or Nothing (USA, Europe) (En,Fr,De).gba      
9D4F1E18,BJBE,08,1     

Name      
CRC32, game id, size in MByte, savetype      

Savetypes:    
0 = Unknown or no save   
1 = 4k Eeprom   
2 = 64K Eeprom   
3 = 256K Sram   
4 = 512K Flash   
5 = 1024K Flash   
6 = 512K Sram   

## intv.txt     
This file stores the Intellivision database which is needed because the save type and rom size are not stored inside the rom.     

Example:     
Air Strike (USA) (Proto).int    
2C668249,1BB8CBB9,0,08,0   

Name      
CRC32(whole ROM), CRC32(first 512bytes), mapper, size in KB, save   

## n64.txt  
This file stores the N64 database which is needed because the save type and rom size are not stored inside the rom.     
The CRC32 checksum is used to verify a good dump.     

Example:    
007 - The World Is Not Enough (Europe) (En,Fr,De).z64     
002C3B2A,3B941695,32,0     

Name    
CRC32, ROM internal CRC1, size in MByte, savetype    

Savetypes:  
0 = no save chip  
1 = SRAM  
4 = Flashram  
5 = 4K Eeprom  
6 = 16K Eeprom  

## nes.txt  
This file stores the CRC32 of the complete ROM minus the iNES header, the CRC32 calculated over the first 512 bytes of the first or last(MMC3) PRG bank and the iNES Header. The iNES header holds all the mapping info.      

Example:   
89 Dennou Kyuusei Uranai (Japan).nes    
BA58ED29,716956B4,4E45531A100010080000070700000001    

Name    
CRC32 (prg+chr combined), CRC32 (512 bytes), iNES Header   

## snes.txt    
This file is needed for odd sized SNES games like Final Fantasy (JAP), Super Metroid(US/JAP) or Tales of Symphonia. Without this file you will get overdumps and the checksum calculation will fail.    

Example:   
2020 Super Baseball (Japan).sfc
E95A3DD7,0C19,379A6FFB,12,024     

Name     
CRC32, internal checksum, CRC32 of header area(0xFFB0-0xFFFF), size in Mbit, number of banks (lorom needs twice as many banks as hirom for the same rom size, lorom: 32kb banks, hirom: 64kb banks)

## loopy.txt
This file stores known Casio Loopy games, including the name and the internal checksum found at 000008h in the ROM header.
Note that most ROM packs you will find are incorrectly dumped little-endian. These CRCs are for the big-endian dump, including any padding. ROMs are 2MB/16Mbit or 3MB/24Mbit.

Example:	
Wanwan Aijou Monogatari.bin		
D90FE762	

Name     
Checksum
