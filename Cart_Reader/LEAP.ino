//******************************************
// LEAP MODULE
//******************************************
#ifdef ENABLE_LEAP
// Leapster
// Cartridge pinout
// 60P 1.27mm pitch connector
//
//     FRONT               BACK
//      SIDE               SIDE
//            +---------+
//       VCC -| 1B   1A |- VCC
//        nc -| 2B   2A |- GND
//       D11 -| 3B   3A |- D4
//        D3 -| 4B   4A |- D12
//       D10 -| 5B   5A |- D5
//       GND -| 6B   6A |- D2
//       D13 -| 7B   7A |- D9
//        nc -| 8B   8A |- nc 
//        D6 -| 9B   9A |- D1
//       D14 -| 10B 10A |- D8
//            |---------|
//        D7 -| 11B 11A |- GND
//        D0 -| 12B 12A |- D15
//      BYTE -| 13B 13A |- OE
//        nc -| 14B 14A |- A22
//    FLA_CE -| 15B 15A |- A23
//        CE -| 16B 16A |- A16
//        A0 -| 17B 17A |- A15
//        A1 -| 18B 18A |- A14
//        A2 -| 19B 19A |- A13
//       GND -| 20B 20A |- A3
//       A12 -| 21B 21A |- A4
//       A11 -| 22B 22A |- A5
//       A10 -| 23B 23A |- A6
//        A9 -| 24B 24A |- A7
//        A8 -| 25B 25A |- A17
//       A19 -| 26B 26A |- A18
//       A20 -| 27B 27A |- A21
//        WE -| 28B 28A |- GND
//   EEP_SDA -| 29B 29A |- EEP_WP
//        nc -| 30B 30A |- EEP_SCL
//            +---------+
//
// CONTROL PINS:
// EEPROM WP (PH0) - SNES /RESET
// EEPROM SCL (PH1) - SNES CPUCLK
// EEPROM SDA (PH4) - SNES /IRQ
// OE (PH3) - SNES /CART
// WE (PH5) - SNES /WR
// CE (PH6) - SNES /RD
// BYTE (PE5) - SNES REFRESH
// FLASH CE (PJ0) - SNES /PARD

//******************************************
// DEFINES
//******************************************
// CONTROL PINS - OE/WE/CE
#define OE_HIGH PORTH |= (1<<3)
#define OE_LOW PORTH &= ~(1<<3) 
#define WE_HIGH PORTH |= (1<<5)
#define WE_LOW PORTH &= ~(1<<5)
#define CE_HIGH PORTH |= (1<<6)
#define CE_LOW PORTH &= ~(1<<6)

// BYTE PIN IS A0 IN BYTE MODE - SHIFT ADDRESS >> 1
#define BYTE_HIGH PORTE |= (1<<5)
#define BYTE_LOW PORTE &= ~(1<<5)

// SERIAL I2C EEPROM PINS 24LC02B 2K
// PIN 5 - SDA
// PIN 6 - SCL
// PIN 7 - WP
// CART B30 = SDA
// CART A31 = SCL
// CART A30 = WP
#define WP_HIGH PORTH |= (1<<0)
#define WP_LOW PORTH &= ~(1<<0)
#define SCL_HIGH PORTH |= (1<<1)
#define SCL_LOW PORTH &= ~(1<<1)
#define SDA_HIGH PORTH |= (1<<4)
#define SDA_LOW PORTH &= ~(1<<4)

// FLASH 39VF040 512K
// CART B16 = CE#
#define FL_CE_HIGH PORTJ |= (1<<0)
#define FL_CE_LOW PORTJ &= ~(1<<0)

#define DATA_C_READ { DDRC = 0; PORTC = 0xFF; } // [INPUT PULLUP]
#define DATA_A_READ { DDRA = 0; PORTA = 0xFF; } // [INPUT PULLUP]

//******************************************
// VARIABLES
//******************************************
// ROM File = LEAP.bin
// RAM File = SAVE.eep
// FLASH File = SAVE.fla

