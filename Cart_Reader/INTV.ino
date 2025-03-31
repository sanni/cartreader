//******************************************
// INTELLIVISION MODULE
//******************************************
#ifdef ENABLE_INTV

// Mattel Intellivision
// Cartridge Pinout
// 44P 2.54mm pitch connector
//
//    TOP SIDE             BOTTOM SIDE
//      (EVEN)             (ODD)
//              +-------+
//         GND -| 2   1 |- GND
//       CBLNK -| 4   3 |- /MSYNC
//     EXT AUD -| 6   5 |- DB7
//     EXT VID -| 8   7 |- DB8
//        MCLK -| 10  9 |- DB6
//       RESET -| 12 11 |- DB9
//         SR1 -| 14 13 |- DB5
//       VOICE -| 16 15 |- DB10
//       VOICE -| 18 17 |- DB4
//         GND -| 20 19 |- DB11
//         GND -| 22 21 |- DB3
//         GND -| 24 23 |- DB12
//         GND -| 26 25 |- DB13
//         GND -| 28 27 |- DB2
//      /BUSAK -| 30 29 |- DB14
//         BC1 -| 32 31 |- DB1
//         BC2 -| 34 33 |- DB0
//        BDIR -| 36 35 |- DB15
//        BDIR -| 38 37 |- BDIR
//         BC2 -| 40 39 |- BC2
//         BC1 -| 42 41 |- BC1
//         GND -| 44 43 |- VCC(+5V)
//              +-------+

// CONTROL PINS:
// /MSYNC(PH3)- PIN 3  [ROM PIN 14]
// BC1(PH4)   - PIN 41 [ROM PIN 28]
// BC2(PH5)   - PIN 39 [ROM PIN 27]
// BDIR(PH6)  - PIN 37 [ROM PIN 26]
// NOTE: BDIR/BC1/BC2 ONLY CONNECTED TO BOTTOM (ODD) PINS

// NOT CONNECTED:
// RESET(PH0) - N/C - GND IN CART
// MCLK(PH1)  - N/C - GND IN CART

// BUS PROTOCOL COMMANDS - BDIR/BC1/BC2
// NO ACTION (NACT):           0/0/0
// ADDR TO REG (ADAR):         0/1/0
// INT ADDR TO BUS (IAB):      0/0/1
// DATA TO BUS (DTB):          0/1/1
// BUS TO ADDR (BAR):          1/0/0
// DATA WRITE (DW):            1/1/0
// DATA WRITE STROBE (DWS):    1/0/1
// INTERRUPT ACK (INTAK):      1/1/1

// Cart Configurations
// Format = {mapper,romlo,romhi,ramsize}
static const byte PROGMEM intvmapsize[] = {
  0, 0, 3, 0,  // default mattel up to 32K (8K/12K/16K/24K/32K)
  1, 2, 4, 0,  // demo cart 16K, championship tennis 32K, wsml baseball 48K
  2, 2, 4, 0,  // up to 48K (16K/32K/48K)
  3, 5, 5, 0,  // tower of doom 48K
  4, 2, 2, 1,  // uscf chess 16K + RAM 1K
  5, 3, 4, 0,  // congo bongo/defender/pac-man 24K, dig dug 32K
  6, 2, 2, 0,  // centipede 16K
  7, 2, 2, 0,  // imagic carts 16K
  8, 2, 2, 0,  // mte-201 test cart 16K
  9, 4, 4, 2,  // triple challenge 32K + RAM 2K
};

byte intvmapcount = 10;  // (sizeof(mapsize)/sizeof(mapsize[0])) / 4;
boolean intvmapfound = false;
byte intvmapselect;
int intvindex;

const byte INTV[] PROGMEM = { 8, 12, 16, 24, 32, 48 };
byte intvlo = 0;  // Lowest Entry
byte intvhi = 5;  // Highest Entry

