### Copy these files to the root of your SD card. If you're on Linux or MAC make sure the Windows style line endings(CRLF) don't get removed.      
Hint: You can select all the databases, right-click, properties, mark checkbox Hidden and now they won't show up in the Cart Reader's file browser.    

## gb.txt / md.txt / pce.txt / sms.txt    
These files store the ROM names and the CRC32 checksums.    

Example:    
007 - The World Is Not Enough (USA, Europe).gbc    
E038E666    

Name    
CRC32   

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
This file stores the iNES Header, the CRC32 of the first 16 bytes after the iNES header and the CRC32 of the complete ROM minus the iNES header.      

Example:   
89 Dennou Kyuusei Uranai (Japan).nes    
BA58ED29,716956B4,4E45531A100010080000070700000001    

Name    
CRC32 (prg+chr combined), CRC32 (16 bytes), iNES Header   

## snes.txt    
This file is needed for odd sized SNES games like Final Fantasy (JAP), Super Metroid(US/JAP) or Tales of Symphonia. Without this file you will get overdumps and the checksum calculation will fail.    

Example:   
'96 Zenkoku Koukou Soccer Senshuken (Japan).sfc     
05FBB855,70DE,12,048      

Name     
CRC32, internal checksum, size in Mbit, number of banks (lorom needs twice as many banks as hirom for the same rom size, lorom: 32kb banks, hirom: 64kb banks)    

## snes_clk.txt    
Calibration data for clock generator.     
