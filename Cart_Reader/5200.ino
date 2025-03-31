//******************************************
// ATARI 5200 MODULE
//******************************************
#ifdef ENABLE_5200
// Atari 5200
// Cartridge Pinout
// 36P 2.54mm pitch connector
//
//                        RIGHT
//                      +-------+
//           INTERLOCK -| 18 19 |- A0
//                  A2 -| 17 20 |- A1
//    L             A5 -| 16 21 |- A3
//    A             A6 -| 15 22 |- A4         B
//    B            GND -| 14 23 |- GND        O
//    E            GND -| 13 24 |- GND        T
//    L            GND -| 12 25 |- GND        T
//                 N/C -| 11 26 |- +5V        O
//   /ENABLE 4000-7FFF -| 10 27 |- A7         M
//   /ENABLE 8000-BFFF -|  9 28 |- N/C
//                  D7 -|  8 29 |- A8         S
//    S             D6 -|  7 30 |- AUD        I
//    I             D5 -|  6 31 |- A9         D
//    D             D4 -|  5 32 |- A13        E
//    E             D3 -|  4 33 |- A10
//                  D2 -|  3 34 |- A12
//                  D1 -|  2 35 |- A11
//                  D0 -|  1 36 |- INTERLOCK
//                      +-------+
//                        LEFT
//
//                                        LABEL SIDE
//
//                                         /EN /EN
//          D0  D1  D2  D3  D4  D5  D6  D7  80  40  -- GND GND GND  A6  A5  A2  INT
//       +--------------------------------------------------------------------------+
//       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  |
// LEFT  |                                                                          | RIGHT
//       |  36  35  34  33  32  31  30  29  28  27  26  25  24  23  22  21  20  19  |
//       +--------------------------------------------------------------------------+
//         INT A11 A12 A10 A13  A9 AUD  A8  --  A7 +5V GND GND GND  A4  A3  A1  A0
//
//                                        BOTTOM SIDE

// CONTROL PINS:
// /4000(PH5) - SNES /WR
// /8000(PH6) - SNES /RD

//******************************************
//  Defines
//******************************************
#define DISABLE_4000 PORTH |= (1 << 5)  // ROM SELECT 4000-7FFF
#define ENABLE_4000 PORTH &= ~(1 << 5)
#define DISABLE_8000 PORTH |= (1 << 6)  // ROM SELECT 8000-BFFF
#define ENABLE_8000 PORTH &= ~(1 << 6)

struct a5200_DB_entry {
  char crc32[9];
  byte gameMapper;
  byte gameSize;
};

//******************************************
//  Supported Mappers
//******************************************
// Supported Mapper Array
// Format = {mapper,romsizelo,romsizehi}
static const byte PROGMEM a5200mapsize[] = {
  0, 0, 3,  // Standard 4K/8K/16K/32K
  1, 2, 2,  // Two Chip 16K
  2, 4, 4,  // Bounty Bob Strikes Back 40K [UNTESTED]
};

byte a5200mapcount = 3;  // (sizeof(a5200mapsize) / sizeof(a5200mapsize[0])) / 3;
byte a5200mapselect;
int a5200index;

byte a5200[] = { 4, 8, 16, 32, 40 };
byte a5200lo = 0;  // Lowest Entry
byte a5200hi = 4;  // Highest Entry
byte a5200mapper = 0;
byte a5200size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptions5200[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void setup_5200() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Atari 5200 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)  /4000(PH5) /8000(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)  /4000(PH5) /8000(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_5200();
  strcpy(romName, "ATARI");

  mode = CORE_5200;
}