byte LEAPSTER[] = {4,8,16};
byte leaplo = 0; // Lowest Entry
byte leaphi = 2; // Highest Entry
byte leapsize;
byte newleapsize;

byte tempbyte;
char tempword;
word ptrword;
word tempcheck;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// ROM STRUCTURE
//******************************************
char LEAP[] = {0x4C,0x45,0x41,0x50}; // "LEAP" MARKER AT 0x00 OR 0x144 (WORD 0xA2)
char TBL[] = {0x01,0x00,0x00,0x01};  // TABLE START
char TXT[] = {0x04,0x00,0x00,0x01};  // NEXT DWORD IS TEXT BLOCK [5 SENTENCES] START
char VER[] = {0x0A,0x00,0x00,0x01};  // NEXT DWORD IS ROM VERSION LOCATION
char TTL[] = {0x0B,0x00,0x00,0x01};  // NEXT DWORD IS ROM TITLE LOCATION
char END[] = {0x10,0x00,0x00,0x01}; // LAST BLOCK - END SEARCH
word sentenceAddr = 0; // Sentence Block Start Address
word versionAddr = 0;  // Version Address
word titleAddr = 0;    // Title Address
char ROMVersion[20];   // Fosters [20] "155-11172 152-11808"
char ROMTitle[50];     // "Mr. Pencils Learn to Draw and Write." [37]
                       // "Thomas and Friends Calling All Engines" [39]
                       // "The Letter Factory.v1.0 - Initial Release JBM3" [47]

//******************************************
// DATA INTEGRITY BLOCK
//******************************************
// 5 Sentences - 172 bytes 
// Location not static between ROMs

static const unsigned char LeapCheck [] = {
0x4C,0x69,0x6C,0x20,0x64,0x75,0x63,0x6B,0x65,0x64,0x2E,0x20,0x20,0x54,0x68,0x65,
0x20,0x6A,0x65,0x74,0x20,0x7A,0x69,0x70,0x70,0x65,0x64,0x20,0x70,0x61,0x73,0x74,
0x20,0x68,0x65,0x72,0x20,0x68,0x65,0x61,0x64,0x2E,0x20,0x20,0x44,0x75,0x73,0x74,
0x20,0x66,0x6C,0x65,0x77,0x2C,0x20,0x4C,0x69,0x6C,0x20,0x73,0x6E,0x65,0x65,0x7A,
0x65,0x64,0x2C,0x20,0x61,0x6E,0x64,0x20,0x4C,0x65,0x61,0x70,0x20,0x74,0x75,0x72,
0x6E,0x65,0x64,0x20,0x72,0x65,0x64,0x2E,0x20,0x20,0x54,0x68,0x65,0x6E,0x20,0x4C,
0x69,0x6C,0x20,0x67,0x6F,0x74,0x20,0x75,0x70,0x2C,0x20,0x61,0x62,0x6F,0x75,0x74,
0x20,0x74,0x6F,0x20,0x79,0x65,0x6C,0x6C,0x2E,0x20,0x20,0x4C,0x65,0x61,0x70,0x20,
0x67,0x61,0x73,0x70,0x65,0x64,0x2C,0x20,0x22,0x4C,0x6F,0x6F,0x6B,0x2C,0x20,0x4C,
0x69,0x6C,0x21,0x20,0x20,0x59,0x6F,0x75,0x72,0x20,0x74,0x6F,0x6F,0x74,0x68,0x21,
0x20,0x20,0x49,0x74,0x20,0x66,0x65,0x6C,0x6C,0x21,0x22,0x00 
};

//******************************************
// MENU
//******************************************
// Base Menu
static const char leapmenuItem4[] PROGMEM = "Read EEPROM";
static const char leapmenuItem5[] PROGMEM = "Write EEPROM";
static const char leapmenuItem6[] PROGMEM = "Read FLASH";
static const char leapmenuItem7[] PROGMEM = "Write FLASH";
static const char* const menuOptionsLEAP[] PROGMEM = {  FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, leapmenuItem4, leapmenuItem5, leapmenuItem6, leapmenuItem7 };