byte intvmapper;
byte intvsize;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptionsINTV[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void setup_INTV() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output (UNUSED)
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //      ---(PH0)   ---(PH1)  /MSYNC(PH3) BC1(PH4)   BC2(PH5)  BDIR(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (DB0-DB15) to Input
  DDRC = 0x00;
  DDRA = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)  /MSYNC(PH3) BC1(PH4)   BC2(PH5)  BDIR(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Pins HIGH
  PORTF = 0xFF;       // A0-A7
  PORTK = 0xFF;       // A8-A15
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_INTV();
  strcpy(romName, "INTV");

  mode = CORE_INTV;
}

void intvMenu() {
  convertPgm(menuOptionsINTV, 4);
  uint8_t mainMenu = question_box(F("INTELLIVISION MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_INTV();
      setup_INTV();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_INTV();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper
      setMapper_INTV();
      checkMapperSize_INTV();
      setROMSize_INTV();
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// INTELLIVISION BUS PROTOCOL COMMANDS
//******************************************
// CONTROL PINS -         BDIR/BC1/BC2
// NO ACTION (NACT):         0/0/0
// ADDR TO REG (ADAR):       0/1/0 [NOTE: ORDER BDIR/BC1/BC2]
// DATA TO BUS (DTB):        0/1/1
// BUS TO ADDR (BAR):        1/0/0
// DATA WRITE (DW):          1/1/0 [NOTE: ORDER BDIR/BC1/BC2]
// DATA WRITE STROBE (DWS):  1/0/1 [NOTE: ORDER BDIR/BC1/BC2]

// IMMEDIATE MODE DATA READ SEQUENCE: BAR-NACT-DTB-NACT
// IMMEDIATE MODE DATA WRITE SEQUENCE: BAR-NACT-DW-DWS-NACT
// DIRECT ADDRESSING MODE READ SEQUENCE: BAR-NACT-ADAR-NACT-DTB-NACT

// NO ACTION (NACT) - 0/0/0
void NACT_INT() {
  // Switch BC1(PH4) + BC2(PH5) + BDIR(PH6) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 5) & ~(1 << 6);
  // DB0..DB15 INPUT
  DDRC = 0x00;
  DDRA = 0x00;
}

// SET ADDRESS - BUS TO ADDR (BAR) - 1/0/0
void BAR_INT() {
  // Switch BDIR(PH6) to HIGH
  PORTH |= (1 << 6);
  // Switch BC1(PH4) + BC2(PH5) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 5);
  // Address OUT
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// READ DATA - DATA TO BUS (DTB) - 0/1/1
void DTB_INT() {
  // Switch BDIR(PH6) to LOW
  PORTH &= ~(1 << 6);
  // Switch BC1(PH4) + BC2(PH5) to HIGH
  PORTH |= (1 << 4) | (1 << 5);
  // Data IN
  DDRC = 0x00;
  DDRA = 0x00;
}

// ADDRESS DATA TO ADDRESS REGISTER (ADAR) - 0/1/0
void ADAR_INT() {
  // Switch BC2(PH5) + BDIR(PH6) to LOW
  PORTH &= ~(1 << 5) & ~(1 << 6);
  // Switch BC1(PH4) to HIGH
  PORTH |= (1 << 4);
}

// DATA WRITE PAIRED COMMAND - DW + DWS
// DATA SHOULD BE STABLE ACROSS BOTH

// DATA WRITE (DW) - 1/1/0
void DW_INT() {
  // Switch BC1(PH4) + BDIR(PH6) to HIGH
  PORTH |= (1 << 4) | (1 << 6);
  // Switch BC2(PH5) to LOW
  PORTH &= ~(1 << 5);
}

// DATA WRITE STROBE (DWS) - 1/0/1
void DWS_INT() {
  // Switch BC2(PH5) + BDIR(PH6) to HIGH
  PORTH |= (1 << 5) | (1 << 6);
  // Switch BC1(PH4) to LOW
  PORTH &= ~(1 << 4);
}

//******************************************
// READ CODE
//******************************************

uint16_t readData_INTV(uint32_t addr) {
  PORTC = addr & 0xFF;
  PORTA = (addr >> 8) & 0xFF;

  BAR_INT();
  // Wait for bus
  // 5 x 62.5ns = 312.5ns
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  NACT_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DTB_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  uint16_t ret = (((PINA & 0xFF) << 8) | (PINC & 0xFF));

  NACT_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  return ret;
}

// MAPPER ROM ADDRESSES
// 0: 0x50-0x70,0xD0-0xE0,0xF0-0x100,           // default mattel up to 32K (8K/16K/24K/32K)
// 1: 0x50-0x70,0xD0-0x100,                     // demo cart 16K, championship tennis 32K, wsml baseball 48K
// 2: 0x50-0x70,0x90-0xC0,0xD0-0xE0,            // up to 48K (16K/32K/48K)
// 3: 0x50-0x70,0x90-0xB0,0xD0-0xE0,0xF0-0x100, // tower of doom 48K
// 4: 0x50-0x70,                                // uscf chess 16K + RAM 1K
// 5: 0x50-0x80,0x90-0xC0,                      // congo bongo/defender/pac-man 24K, dig dug 32K
// 6: 0x60-0x80,                                // centipede 16K
// 7: 0x48-0x68,                                // imagic carts 16K
// 8: 0x50-0x60,0x70-0x80,                      // mte-201 test cart 16K
// 9: 0x50-0x70,0x90-0xB0,[0xC0-0xC8,0xD0-0xD8] // triple challenge 32K + RAM 2K [0xC0 + 0xD0 segments are not needed]

void readSegment_INTV(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 256) {
    for (uint16_t w = 0; w < 256; w++) {
      uint16_t temp = readData_INTV(addr + w);
      sdBuffer[w * 2] = (temp >> 8) & 0xFF;
      sdBuffer[(w * 2) + 1] = temp & 0xFF;
    }
    myFile.write(sdBuffer, 512);
  }
}

// MODIFIED READ ROUTINE FOR ALL 10 MAPPERS
void readROM_INTV() {
  createFolderAndOpenFile("INTV", "ROM", romName, "int");

  switch (intvmapper) {
    case 0:                              //default mattel up to 32K (8K/12K/16K/24K/32K)
      readSegment_INTV(0x5000, 0x6000);  // 8K
      if (intvsize > 0) {
        readSegment_INTV(0x6000, 0x6800);  // +4K = 12K
        if (intvsize > 1) {
          readSegment_INTV(0x6800, 0x7000);  // +4K = 16K
          if (intvsize > 2) {
            readSegment_INTV(0xD000, 0xE000);  // +8K = 24K
            if (intvsize > 3)
              readSegment_INTV(0xF000, 0x10000);  // +8K = 32K
          }
        }
      }
      break;

    case 1:                              // demo cart/championship tennis/wsml baseball
      readSegment_INTV(0x5000, 0x7000);  // 16K Demo Cart
      if (intvsize > 2) {
        readSegment_INTV(0xD000, 0xE000);  // +8K = 24K [NONE]
        if (intvsize > 3) {
          readSegment_INTV(0xE000, 0xF000);  // +8K = 32K Championship Tennis
          if (intvsize > 4) {
            readSegment_INTV(0xF000, 0x10000);  // +8K = 40K WSML Baseball [MISSING 8K ECS BANK]
            // ecs bank switch
            ecsBank(0xFFFF, 0x1);               // switch ecs page 1 to 0xF000
            readSegment_INTV(0xF000, 0x10000);  // + 8K = 48K WSML Baseball
            ecsBank(0xFFFF, 0x0);               // reset ecs page 0 to 0xF000
          }
        }
      }
      break;

    case 2:                              // up to 48K (16K/32K/48K)
      readSegment_INTV(0x5000, 0x7000);  // 16K
      if (intvsize > 2) {
        readSegment_INTV(0x9000, 0xA000);  // +8K = 24K [NONE]
        if (intvsize > 3) {
          readSegment_INTV(0xA000, 0xB000);  // +8K = 32K
          if (intvsize > 4) {
            readSegment_INTV(0xB000, 0xC000);  // +8K = 40K
            readSegment_INTV(0xD000, 0xE000);  // +8K = 48K
          }
        }
      }
      break;

    case 3:                               // tower of doom 48K
      readSegment_INTV(0x5000, 0x7000);   // 16K
      readSegment_INTV(0x9000, 0xB000);   // +16K = 32K
      readSegment_INTV(0xD000, 0xE000);   // +8K = 40K
      readSegment_INTV(0xF000, 0x10000);  // +8K = 48K
      break;

    case 4:                              // chess 16K
      PORTH &= ~(1 << 3);                // /MSYNC to LOW
      readSegment_INTV(0x5000, 0x6000);  // 8K
      PORTH |= (1 << 3);                 // /MSYNC to HIGH
      readSegment_INTV(0x6000, 0x7000);  // 8K
      break;

    case 5:                              // congo bongo/defender/pac-man/dig dug
      readSegment_INTV(0x5000, 0x7000);  // 16K
      readSegment_INTV(0x7000, 0x8000);  // +8K = 24K Congo Bongo/Defender/Pac-Man
      if (intvsize > 3) {
        readSegment_INTV(0x9000, 0xA000);  // +8K = 32K Dig Dug
        //readSegment_INTV(0xA000,0xC000); // +16K = 48K [UNUSED]
      }
      break;

    case 6:                              // centipede 16K
      readSegment_INTV(0x6000, 0x8000);  // 16K
      break;

    case 7:                              // imagic carts 16K
      readSegment_INTV(0x4800, 0x6800);  // 16K
      break;

    case 8:                              //mte-201 test cart 16K
      readSegment_INTV(0x5000, 0x6000);  // 8K
      readSegment_INTV(0x7000, 0x8000);  // +8K = 16K
      break;

    case 9:                              // triple challenge 32K [KNOWN ROM 44K BAD!]
      readSegment_INTV(0x5000, 0x7000);  // 16K
      readSegment_INTV(0x9000, 0xB000);  // +16K = 32K
      // 0xC000 + 0xD000 SEGMENTS ARE NOT NEEDED (PER INTVNUT POST)
      //      readSegment_INTV(0xC000,0xC800); // +4K = 36K
      //      readSegment_INTV(0xD000,0xE000); // +8K = 44K
      break;
  }
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  compareCRC("intv.txt", 0, 1, 0);

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

// ECS BANKSWITCH FOR WSML BASEBALL
// write $xA5y to $xFFF
// x = rom location ($x000 - $xFFF)
// y = page (up to 16 - WSML Baseball only uses 0/1)
void ecsBank(uint32_t addr, uint8_t bank) {
  uint16_t ecsdata = (addr & 0xF000) + 0x0A50 + bank;  // $xA5y

  // Data OUT
  DDRA = 0xFF;
  DDRC = 0xFF;

  // Set Address
  PORTA = (addr >> 8) & 0xFF;
  PORTC = addr & 0xFF;

  BAR_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NACT_INT();
  NOP;

  // Data OUT
  DDRA = 0xFF;
  DDRC = 0xFF;
  PORTA = (ecsdata >> 8) & 0xFF;  // $xA
  PORTC = ecsdata & 0xFF;         // $5y

  DW_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  DWS_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NACT_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
}

//******************************************
// MAPPER CODE
//******************************************
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printMapperSelection_INTV(int index) {
  display_Clear();
  print_Msg(FS(FSTRING_MAPPER));
  intvindex = index * 4;
  intvmapselect = pgm_read_byte(intvmapsize + intvindex);
  println_Msg(intvmapselect);
}
#endif

void setMapper_INTV() {
  byte newintvmapper;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  navigateMenu(0, intvmapcount - 1, &printMapperSelection_INTV);
  newintvmapper = intvmapselect;
  
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(newintvmapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  intvmapfound = false;
  Serial.print(F("Enter Mapper [0-9]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newintvmapper = newmap.toInt();
  for (int i = 0; i < intvmapcount; i++) {
    intvindex = i * 4;
    intvmapselect = pgm_read_byte(intvmapsize + intvindex);
    if (newintvmapper == intvmapselect)
      intvmapfound = true;
  }
  if (intvmapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(FS(FSTRING_EMPTY));
    newintvmapper = 0;
    goto setmapper;
  }
#endif
  EEPROM_writeAnything(7, newintvmapper);
  intvmapper = newintvmapper;
}

void checkMapperSize_INTV() {
  for (int i = 0; i < intvmapcount; i++) {
    intvindex = i * 4;
    byte mapcheck = pgm_read_byte(intvmapsize + intvindex);
    if (mapcheck == intvmapper) {
      intvlo = pgm_read_byte(intvmapsize + intvindex + 1);
      intvhi = pgm_read_byte(intvmapsize + intvindex + 2);
      break;
    }
  }
}

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_INTV(int index) {
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(pgm_read_byte(&(INTV[index])));
}
#endif

void setROMSize_INTV() {
  byte newintvsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (intvlo == intvhi)
    newintvsize = intvlo;
  else {
    newintvsize = navigateMenu(intvlo, intvhi, &printRomSize_INTV);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(pgm_read_byte(&(INTV[newintvsize])));
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (intvlo == intvhi)
    newintvsize = intvlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (intvhi - intvlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(pgm_read_byte(&(INTV[i + intvlo])));
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newintvsize = sizeROM.toInt() + intvlo;
    if (newintvsize > intvhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(pgm_read_byte(&(INTV[newintvsize])));
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newintvsize);
  intvsize = newintvsize;
}

void checkStatus_INTV() {
  EEPROM_readAnything(7, intvmapper);
  EEPROM_readAnything(8, intvsize);
  if (intvmapper > 9) {
    intvmapper = 0;
    EEPROM_writeAnything(7, intvmapper);
  }
  if (intvsize > 5) {
    intvsize = 0;
    EEPROM_writeAnything(8, intvsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("INTELLIVISION READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Msg(intvmapper);
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(pgm_read_byte(&(INTV[intvsize])));
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:   "));
  Serial.println(intvmapper);
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(pgm_read_byte(&(INTV[intvsize])));
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
struct database_entry_INTV {
  char crc_search[9];
  byte gameMapper;
  byte gameSize;
};

void readDataLine_INTV(FsFile& database, void* entry) {
  struct database_entry_INTV* castEntry = (database_entry_INTV*)entry;
  // Read CRC32 checksum
  for (byte i = 0; i < 8; i++) {
    checksumStr[i] = char(database.read());
  }

  // Skip over semicolon
  database.seekCur(1);

  // Read CRC32 of first 512 bytes
  for (byte i = 0; i < 8; i++) {
    castEntry->crc_search[i] = char(database.read());
  }

  // Skip over semicolon
  database.seekCur(1);

  // Read mapper
  castEntry->gameMapper = database.read() - 48;

  // Skip over semicolon
  database.seekCur(1);

  // Read rom size
  // Read the next ascii character and subtract 48 to convert to decimal
  castEntry->gameSize = ((database.read() - 48) * 10) + (database.read() - 48);

  // Skip over semicolon
  database.seekCur(1);

  // Read SRAM size
  byte sramSize __attribute__((unused)) = database.read() - 48;

  // Skip rest of line
  database.seekCur(2);
}

void printDataLine_INTV(void* entry) {
  struct database_entry_INTV* castEntry = (database_entry_INTV*)entry;
  print_Msg(FS(FSTRING_SIZE));
  print_Msg(castEntry->gameSize);
  println_Msg(F("KB"));
  print_Msg(FS(FSTRING_MAPPER));
  println_Msg(castEntry->gameMapper);
}

void setCart_INTV() {
  //go to root
  sd.chdir();

  struct database_entry_INTV entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("intv.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLine_INTV, &entry, &printDataLine_INTV)) {
      //byte INTV[] = {8, 12, 16, 24, 32, 48};
      switch (entry.gameSize) {
        case 8:
          intvsize = 0;
          break;

        case 12:
          intvsize = 1;
          break;

        case 16:
          intvsize = 2;
          break;

        case 24:
          intvsize = 3;
          break;

        case 32:
          intvsize = 4;
          break;

        case 48:
          intvsize = 5;
          break;

        default:
          intvsize = 0;
          break;
      }
      EEPROM_writeAnything(7, entry.gameMapper);
      EEPROM_writeAnything(8, intvsize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
//******************************************
// End of File
//******************************************