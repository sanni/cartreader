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

## 2600.txt
This file stores known Atari VCS / 2600 games with the required mapping value.

Example:   
Wall-Defender   
064

Name   
mapper

mapper:   
004 = Atari 32K with RAM [F4SC]   
006 = Atari 16K with RAM [F6SC]   
008 = Atari 8K with RAM [F8SC]   
010 = UA Ltd 8K [UA]   
032 = Atari 2K   
063 = Tigervision 8K   
064 = Atari 4K (Default)   
192 = Commavid 2K [CV]   
208 = Pitfall II 10K [DPC]   
224 = Parker Bros 8K   
231 = M-Network 16K   
240 = Megaboy 64K   
244 = Atari 32K [F4]   
246 = Atari 16K [F6]   
248 = Atari 8K [F8]   
249 = Time Pilot 8K [TP]   
250 = CBS RAM Plus 12K   
254 = Activision 8K   

## 5200.txt
This file stores known Atari 5200 games with the required mapping value and cart size.

Example:   
Defender   
1,2   

Name   
mapper, cart size

mapper:   
0 = Standard 4K/8K/16K/32K   
1 = Two Chip 16K   
2 = Bounty Bob Strikes Back 40K   

cart size:   
0 = 4K   
1 = 8K   
2 = 16K   
3 = 32K   
4 = 40K   

## 7800.txt   
This file stores known Atari 7800 games with the required mapping value and cart size.   

Example:   
Rampage   
4,4   

Name   
mapper, cart size   

mapper:   
0 = Standard 16K/32K/48K [7816/7832/7848]   
1 = SuperGame 128K [78SG]   
2 = SuperGame - Alien Brigade/Crossbow 144K [78S9]   
3 = F-18 Hornet 64K [78AB]   
4 = Double Dragon/Rampage 128K [78AC]   
5 = Realsports Baseball/Tank Command/Tower Toppler/Waterski 64K [78S4]   
6 = Karateka (PAL) 64K [78S4 Variant]   
7 = Bankset switching   

cart size:   
0 = 16K   
1 = 32K   
2 = 48K   
3 = 64K   
4 = 128K   
5 = 144K   

## arccart.txt   
This file stores known Emerson Arcadia 2001 games with the required cart size.   

Example:   
American Football   
2   

Name   
cart size   

cart size:   
0 = 2K   
1 = 4K   
2 = 6K   
3 = 8K   

## c64cart.txt   
This file stores known Commodore 64 games with the required mapping value and cart size.   

Example:   
Donkey Kong   
00,3   

Name   
mapper, cart size   

mapper:   
00 = Normal 4K/8K/16K + Ultimax 8K/16K   
01 = Action Replay 32K   
02 = KCS Power Cartridge 16K   
03 = Final Cartridge III 64K   
04 = Simons Basic 16K   
05 = Ocean 128K/256K/512K   
06 = Expert Cartridge 8K   
07 = Fun Play, Power Play 128K   
08 = Super Games 64K   
09 = Atomic Power 32K   
10 = Epyx Fastload 8K   
11 = Westermann Learning 16K   
12 = Rex Utility 8K   
13 = Final Cartridge I 16K   
14 = Magic Formel 64K   
15 = C64 Game System, System 3 512K   
16 = WarpSpeed 16K   
17 = Dinamic 128K   
18 = Zaxxon, Super Zaxxon (SEGA) 20K   
19 = Magic Desk, Domark, HES Australia 32K/64K/128K   
20 = Super Snapshot 5 64K   
21 = Comal-80 64K   

cart size:   
0 = 4K   
1 = 8K   
2 = 12K   
3 = 16K   
4 = 20K   
5 = 32K   
6 = 64K   
7 = 128K   
8 = 256K   
9 = 512K   

## fairchildcart.txt   
This file stores known Fairchild Channel F games with the required cart size.   

Example:   
Hangman   
1   

Name   
cart size   

cart size:   
0 = 2K   
1 = 3K   
2 = 4K   
3 = 6K   

## msxcart.txt   
This file stores known MSX games with the required mapper, cart size and ram size.   

Example:   
Aoki Ookami - Genchou Hishi   
07,8,4   

Name   
mapper, cart size, ram size   

mapper:   
00 = NONE   
01 = ASCII8   
02 = ASCII16   
03 = CROSS BLAIM   
04 = GAME MASTER 2   
05 = HAL NOTE   
06 = HARRY FOX YUKI   
07 = KOEI   
08 = KONAMI   
09 = KONAMI SSC   
10 = MSX-DOS2   
11 = PAC/FM-PAC   
12 = R-TYPE   
13 = SUPER LODE RUNNER   