void leapMenu()
{
  convertPgm(menuOptionsLEAP, 7);
  uint8_t mainMenu = question_box(F("LEAPSTER MENU"), menuOptions, 7, 0);

  switch (mainMenu)
  {
    // Select Cart
    case 0:
      setCart_LEAP();
      setup_LEAP();
      break;

    // Read ROM
    case 1:
      sd.chdir("/");
      readROM_LEAP();
      sd.chdir("/");
      break;

    // Set Size
    case 2:
      setROMSize_LEAP();
      break;

    // Read EEPROM
    case 3:
      sd.chdir("/");
      readEEP_LEAP();
      sd.chdir("/");
      break;

    // Write EEPROM
    case 4:
      writeEEP_LEAP();
      break;

    // Read FLASH
    case 5:
      idFLASH_LEAP();
      if (strcmp(flashid_str, "BFD7") == 0) {
        sd.chdir("/");
        readFLASH_LEAP();
        sd.chdir("/");
      }
      break;

    // Write FLASH
    case 6:
      idFLASH_LEAP();
      if (strcmp(flashid_str, "BFD7") == 0)
        writeFLASH_LEAP();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_LEAP()
{
  // Request 3.3V
  setVoltage(VOLTS_SET_3V3);

  // Control Pins PH0..PH1, PH3..PH6  BYTE, OE, CE, WE
  DDRH = 0x7B;  // 0b01111011 - CE, WE, SDA, OE, SCL, WP [OUTPUT] - Unused Pins [INPUT]
  PORTH = 0xFF; // 0b11111111 - CE, WE, SDA, OE, SCL, WP [HIGH] - Unused Pins [HIGH]

  // Address Pins
  DDRF = 0xFF;  // Address A0-A7 [OUTPUT]
  DDRK = 0xFF;  // Address A8-A15 [OUTPUT]
  DDRL = 0xFF;  // Address A16-A23 [OUTPUT]
  // Data Pins
  DDRC = 0x00;  // D0-D7 [INPUT]
  DDRA = 0x00;  // D8-D15 [INPUT]
  // BYTE Pin PE5
  DDRE = 0x20;  // 0b00100000 BYTE [OUTPUT] - Unused Pins [INPUT]
  PORTE = 0xFF; // 0b11111111 BYTE [HIGH] - Unused Pins [HIGH]
  // Flash Pin PJ0
  DDRJ = 0x01;  // 0b00000001 - FLA_CE [OUTPUT] - Unused Pins [INPUT]
  PORTJ = 0xFF; // 0b11111111 - FLA_CE [HIGH] - Unused Pins {HIGH]

  // Check Start for "LEAP" marker
  // Read ROM Version & Title
  checkStart_LEAP();

  // 39VF040 Flash Check
//  idFLASH();

  checkStatus_LEAP();
  strncpy(romName, ROMTitle, 16);  // Truncate ROMTitle to fit

  mode = CORE_LEAP;
}

//******************************************
// READ FUNCTIONS
//******************************************
// Max ROM Size 0x1000000 (Highest Address = 0xFFFFFF) - FF FFFF
word read_rom_word_LEAP(unsigned long address) // OLD TIMING #1
{
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  PORTF = address & 0xFF;
  CE_LOW;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  OE_LOW;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  unsigned char data1 = PINC;
  unsigned char data2 = PINA;
  word data = (data1 << 8) | (data2);
  OE_HIGH;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  CE_HIGH;

  return data;
}

byte read_flash_byte_LEAP(unsigned long address) // CE HIGH, OE LOW, FL_CE LOW
{
  PORTL = (address >> 17) & 0x7F;
  PORTK = (address >> 9) & 0xFF;
  PORTF = (address >> 1) & 0xFF;
  if (address & 0x1) // BYTE = A0
    BYTE_HIGH;
  else
    BYTE_LOW;
  CE_HIGH;
  OE_LOW;
  FL_CE_LOW;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  unsigned char data = PINC;
  FL_CE_HIGH;
  OE_HIGH;

  return data;
}

//******************************************
// WRITE FUNCTION
//******************************************

void write_flash_byte_LEAP(unsigned long address, unsigned char data)
{
  PORTL = (address >> 17) & 0x7F;
  PORTK = (address >> 9) & 0xFF;
  PORTF = (address >> 1) & 0xFF;
  if (address & 0x1) // BYTE = A0
    BYTE_HIGH;
  else
    BYTE_LOW;
  OE_HIGH;
  FL_CE_LOW;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  PORTC = data;
  WE_LOW;
  WE_HIGH;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  FL_CE_HIGH;
  OE_LOW;
}

//******************************************
// DATA INTEGRITY FUNCTIONS
//******************************************

void checkStart_LEAP()
{
  delay(500);
  OE_LOW;
  CE_LOW;
  BYTE_HIGH;
  tempword = read_rom_word_LEAP(0x0);
  if ((LEAP[0] == ((tempword >> 0x8) & 0xFF)) && (LEAP[1] == (tempword & 0xFF))) {
    tempword = read_rom_word_LEAP(0x1);
    findTable_LEAP(0x4, 0xA2);
  }
  else {
    delay(500);
    tempword = read_rom_word_LEAP(0xA2);
    if ((LEAP[0] == ((tempword >> 0x8) & 0xFF)) && (LEAP[1] == (tempword & 0xFF))) {
      tempword = read_rom_word_LEAP(0xA3);
      findTable_LEAP(0xA4, 0x146);
    }
  }
  CE_HIGH;
  OE_HIGH;
}

void findTable_LEAP(unsigned long startAddr, unsigned long endAddr)
{
  delay(500);
  CE_LOW;
  for (unsigned long addr = startAddr; addr < endAddr; addr +=2) {
    tempword = read_rom_word_LEAP(addr);
    if ((TBL[0] == ((tempword >> 0x8) & 0xFF))&&(TBL[1] == (tempword & 0xFF))) {
      tempword = read_rom_word_LEAP(addr + 1);
      if ((TBL[2] == ((tempword >> 0x8) & 0xFF))&&(TBL[3] == (tempword & 0xFF))) {
        readTable_LEAP(startAddr, endAddr);
        break;
      }
    }
  }
  CE_HIGH;
}

void readTable_LEAP(unsigned long startAddr, unsigned long endAddr)
{
  delay(500);
  CE_LOW;
  for (unsigned long addr = startAddr; addr < endAddr; addr++) {
    tempword = read_rom_word_LEAP(addr);
    if ((TXT[0] == ((tempword >> 0x8) & 0xFF))&&(TXT[1] == (tempword & 0xFF))) {
      tempword = read_rom_word_LEAP(addr + 1);
      if ((TXT[2] == ((tempword >> 0x8) & 0xFF))&&(TXT[3] == (tempword & 0xFF))) { // Text Block Marker Found
        ptrword = read_rom_word_LEAP(addr + 2);
        sentenceAddr = (((ptrword >> 8) & 0xFF) | ((ptrword & 0xFF) << 8)); // Swap Byte Order
      }
    }
    else if ((VER[0] == ((tempword >> 0x8) & 0xFF))&&(VER[1] == (tempword & 0xFF))) {
      tempword = read_rom_word_LEAP(addr + 1);
      if ((VER[2] == ((tempword >> 0x8) & 0xFF))&&(VER[3] == (tempword & 0xFF))) { // Version Marker Found
        ptrword = read_rom_word_LEAP(addr + 2);
        versionAddr = (((ptrword >> 8) & 0xFF) | ((ptrword & 0xFF) << 8)); // Swap Byte Order
      }
    }
    else if ((TTL[0] == ((tempword >> 0x8) & 0xFF))&&(TTL[1] == (tempword & 0xFF))) {
      tempword = read_rom_word_LEAP(addr + 1);      
      if ((TTL[2] == ((tempword >> 0x8) & 0xFF))&&(TTL[3] == (tempword & 0xFF))) { // Title Marker Found
        ptrword = read_rom_word_LEAP(addr + 2);
        titleAddr = (((ptrword >> 8) & 0xFF) | ((ptrword & 0xFF) << 8)); // Swap Byte Order
      }
    }
    else if ((END[0] == ((tempword >> 0x8) & 0xFF))&&(END[1] == (tempword & 0xFF))) {
      tempword = read_rom_word_LEAP(addr + 1);      
      if ((END[2] == ((tempword >> 0x8) & 0xFF))&&(END[3] == (tempword & 0xFF))) { // END OF TABLE
        break;
      }
    }
  }
  CE_HIGH;
//  print_Msg(F("Text Addr: "));
//  println_Msg(sentenceAddr, HEX);
//  print_Msg(F("Version Addr: "));
//  println_Msg(versionAddr, HEX);
//  print_Msg(F("Title Addr: "));
//  println_Msg(titleAddr, HEX);
//  display_Update();

  delay(500);
  CE_LOW;
  for (int x = 0; x < 10; x++) {
    tempword = read_rom_word_LEAP((versionAddr / 2) + x);
    ROMVersion[x * 2] = (tempword >> 0x8) & 0xFF;
    ROMVersion[(x * 2) + 1] = tempword & 0xFF;
  }

  delay(500);
  CE_LOW;
  for (int x = 0; x < 25; x++) {
    tempword = read_rom_word_LEAP((titleAddr / 2) + x);
    ROMTitle[x * 2] = (tempword >> 0x8) & 0xFF;
    ROMTitle[(x * 2) + 1] = tempword & 0xFF;
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_LEAP()
{
  createFolderAndOpenFile("LEAP", "ROM", romName, "bin");

  for (unsigned long address = 0; address < 0x200000; address += 256) { // 4MB
    for (unsigned int x = 0; x < 256; x++) {
      tempword = read_rom_word_LEAP(address + x);
      sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
      sdBuffer[(x * 2) + 1] = tempword & 0xFF;
    }
    myFile.write(sdBuffer, 512);
  }
  if (leapsize > 0) {
    for (unsigned long address = 0x200000; address < 0x400000; address += 256) { // +4MB = 8MB
      for (unsigned int x = 0; x < 256; x++) {
        tempword = read_rom_word_LEAP(address + x);
        sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
        sdBuffer[(x * 2) + 1] = tempword & 0xFF;
      }
      myFile.write(sdBuffer, 512);
    }
    if (leapsize > 1) {
      for (unsigned long address = 0x400000; address < 0x800000; address += 256) { // +8MB = 16MB
        for (unsigned int x = 0; x < 256; x++) {
          tempword = read_rom_word_LEAP(address + x);
          sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
          sdBuffer[(x * 2) + 1] = tempword & 0xFF;
        }
        myFile.write(sdBuffer, 512);
      }
    }
  }
  myFile.close();

  printCRC(fileName, NULL, 0);

  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_LEAP(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(LEAPSTER[index]);
}
#endif

void setROMSize_LEAP()
{
  byte newleapsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (leaplo == leaphi)
    newleapsize = leaplo;
  else {
    newleapsize = navigateMenu(leaplo, leaphi, &printRomSize_LEAP);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(LEAPSTER[newleapsize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (leaplo == leaphi)
    newleapsize = leaplo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (leaphi - leaplo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(LEAPSTER[i + leaplo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newleapsize = sizeROM.toInt() + leaplo;
    if (newleapsize > leaphi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(LEAPSTER[newleapsize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newleapsize);
  leapsize = newleapsize;
}

void checkStatus_LEAP()
{
  EEPROM_readAnything(8, leapsize);
  if (leapsize > leaphi) {
    leapsize = 1; // default 8M
    EEPROM_writeAnything(8, leapsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("LEAPSTER READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("TITLE: "));
  println_Msg(ROMTitle);
  print_Msg(F("VER: "));
  println_Msg(ROMVersion);
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(LEAPSTER[leapsize]);
  println_Msg(F("MB"));
  display_Update();
  wait();
#else
  Serial.print(F("TITLE: "));
  Serial.println(ROMTitle);
  Serial.print(F("VER: "));
  Serial.println(ROMVersion);
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(LEAPSTER[leapsize]);
  Serial.println(F("MB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// FLASH SAVES
//******************************************
// FLASH 39VF040
// MR. PENCIL'S LEARN TO DRAW & WRITE
// TOP SECRET - PERSONAL BEESWAX
// FLASH IS BYTE MODE 
// BYTE PIN IS A0 - SHIFT ADDRESS >> 1

void dataOut_LEAP()
{
  DDRC = 0xFF;
  DDRA = 0xFF;
}

void dataIn_LEAP()
{
  DDRC = 0x00;
  DDRA = 0x00;
}

void idFLASH_LEAP() // "BFD7" = 39VF040
{
  int ID1 = 0;
  int ID2 = 0;
  dataOut_LEAP();
  resetFLASH_LEAP();
  write_flash_byte_LEAP(0x5555, 0xAA);
  write_flash_byte_LEAP(0x2AAA, 0x55);
  write_flash_byte_LEAP(0x5555, 0x90);
  dataIn_LEAP();
  ID1 = read_flash_byte_LEAP(0x0000);
  ID2 = read_flash_byte_LEAP(0x0001);
  dataOut_LEAP();
  resetFLASH_LEAP();
  dataIn_LEAP();
  sprintf(flashid_str, "%02X%02X", ID1, ID2);
  display_Clear();
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);
  display_Update();
}

void resetFLASH_LEAP()
{
  write_flash_byte_LEAP(0x5555, 0xF0);
}

void eraseFLASH_LEAP()
{
  write_flash_byte_LEAP(0x5555, 0xAA);
  write_flash_byte_LEAP(0x2AAA, 0x55);
  write_flash_byte_LEAP(0x5555, 0x80);
  write_flash_byte_LEAP(0x5555, 0xAA);
  write_flash_byte_LEAP(0x2AAA, 0x55);
  write_flash_byte_LEAP(0x5555, 0x10);
}

void programFLASH_LEAP()
{
  write_flash_byte_LEAP(0x5555, 0xAA);
  write_flash_byte_LEAP(0x2AAA, 0x55);
  write_flash_byte_LEAP(0x5555, 0xA0);
}

void statusFLASH_LEAP()
{
  byte flashStatus = 1;
  do {
    flashStatus = ((PORTA & 0x80) >> 7); // D7 = PORTA7
    _delay_us(4);
  }
  while (flashStatus == 1);
}

void readFLASH_LEAP()
{
  createFolderAndOpenFile("LEAP", "SAVE", romName, "fla");

  if(myFile) {
    CE_HIGH;
    OE_LOW;
    FL_CE_LOW;
    for (unsigned long address = 0x0; address < 0x80000; address += 512) { // 512K
      if ((address % 0x8000) == 0) {
        print_Msg(F("*"));
        display_Update();
      }
      for (unsigned int x = 0; x < 512; x++) {
        // CONSOLE READS ADDRESS 4X
        do {
          tempbyte = read_flash_byte_LEAP(address + x);
          tempcheck = read_flash_byte_LEAP(address + x);
        }
        while (tempbyte != tempcheck);
        sdBuffer[x] = tempbyte;
      }
      myFile.write(sdBuffer, 512);
    }
    myFile.flush();
    myFile.close();
    FL_CE_HIGH;
    OE_HIGH;
    println_Msg(FS(FSTRING_EMPTY));
    printCRC(fileName, NULL, 0); // 512K
  }
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void writeFLASH_LEAP()
{
  fileBrowser(F("Select FLASH File"));
  sd.chdir();
  sprintf(filePath, "%s/%s", filePath, fileName);

  display_Clear();
  println_Msg(F("Writing File: "));
  println_Msg(filePath);
  println_Msg(fileName);
  display_Update();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    CE_HIGH;
    dataOut_LEAP();
    eraseFLASH_LEAP();
    statusFLASH_LEAP();
    delay(100); // WORKS ~75 OK
    for (unsigned long address = 0x0; address < 0x80000; address += 512) { // 512K
      myFile.read(sdBuffer, 512);
      if ((address % 0x10000) == 0) {
        print_Msg(F("*"));
        display_Update();
      }
      for (unsigned int x = 0; x < 512; x++) {
        programFLASH_LEAP();
        write_flash_byte_LEAP(address + x, sdBuffer[x]);
      }
    }
    dataIn_LEAP();
    myFile.close();
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("FLASH FILE WRITTEN!"));
    display_Update();
  }
  else {
    println_Msg(F("SD ERROR"));
    display_Update();
  }
  sd.chdir(); // root
  filePath[0] = '\0'; // Reset filePath
}

//******************************************
// EEPROM SAVES
//******************************************
// EEPROM 24LC02/24LC16
// 24LC02B = 256 BYTES
// 24LC16B = 2048 BYTES
// DORA THE EXPLORER - WILDLIFE RESCUE 24LC16B
// SCHOLASTIC MATH MISSIONS 24LC16B

// MODIFIED FOR COMPATIBILITY WITH ISSI EEPROM
// PET PALS ISSI 416A-2GLI = 24C16A

// START - SDA HIGH TO LOW WHEN SCL HIGH
void eepromStart_LEAP()
{
  SCL_LOW;
  SDA_LOW;
  SCL_HIGH;
  _delay_us(2);
  SDA_HIGH;
  _delay_us(2);
  SDA_LOW;
  _delay_us(2);
  SCL_LOW;
  _delay_us(2);
}

void eepromSet0_LEAP()
{
  SCL_LOW;
  SDA_LOW;
  _delay_us(2);
  SCL_HIGH;
  _delay_us(5);
  SCL_LOW;
  _delay_us(2);
}

void eepromSet1_LEAP()
{
  SCL_LOW;
  SDA_LOW;
  _delay_us(2);
  SDA_HIGH;
  _delay_us(2);
  SCL_HIGH;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP; // ~500ns
  SCL_LOW;
  SDA_LOW;
  _delay_us(2);
}

void eepromDevice_LEAP()
{
  eepromSet1_LEAP();
  eepromSet0_LEAP();
  eepromSet1_LEAP();
  eepromSet0_LEAP();
}

void eepromSetDeviceAddress_LEAP(unsigned long addrhi)
{
  for (int i = 0; i < 3; i++) {
    if ((addrhi >> 2) & 0x1) // Bit is HIGH
      eepromSet1_LEAP();
    else // Bit is LOW
      eepromSet0_LEAP();
    addrhi <<= 1; // rotate to the next bit
  }
}

void eepromStatus_LEAP() // ACK
{
  byte eepStatus = 1;
  DDRH &= ~(1 << 4); // SDA to INPUT
  SCL_LOW;
  _delay_us(2);
  SCL_HIGH;
  _delay_us(5);
  SCL_LOW;
  do {
    eepStatus = ((PINH & 0x10) >> 4); // SDA = PORTH4
  }
  while (eepStatus == 1);
  DDRH |= (1 << 4); // SDA to OUTPUT
}

void eepromReadMode_LEAP()
{
  eepromSet1_LEAP(); // READ
  eepromStatus_LEAP(); // ACK
}

void eepromWriteMode_LEAP()
{
  eepromSet0_LEAP(); // WRITE
  eepromStatus_LEAP(); // ACK
}

void eepromReadData_LEAP()
{
  DDRH &= ~(1 << 4); // SDA to INPUT
  for (int i = 0; i < 8; i++) {
    SCL_LOW;
    SDA_LOW;
    _delay_us(2);
    SCL_HIGH;
    eepbit[i] = ((PINH & 0x10) >> 4); // SDA = PORTH4
    if (eepbit[i] == 1) {
      SCL_LOW;
      _delay_us(4);
    }
    else {
      _delay_us(4);
      SCL_LOW;
    }
    _delay_us(2);
  }
  DDRH |= (1 << 4); // SDA to OUTPUT
}

void eepromWriteData_LEAP(byte data)
{
  for (int i = 0; i < 8; i++) {
    if ((data >> 7) & 0x1) // Bit is HIGH
      eepromSet1_LEAP();
    else // Bit is LOW
      eepromSet0_LEAP();
    data <<= 1; // rotate to the next bit
  }
  eepromStatus_LEAP(); // ACK
}

// STOP - SDA LOW TO HIGH WHEN SCL HIGH
void eepromStop_LEAP()
{
  SCL_LOW;
  SDA_LOW;
  _delay_us(1);
  SCL_HIGH;
  _delay_us(2);
  SDA_HIGH;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP; // ~500nS
  SCL_LOW;
  SDA_LOW;
}

void eepromSetAddress_LEAP(word address)
{
  for (int i = 0; i < 8; i++) {
    if ((address >> 7) & 0x1) // Bit is HIGH
      eepromSet1_LEAP();
    else // Bit is LOW
      eepromSet0_LEAP();
    address <<= 1; // rotate to the next bit
  }
  eepromStatus_LEAP(); // ACK
}

void readEepromByte_LEAP(word address)
{
  word addrhi = address >> 8;
  word addrlo = address & 0xFF;
  eepromStart_LEAP(); // START
  eepromDevice_LEAP(); // DEVICE [1010]
  eepromSetDeviceAddress_LEAP(addrhi); // ADDR [A10..A8]
  eepromWriteMode_LEAP();
  eepromSetAddress_LEAP(addrlo);
  eepromStart_LEAP(); // START
  eepromDevice_LEAP(); // DEVICE [1010]
  eepromSetDeviceAddress_LEAP(addrhi); // ADDR [A10..A8]
  eepromReadMode_LEAP();
  eepromReadData_LEAP();
  eepromStop_LEAP(); // STOP
  // OR 8 bits into byte
  eeptemp = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  sdBuffer[addrlo] = eeptemp;
}

void writeEepromByte_LEAP(word address)
{
  word addrhi = address >> 8;
  word addrlo = address & 0xFF;
  eeptemp = sdBuffer[addrlo];
  eepromStart_LEAP(); // START
  eepromDevice_LEAP(); // DEVICE [1010]
  eepromSetDeviceAddress_LEAP(addrhi); // ADDR [A10-A8]
  eepromWriteMode_LEAP(); // WRITE
  eepromSetAddress_LEAP(addrlo);
  eepromWriteData_LEAP(eeptemp);
  eepromStop_LEAP(); // STOP
}

// Read EEPROM and save to the SD card
void readEEP_LEAP()
{
  createFolderAndOpenFile("LEAP", "SAVE", romName, "eep");

  if (myFile) {
    for (word currByte = 0; currByte < 2048; currByte += 256) {
      for (int i = 0; i < 256; i++) {
        readEepromByte_LEAP(currByte + i);
      }
      myFile.write(sdBuffer, 256);
    }
// 24LC02
//    for (word currByte = 0; currByte < 256; currByte++) {
//      readEepromByte_LEAP(currByte);
//    }
//    myFile.write(sdBuffer, 256);
    myFile.close();

    printCRC(fileName, NULL, 0); // 2048
  }
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void writeEEP_LEAP()
{
  fileBrowser(F("Select EEPROM File"));
  sd.chdir();
  sprintf(filePath, "%s/%s", filePath, fileName);

  display_Clear();
  println_Msg(F("Writing File: "));
//  println_Msg(filePath);
  println_Msg(fileName);
  display_Update();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    WP_LOW;
    for (word currByte = 0; currByte < 2048; currByte += 256) {
      myFile.read(sdBuffer, 256);
      for (int i = 0; i < 256; i++) {
        writeEepromByte_LEAP(currByte + i);
        delay(50);
      }
      print_Msg(F("*"));
      display_Update();
    }
    WP_HIGH;
    myFile.close();
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("EEPROM FILE WRITTEN!"));
    display_Update();
  }
  else {
    println_Msg(F("SD ERROR"));
    display_Update();
  }
  sd.chdir(); // root
  filePath[0] = '\0'; // Reset filePath
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_LEAP()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("leapster.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