void a5200Menu() {
  convertPgm(menuOptions5200, 4);
  uint8_t mainMenu = question_box(F("ATARI 5200 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_5200();
      setup_5200();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_5200();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper + Size
      setMapper_5200();
      checkMapperSize_5200();
      setROMSize_5200();
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// READ CODE
//******************************************

uint8_t readData_5200(uint16_t addr)  // Add Input Pullup
{
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13
  cycleDelay(5);

  // DDRC = 0x00; // Set to Input
  PORTC = 0xFF;  // Input Pullup
  cycleDelay(15); // Standard + extended delay for Vanguard

  uint8_t ret = PINC;
  cycleDelay(5);

  return ret;
}

void readSegment_5200(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_5200(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readBankBountyBob_5200(uint16_t startaddr) {
  for (int w = 0; w < 4; w++) {
    readData_5200(startaddr + 0xFF6 + w);
    readSegment_5200(startaddr, startaddr + 0xE00);
    // Split Read of Last 0x200 bytes
    for (int x = 0; x < 0x1F6; x++) {
      sdBuffer[x] = readData_5200(startaddr + 0xE00 + x);
    }
    myFile.write(sdBuffer, 502);
    // Bank Registers 0xFF6-0xFF9
    for (int y = 0; y < 4; y++) {
      readData_5200(startaddr + 0xFFF);  // Reset Bank
      sdBuffer[y] = readData_5200(startaddr + 0xFF6 + y);
    }
    // End of Bank 0xFFA-0xFFF
    readData_5200(startaddr + 0xFFF);      // Reset Bank
    readData_5200(startaddr + 0xFF6 + w);  // Set Bank
    for (int z = 4; z < 10; z++) {
      sdBuffer[z] = readData_5200(startaddr + 0xFF6 + z);  // 0xFFA-0xFFF
    }
    myFile.write(sdBuffer, 10);
  }
  readData_5200(startaddr + 0xFFF);  // Reset Bank
}

//******************************************
// READ ROM
//******************************************

void readROM_5200() {
  createFolderAndOpenFile("5200", "ROM", romName, "a52");

  // 5200 A13-A0 = 10 0000 0000 0000
  switch (a5200mapper) {
    case 0:  // Standard 4KB/8KB/16KB/32KB
      // Lower Half of 32K is at 0x4000
      if (a5200size == 3) {  // 32K
        ENABLE_4000;
        cycleDelay(15);
        readSegment_5200(0x4000, 0x8000);  // +16K = 32K
        DISABLE_4000;
        cycleDelay(15);
      }
      // 4K/8K/16K + Upper Half of 32K
      ENABLE_8000;
      cycleDelay(15);
      if (a5200size > 1)
        readSegment_5200(0x8000, 0xA000);  // +8K = 16K
      if (a5200size > 0)
        readSegment_5200(0xA000, 0xB000);  // +4K = 8K
      // Base 4K
      readSegment_5200(0xB000, 0xC000);  // 4K
      DISABLE_8000;
      cycleDelay(15);
      break;

    case 1:  // Two Chip 16KB
      ENABLE_4000;
      cycleDelay(15);
      readSegment_5200(0x4000, 0x6000);  // 8K
      DISABLE_4000;
      cycleDelay(15);
      ENABLE_8000;
      cycleDelay(15);
      readSegment_5200(0x8000, 0xA000);  // +8K = 16K
      DISABLE_8000;
      cycleDelay(15);
      break;

    case 2:  // Bounty Bob Strikes Back 40KB [UNTESTED]
      ENABLE_4000;
      cycleDelay(15);
      // First 16KB (4KB x 4)
      readBankBountyBob_5200(0x4000);
      // Second 16KB (4KB x 4)
      readBankBountyBob_5200(0x5000);
      DISABLE_4000;
      cycleDelay(15);
      ENABLE_8000;
      cycleDelay(15);
      readSegment_5200(0x8000, 0xA000);  // +8K = 40K
      DISABLE_8000;
      cycleDelay(15);
      break;
  }
  myFile.close();

  compareCRC("5200.txt", 0, 1, 0);

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void println_Mapper5200(byte mapper) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  if (mapper == 0)
    println_Msg(F("STANDARD"));
  else if (mapper == 1)
    println_Msg(F("TWO CHIP"));
  else if (mapper == 2)
    println_Msg(F("BOUNTY BOB"));
#else
  if (mapper == 0)
    Serial.println(F("STANDARD"));
  else if (mapper == 1)
    Serial.println(F("TWO CHIP"));
  else if (mapper == 2)
    Serial.println(F("BOUNTY BOB"));
#endif
}

void checkMapperSize_5200() {
  for (int i = 0; i < a5200mapcount; i++) {
    a5200index = i * 3;
    byte mapcheck = pgm_read_byte(a5200mapsize + a5200index);
    if (mapcheck == a5200mapper) {
      a5200lo = pgm_read_byte(a5200mapsize + a5200index + 1);
      a5200hi = pgm_read_byte(a5200mapsize + a5200index + 2);
      break;
    }
  }
}

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_5200(int index) {
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(a5200[index]);
}
#endif

void setROMSize_5200() {
  byte new5200size;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (a5200lo == a5200hi)
    new5200size = a5200lo;
  else {
    new5200size = navigateMenu(a5200lo, a5200hi, &printRomSize_5200);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(a5200[new5200size]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (a5200lo == a5200hi)
    new5200size = a5200lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (a5200hi - a5200lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(a5200[i + a5200lo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    new5200size = sizeROM.toInt() + a5200lo;
    if (new5200size > a5200hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(a5200[new5200size]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, new5200size);
  a5200size = new5200size;
}

void checkStatus_5200() {
  EEPROM_readAnything(7, a5200mapper);
  EEPROM_readAnything(8, a5200size);
  if (a5200mapper > 2) {
    a5200mapper = 0;  // default STANDARD
    EEPROM_writeAnything(7, a5200mapper);
  }
  if (a5200size > 4) {
    a5200size = 1;  // default 8K
    EEPROM_writeAnything(8, a5200size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("ATARI 5200 READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Msg(a5200mapper);
  println_Mapper5200(a5200mapper);
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(a5200[a5200size]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("MAPPER:   "));
  Serial.println(a5200mapper);
  println_Mapper5200(a5200mapper);
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(a5200[a5200size]);
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// READ MAPPER
//******************************************

void readDbEntry(FsFile& database, void* entry) {
  struct a5200_DB_entry* castEntry = (a5200_DB_entry*)entry;

  // Read expected CRC32 as a string
  for (int i = 0; i < 8; ++i) {
    castEntry->crc32[i] = database.read();
  }
  castEntry->crc32[8] = '\0';
  database.seekCur(1); // Skip comma delimiter

  // Read mapper
  castEntry->gameMapper = database.read() - 48;

  // if next char is not a comma, expect an additional digit
  char temp = database.read();
  if (temp != ',') {
    castEntry->gameMapper = (castEntry->gameMapper * 10) + (temp - 48);
    database.seekCur(1); // Skip over comma
  }

  // Read rom size
  castEntry->gameSize = database.read() - 48;
  database.seekCur(2); // Skip rest of line
}

//******************************************
// SET MAPPER
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printMapperSelection_5200(int index) {
  display_Clear();
  print_Msg(FS(FSTRING_MAPPER));
  a5200index = index * 3;
  a5200mapselect = pgm_read_byte(a5200mapsize + a5200index);
  println_Msg(a5200mapselect);
  println_Mapper5200(a5200mapselect);
}
#endif

void setMapper_5200() {
  byte new5200mapper;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  navigateMenu(0, a5200mapcount - 1, &printMapperSelection_5200);
  new5200mapper = a5200mapselect;

  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(new5200mapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  Serial.println(F("SUPPORTED MAPPERS:"));
  Serial.println(F("0 = Standard 4K/8K/16K/32K"));
  Serial.println(F("1 = Two Chip 16K"));
  Serial.println(F("2 = Bounty Bob Strikes Back 40K"));
  Serial.print(F("Enter Mapper [0-2]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  a5200index = newmap.toInt() * 3;
  new5200mapper = pgm_read_byte(a5200mapsize + a5200index);
#endif
  EEPROM_writeAnything(7, new5200mapper);
  a5200mapper = new5200mapper;
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_5200() {
  //go to root
  sd.chdir();

  struct a5200_DB_entry entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("5200.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDbEntry, &entry)) {
      EEPROM_writeAnything(7, entry.gameMapper);
      EEPROM_writeAnything(8, entry.gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}

// While not precise in terms of exact cycles for NOP due to the for-loop
// overhead, it simplifies the code while still achieving a similar result. 
void cycleDelay(byte cycleCount) {
  for (byte i = 0; i < cycleCount; ++i) {
    NOP;
  }
}
#endif