cart size:   
0 = 0K   
1 = 8K   
2 = 16K   
3 = 32K   
4 = 64K   
5 = 128K   
6 = 256K   
7 = 512K   
8 = 1024K   

ram size:   
0 = 0K   
1 = 2K   
2 = 8K   
3 = 16K   
4 = 32K   

## ody2cart.txt   
This file stores known Magnavox Odyssey 2 games with the required mapper and cart size.   

Example:   
Baseball   
0,0   

Name   
mapper, cart size   

mapper:   
0 = STANDARD   
1 = A10

cart size:   
0 = 2K   
1 = 4K   
2 = 8K   
3 = 12K   
4 = 16K   

## vectrexcart.txt   
This file stores known Vectrex games with the required cart size.   

Example:   
Mine Storm II   
0   

Name    
cart size

cart size:   
0 = 4K   
1 = 8K   
2 = 12K   
3 = 16K   
4 = 32K   
5 = 64K   

## ballycart.txt   
romsize   
0 = 2K   
1 = 4K   
2 = 8K   

## pv1000cart.txt   
romsize   
0 = 8K   
1 = 16K   

## pyuutacart.txt   
romsize   
0 = 8K   
1 = 16K   
2 = 32K   

## rcacart.txt   
romsize   
0 = 512B   
1 = 1024B (1K)   

## trs80cart.txt   
romsize   
0 = 2K   
1 = 4K   
2 = 8K   
3 = 10K   
4 = 16K   
5 = 32K   
6 = 64K   
7 = 128K   

## vic20cart.txt (rommap,romsize)   
rommap   
upper nibble = ROM0 {0x20,0x40,0x60,0x70,0xA0,0xB0}   
lower nibble = ROM1 {0x40,0x60,0xA0}   
0 = 0x20 = 0x2000   
1 = 0x24 = 0x2000/0x4000   
2 = 0x2A = 0x2000/0xA000   
3 = 0x46 = 0x4000/0x6000   
4 = 0x60 = 0x6000   
5 = 0x6A = 0x6000/0xA000   
6 = 0x70 = 0x7000   
7 = 0xA0 = 0xA000   
8 = 0xB0 = 0xB000   

romsize   
upper nibble = ROM0 {2,4,8}   
lower nibble = ROM1 {0,4,8}   
0 = 0x20 = 2K/0K 0x800   
1 = 0x40 = 4K/0K 0x1000   
2 = 0x80 = 8K/0K 0x2000   
3 = 0x44 = 4K/4K 0x1000/0x1000   
4 = 0x48 = 4K/8K 0x1000/0x2000   
5 = 0x84 = 8K/4K 0x2000/0x1000   
6 = 0x88 = 8K/8K 0x2000/0x2000   

## ti99cart.txt (mapper,gromsize,grommap,romsize)   
mapper   
0 = Normal Carts   
1 = MBX   
2 = TI-CALC [UNTESTED]   

gromsize   
0 = 0K   
1 = 6K   
2 = 12K   
3 = 18K   
4 = 24K   
5 = 30K   

grommap   
Map corresponding bits   
GROM#    BITMAP    HEX   DEC    
3     = 0000 1000 = 08 = 008   
4     = 0001 0000 = 10 = 016   
5     = 0010 0000 = 20 = 032   
7     = 1000 0000 = 80 = 128   
34    = 0001 1000 = 18 = 024   
35    = 0010 1000 = 28 = 040   
45    = 0011 0000 = 30 = 048   
56    = 0110 0000 = 60 = 096   
67    = 1100 0000 = C0 = 192   
345   = 0011 1000 = 38 = 056   
356   = 0110 1000 = 68 = 104   
456   = 0111 0000 = 70 = 112   
3456  = 0111 1000 = 78 = 120   
3457  = 1011 1000 = B8 = 184   
3467  = 1101 1000 = D8 = 216   
34567 = 1111 1000 = F8 = 248   

romsize   
0 = 0K   
1 = 4K   
2 = 8K   
3 = 12K   
4 = 16K   

## leapster.txt   
romsize   
0 = 4K   
1 = 8K   
2 = 16K   

## vsmilecart.txt   
romsize   
0 = 4K   
1 = 6K   
2 = 8K   
3 = 16K   

## ljcart.txt   
romsize   
0 = 1M   
1 = 2M   
2 = 4M   

## atari8cart.txt (slot,romsize)   
slot   
0 = left slot   
1 = right slot   

romsize   
0 = 8K   
1 = 16K   
2 = 32K   
3 = 40K   
4 = 64K   
5 = 128K   
