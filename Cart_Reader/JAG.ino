//******************************************************************************
//* Atari Jaguar Module                                                        *
//*                                                                            *
//* Author:  skaman / cephiros                                                 *
//* Date:    2024-07-05                                                        *
//*                                                                            *
//******************************************************************************
#ifdef ENABLE_JAGUAR
// HARDWARE PROFILE
// PIN ASSIGNMENTS
// DATA D0-D7 - PORTF
// DATA D8-D15 - PORTK
// DATA D16-D23 - PORTL
// DATA D24-D31 - PORTC
// 74HC595 (ADDR) PINS - PORTE: ADDR (PE3-PJ0), SRCLR (PE4), SRCLK (PE5)
//                       PORTG: RCLK (PG5)
// EEPROM PINS - EEPDO (PA5), EEPSK (PA6), EEPCS (PA7), [EEPDI = DATA D0 (PF0)]
// CONTROL PINS - PORTH: OEH (PH3), OEL (PH4), CE (PH5), WE (PH6)

//******************************************
//  Defines
//******************************************
uint16_t tempDataLO = 0;
uint16_t tempDataHI = 0;

unsigned long jagCartSize;  // (0x20000)/0x100000/0x200000/0x400000
byte jagromSize = 0;        // 0 = 1MB, 1 = 2MB, 2 = 4MB
// Variable to count errors
//unsigned long writeErrors;
// SAVE TYPE
// 0 = SERIAL EEPROM
// 1 = FLASH
byte jagSaveType = 0;  // Serial EEPROM

// SERIAL EEPROM 93CX6
// CONTROL: EEPCS (PA7), EEPSK (PA6), EEPDI (PF0) [DATA D0]
// SERIAL DATA OUTPUT: EEPDO (PA5)
#define EEP_CS_SET PORTA |= (1 << 7)
#define EEP_CS_CLEAR PORTA &= ~(1 << 7)
#define EEP_SK_SET PORTA |= (1 << 6)
#define EEP_SK_CLEAR PORTA &= ~(1 << 6)
#define EEP_DI_SET PORTF |= (1 << 0)
#define EEP_DI_CLEAR PORTF &= ~(1 << 0)

// SERIAL EEPROM SIZES
// 0 = 93C46 = 128 byte = Standard
// 1 = 93C56 = 256 byte = Aftermarket
// 2 = 93C66 = 512 byte = Aftermarket
// 3 = 93C76 = 1024 byte = Aftermarket
// 4 = 93C86 = 2048 byte = Aftermarket - Battlesphere Gold
int jagEepSize;
byte jagEepBuf[2];
boolean jagEepBit[16];

// MEMORY TRACK CART
boolean jagMemorytrack = 0;
char jagFlashID[5];            // AT29C010 = "1FD5"
unsigned long jagflaSize = 0;  // AT29C010 = 128K

// JAGUAR EEPROM MAPPING
// 08 ROM SIZE
// 10 EEP SIZE

//******************************************
//  MENU
//******************************************
// Base Menu

static const char jagMenuItem6[] PROGMEM = "Read MEMORY TRACK";
static const char jagMenuItem7[] PROGMEM = "Write FLASH";

static const char* const menuOptionsJag[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, jagMenuItem6, jagMenuItem7 };

static const char jagRomItem1[] PROGMEM = "1MB ROM";
static const char jagRomItem2[] PROGMEM = "2MB ROM";
static const char jagRomItem3[] PROGMEM = "4MB ROM";
static const char* const jagRomMenu[] PROGMEM = { jagRomItem1, jagRomItem2, jagRomItem3 };

static const char jagEepItem1[] PROGMEM = "128B (93C46)";
static const char jagEepItem2[] PROGMEM = "256B (93C56)";
static const char jagEepItem3[] PROGMEM = "512B (93C66)";
static const char jagEepItem4[] PROGMEM = "1024B (93C76)";
static const char jagEepItem5[] PROGMEM = "2048B (93C86)";
static const char* const jagSaveMenu[] PROGMEM = { jagEepItem1, jagEepItem2, jagEepItem3, jagEepItem4, jagEepItem5 };
static const char* const ConfirmMenu[] PROGMEM = { FSTRING_OK, FSTRING_RESET };
void jagMenu() {

  convertPgm(menuOptionsJag, 7);
  uint8_t mainMenu = question_box(F("Jaguar MENU"), menuOptions, 8, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    // Select Cart
    case 0:
      setCart_Jag();
      break;

    // Read ROM
    case 1:
      display_Clear();
      // Change working dir to root
      readJagROM();
      break;

    // Set ROM Size
    case 2:
      jagSizeMenu();
      jagEepMenu();
      setDefaultRomName();
      getJagCartInfo();
      break;

    // Read EEPROM
    case 3:
      display_Clear();
      if (jagSaveType == 0)
        readJagEEP();
      else {
        println_Msg(F("Cart has no EEPROM"));
        display.display();
      }
      break;

    // Write EEPROM
    case 4:
      if (jagSaveType == 0) {
        // Launch file browser
        sd.chdir("/");
        fileBrowser(FS(FSTRING_SELECT_FILE));
        display_Clear();
        writeJagEEP();
      } else {
        println_Msg(F("Cart has no EEPROM"));
        display.display();
      }
      break;

    // Read MEMORY TRACK
    case 5:
      readJagMEMORY();
      if (jagSaveType == 1)
        readJagFLASH();
      else {
        println_Msg(F("Cart has no FLASH"));
        display.display();
      }
      break;

    // Write FLASH
    case 6:
      display_Clear();
      if (jagSaveType == 1) {
        // Launch file browser
        sd.chdir("/");
        fileBrowser(FS(FSTRING_SELECT_FILE));
        display_Clear();
        writeJagFLASH();
        verifyJagFLASH();
      } else {
        println_Msg(F("Cart has no FLASH"));
        display.display();
      }
      break;
  }
}

//******************************************
// CART SELECT CODE
//******************************************
struct database_entry_Jag {
  byte gameSize;
  byte saveSize;
};

void readDataLine_Jag(FsFile& database, void* entry) {
  struct database_entry_Jag* castEntry = (database_entry_Jag*)entry;
  // Read CRC32 checksum
  for (byte i = 0; i < 8; i++) {
    checksumStr[i] = char(database.read());
  }

  // Skip over semicolon
  database.seekCur(1);


  // Read rom size
  // Read the next ascii character and subtract 48 to convert to decimal
  castEntry->gameSize = (database.read() - 48);

  // Skip over semicolon
  database.seekCur(1);

  // Read save size
  // Read the next ascii character and subtract 48 to convert to decimal
  castEntry->saveSize = (database.read() - 48);

  // Skip rest of line
  database.seekCur(2);
}

void printDataLine_Jag(void* entry) {
  struct database_entry_Jag* castEntry = (database_entry_Jag*)entry;
  print_Msg(FS(FSTRING_SIZE));
  print_Msg(castEntry->gameSize);
  println_Msg(F("MB"));
  // 0 = 93C46 = 128 byte = Standard
  // 1 = 93C56 = 256 byte = Aftermarket
  // 2 = 93C66 = 512 byte = Aftermarket
  // 3 = 93C76 = 1024 byte = Aftermarket
  // 4 = 93C86 = 2048 byte = Aftermarket - Battlesphere Gold
  switch (castEntry->saveSize) {
    case 0:
      println_Msg(F("Save: 128B"));
      break;

    case 1:
      println_Msg(F("Save: 256B"));
      break;

    case 2:
      println_Msg(F("Save: 512B"));
      break;
    case 3:
      println_Msg(F("Save: 1KB"));
      break;
    case 4:
      println_Msg(F("Save: 2KB"));
      break;
  }
}
#ifndef ENABLE_NES
void setDefaultRomName() {
  romName[0] = 'C';
  romName[1] = 'A';
  romName[2] = 'R';
  romName[3] = 'T';
  romName[4] = '\0';
}

void setRomnameFromString(const char* input) {
  uint8_t myLength = 0;
  for (uint8_t i = 0; i < 20 && myLength < 15; i++) {
    // Stop at first "(" to remove "(Country)"
    if (input[i] == '(') {
      break;
    }
    if (
      (input[i] >= '0' && input[i] <= '9') || (input[i] >= 'A' && input[i] <= 'Z') || (input[i] >= 'a' && input[i] <= 'z')) {
      romName[myLength++] = input[i];
    }
  }

  // If name consists out of all japanese characters use CART as name
  if (myLength == 0) {
    setDefaultRomName();
  }
}
#endif
void setCart_Jag() {
  //go to root
  sd.chdir();

  struct database_entry_Jag entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("jag.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);


    if (checkCartSelection(myFile, &readDataLine_Jag, &entry, &printDataLine_Jag, &setRomnameFromString)) {
      EEPROM_writeAnything(10, entry.saveSize);
      jagEepSize = entry.saveSize;
      switch (entry.gameSize) {
        case 1:
          jagromSize = 0;
          EEPROM_writeAnything(8, jagromSize);
          jagCartSize = 0x100000;
          break;

        case 2:
          jagromSize = 1;
          EEPROM_writeAnything(8, jagromSize);
          jagCartSize = 0x200000;
          break;

        case 4:
          jagromSize = 2;
          EEPROM_writeAnything(8, jagromSize);
          jagCartSize = 0x400000;
          break;
      }
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}

//******************************************************************************
// ROM SIZE MENU
//******************************************************************************
void jagSizeMenu() {

  convertPgm(jagRomMenu, 3);
  unsigned char subMenu = question_box(F("Select ROM Size"), menuOptions, 3, 0);

  switch (subMenu) {
    case 0:
      jagromSize = 0;
      EEPROM_writeAnything(8, jagromSize);
      jagCartSize = 0x100000;
      break;

    case 1:
      jagromSize = 1;
      EEPROM_writeAnything(8, jagromSize);
      jagCartSize = 0x200000;
      break;

    case 2:
      jagromSize = 2;
      EEPROM_writeAnything(8, jagromSize);
      jagCartSize = 0x400000;
      break;
  }
}

//******************************************************************************
// EEPROM SIZE MENU
//******************************************************************************
void jagEepMenu() {
  convertPgm(jagSaveMenu, 5);
  unsigned char subMenu = question_box(F("Select EEPROM Size"), menuOptions, 5, 0);

  switch (subMenu) {
    case 0:
      jagEepSize = 0;  // 128B
      EEPROM_writeAnything(10, jagEepSize);
      println_Msg(F("128B (93C46)"));
      display.display();
      break;

    case 1:
      jagEepSize = 1;  // 256B
      EEPROM_writeAnything(10, jagEepSize);
      println_Msg(F("256B (93C56)"));
      display.display();
      break;

    case 2:
      jagEepSize = 2;  // 512B
      EEPROM_writeAnything(10, jagEepSize);
      println_Msg(F("512B (93C66)"));
      display.display();
      break;

    case 3:
      jagEepSize = 3;  // 1024B
      EEPROM_writeAnything(10, jagEepSize);
      println_Msg(F("1024B (93C76)"));
      display.display();
      break;

    case 4:
      jagEepSize = 4;  // 2048B
      EEPROM_writeAnything(10, jagEepSize);
      println_Msg(F("2048B (93C86)"));
      display.display();
      break;
  }
}
//******************************************
//  SETUP
//******************************************
void setup_Jag() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output ADDR(PE3)(PJ0), SRCLR(PE4), SRCLK(PE5), RCLK(PG5)
  DDRE |= (1 << 3) | (1 << 4) | (1 << 5);
  DDRG |= (1 << 5);

  // Set Control Pins to Output OEH(PH3), OEL(PH4), CE(PH5), WE(PH6)
  DDRH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set EEPROM Control Pins to Output EEPSK(PA6), EEPCS(PA7)
  DDRA |= (1 << 6) | (1 << 7);

  // Set EEPROM Data to Input EEPDO(PA5)
  DDRA &= ~(1 << 5);

  // Set Data Pins (D0-D31) to Input
  DDRF = 0x00;
  DDRK = 0x00;
  DDRL = 0x00;
  DDRC = 0x00;

  // Set Control Pins HIGH - OEH(PH3), OEL(PH4), CE(PH5), WE(PH6)
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set 74HC595 (Address) Clear LOW - SRCLR (PE4)
  PORTE &= ~(1 << 4);  // Disable Address Shift Register

  EEPROM_readAnything(8, jagromSize);
  EEPROM_readAnything(10, jagEepSize);
  // Print all the info
  if (jagromSize > 2) {
    jagromSize = 1;  // default 2MB
    EEPROM_writeAnything(8, jagromSize);
  }
  jagCartSize = long(int_pow(2, jagromSize)) * 0x100000;
  strcpy(romName, "JAG");
  if (jagEepSize > 4) {
    jagEepSize = 0;  // default 128B
    EEPROM_writeAnything(10, jagEepSize);
  }
  readJagID();

  if (strcmp(jagFlashID, "1FD5") == 0) {  // AT29C010 128K FLASH
    jagMemorytrack = 1;                   // Memory Track Found
    jagSaveType = 1;                      // FLASH
    strcpy(romName, "jagMemorytrack");
    jagCartSize = 0x20000;
    jagflaSize = 0x20000;
  } else
    setCart_Jag();
  getJagCartInfo();

  mode = CORE_JAGUAR;
}

//******************************************************************************
// GET CART INFO
//******************************************************************************
void getJagCartInfo() {
  // Check for Memory Track NVRAM Cart

  display_Clear();
  println_Msg(F("CART INFO"));
  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Size: "));
  if (jagMemorytrack) {
    print_Msg(jagCartSize / 1024);
    println_Msg(F(" KB"));
  } else {

    print_Msg(jagCartSize / 1024 / 1024);
    println_Msg(F(" MB"));
  }
  if (jagSaveType == 0) {
    print_Msg(F("EEPROM: "));
    print_Msg(int_pow(2, jagEepSize) * 128);  // 128/256/512/1024/2048 BYTES
    println_Msg(F(" B"));
  } else if (jagSaveType == 1) {
    print_Msg(F("FLASH: "));
    print_Msg(jagflaSize / 1024);
    println_Msg(F(" KB"));
  }
  display_Update();

  println_Msg(F("Press Button..."));

  wait();
}

//******************************************************************************
// SHIFT OUT
//******************************************************************************
// SHIFT OUT ADDRESS CODE
// SN74HC595 (ADDR) PINS - PORTE: ADDR (PJ3-PE3), SRCLR (PE4), SRCLK (PE5)
//                         PORTG: RCLK (PG5)
// DATA (SER/DS) PIN 14 = PE3 PJ0
// /OE (GND) PIN 13
// LATCH (RCLK/STCP) PIN 12 - LO/HI TO OUTPUT ADDRESS = PG5
// CLOCK (SRCLK/SHCP) PIN 11 - LO/HI TO READ ADDRESS  = PE5
// RESET (/SRCLR//MR) PIN 10  = PE4

#define SER_CLEAR PORTE &= ~(1 << 3);
#define SER_SET PORTE |= (1 << 3);
#define SRCLR_CLEAR PORTE &= ~(1 << 4);
#define SRCLR_SET PORTE |= (1 << 4);
#define CLOCK_CLEAR PORTE &= ~(1 << 5);
#define CLOCK_SET PORTE |= (1 << 5);
#define LATCH_CLEAR PORTG &= ~(1 << 5);
#define LATCH_SET PORTG |= (1 << 5);

// INPUT ADDRESS BYTE IN MSB
// LATCH LO BEFORE FIRST SHIFTOUT
// LATCH HI AFTER LAST SHIFOUT
void shiftOutFAST(byte addr) {    //
  for (int i = 7; i >= 0; i--) {  // MSB
    CLOCK_CLEAR;
    if (addr & (1 << i)) {
      SER_SET;  // 1
    } else {
      SER_CLEAR;  // 0
    }
    CLOCK_SET;  // shift bit
    SER_CLEAR;  // reset 1
  }
  CLOCK_CLEAR;
}

//******************************************
// READ DATA
//******************************************
void readJagData(unsigned long myAddress) {
  SRCLR_CLEAR;
  SRCLR_SET;
  LATCH_CLEAR;

  shiftOutFAST((myAddress >> 16) & 0xFF);
  shiftOutFAST((myAddress >> 8) & 0xFF);
  shiftOutFAST(myAddress);
  LATCH_SET;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting CE(PH5) LOW
  PORTH &= ~(1 << 5);
  // Setting OEH3(PH3) + OEL(PH4) LOW
  PORTH &= ~(1 << 3) & ~(1 << 4);

  // Long delay here or there will be read errors
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  tempDataLO = (((PINK & 0xFF) << 8) | (PINF & 0xFF));  // D0-D15 [ROM U1]
  tempDataHI = (((PINC & 0xFF) << 8) | (PINL & 0xFF));  // D16-D31 [ROM U2]

  // Setting OEH(PH3) + OEL(PH4) HIGH
  PORTH |= (1 << 3) | (1 << 4);
  // Setting CE(PH5) HIGH
  PORTH |= (1 << 5);
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  SRCLR_CLEAR;
}
// Switch data pins to write
void dataOut_Jag() {
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRL = 0xFF;
  DDRC = 0xFF;
}

// Switch data pins to read
void dataIn_Jag() {
  DDRF = 0x00;
  DDRK = 0x00;
  DDRL = 0x00;
  DDRC = 0x00;
}



//******************************************************************************
// READ ROM
//******************************************************************************
// Read rom and save to the SD card
void readJagROM() {
  // Set control
  dataIn_Jag();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".J64");

  // create a new folder
  sd.chdir();
  EEPROM_readAnything(0, foldern);
  //  sprintf(folder, "JAG/ROM/%s/%d", romName, foldern);
  sprintf(folder, "JAG/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving ROM to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("SD ERROR"));
    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(jagCartSize);
  draw_progressbar(0, totalProgressBar);

  int d = 0;
  for (unsigned long currBuffer = 0; currBuffer < jagCartSize; currBuffer += 512) {
    // Blink led
    if (currBuffer % 16384 == 0)
      blinkLED();
    // MaskROM does not use A0 + A1 - Shift Address *4
    for (int currWord = 0; currWord < 512; currWord += 4) {
      readJagData(currBuffer + currWord);
      // Move WORD into SD Buffer
      sdBuffer[d] = (tempDataHI >> 8) & 0xFF;
      sdBuffer[d + 1] = tempDataHI & 0xFF;
      sdBuffer[d + 2] = (tempDataLO >> 8) & 0xFF;
      sdBuffer[d + 3] = tempDataLO & 0xFF;
      d += 4;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  // Close the file:
  myFile.close();

  compareCRC("jag.txt", 0, 1, 0);

  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}


//*****************************************************************************
// MICROCHIP EEPROM 93C56/66/76/86 CODE
// SERIAL DATA OUTPUT (DO):  PIN PA5
//
// ISSI 93C46-3P EEPROM ONLY USES WORD MODE
// EEPROM CODE IS WRITTEN IN WORD MODE (16bit)
//
// 93C46 (16bit): A5..A0, EWEN/ERAL/EWDS XXXX
// 93C56 (16bit): A6..A0, EWEN/ERAL/EWDS XXXXXX
// 93C66 (16bit): A7..A0, EWEN/ERAL/EWDS XXXXXX
// 93C76 (16bit): A8..A0, EWEN/ERAL/EWDS XXXXXXXX
// 93C86 (16bit): A9..A0, EWEN/ERAL/EWDS XXXXXXXX
//
// MICROCHIP EEPROM - TIE PIN 6 (ORG) TO VCC (ORG = 1) TO ENABLE 16bit MODE
// FOR 93C76 & 93C86, TIE PIN 7 (PE) TO VCC TO ENABLE PROGRAMMING
//*****************************************************************************
void Eepromdisplay_Clear() {
  DDRF |= (1 << 0);   // DATA PIN PF0 AS OUTPUT
  DDRA &= ~(1 << 5);  // DO INPUT
  EEP_CS_CLEAR;
  EEP_SK_CLEAR;
  EEP_DI_CLEAR;
}

void EepromresetArduino() {
  DDRF &= ~(1 << 0);  // DATA PIN PF0 AS INPUT
  EEP_CS_CLEAR;
  EEP_SK_CLEAR;
  EEP_DI_CLEAR;
}

void Eeprom0() {
  EEP_DI_CLEAR;
  EEP_SK_SET;
  _delay_us(1);  // minimum 250ns
  EEP_DI_CLEAR;
  EEP_SK_CLEAR;
  _delay_us(1);  // minimum 250ns
}

void Eeprom1() {
  EEP_DI_SET;
  EEP_SK_SET;
  _delay_us(1);  // minimum 250ns
  EEP_DI_CLEAR;
  EEP_SK_CLEAR;
  _delay_us(1);  // minimum 250ns
}

void EepromRead(uint16_t addr) {
  Eepromdisplay_Clear();
  EEP_CS_SET;
  Eeprom1();  // 1
  Eeprom1();  // 1
  Eeprom0();  // 0
  if ((jagEepSize == 1) || (jagEepSize == 3))
    Eeprom0();  // Dummy 0 for 56/76
  jagEepromSetAddress(addr);
  _delay_us(12);  // From Willem Timing
  // DATA OUTPUT
  EepromreadData();
  EEP_CS_CLEAR;
  // OR 16 bits into two bytes
  for (int j = 0; j < 16; j += 8) {
    jagEepBuf[(j / 8)] = jagEepBit[0 + j] << 7 | jagEepBit[1 + j] << 6 | jagEepBit[2 + j] << 5 | jagEepBit[3 + j] << 4 | jagEepBit[4 + j] << 3 | jagEepBit[5 + j] << 2 | jagEepBit[6 + j] << 1 | jagEepBit[7 + j];
  }
}

// Capture 2 bytes in 16 bits into bit array jagEepBit[]
void EepromreadData(void) {
  for (int i = 0; i < 16; i++) {
    EEP_SK_SET;
    _delay_us(1);                         // minimum 250ns
    jagEepBit[i] = ((PINA & 0x20) >> 5);  // Read DO (PA5) - PINA with Mask 0x20
    EEP_SK_CLEAR;
    _delay_us(1);  // minimum 250ns
  }
}

void EepromWrite(uint16_t addr) {
  Eepromdisplay_Clear();
  EEP_CS_SET;
  Eeprom1();  // 1
  Eeprom0();  // 0
  Eeprom1();  // 1
  if ((jagEepSize == 1) || (jagEepSize == 3))
    Eeprom0();  // Dummy 0 for 56/76
  jagEepromSetAddress(addr);
  // DATA OUTPUT
  EepromWriteData();
  EEP_CS_CLEAR;
  jagEepromStatus();
}

void EepromWriteData(void) {
  byte UPPER = jagEepBuf[1];
  byte LOWER = jagEepBuf[0];
  for (int i = 0; i < 8; i++) {
    if (((UPPER >> 7) & 0x1) == 1) {  // Bit is HIGH
      Eeprom1();
    } else {  // Bit is LOW
      Eeprom0();
    }
    // rotate to the next bit
    UPPER <<= 1;
  }
  for (int j = 0; j < 8; j++) {
    if (((LOWER >> 7) & 0x1) == 1) {  // Bit is HIGH
      Eeprom1();
    } else {  // Bit is LOW
      Eeprom0();
    }
    // rotate to the next bit
    LOWER <<= 1;
  }
}

void jagEepromSetAddress(uint16_t addr) {  // 16bit
  uint8_t shiftaddr = jagEepSize + 6;      // 93C46 = 0 + 6, 93C56 = 7, 93C66 = 8, 93C76 = 9, 93C86 = 10
  for (int i = 0; i < shiftaddr; i++) {
    if (((addr >> shiftaddr) & 1) == 1) {  // Bit is HIGH
      Eeprom1();
    } else {  // Bit is LOW
      Eeprom0();
    }
    // rotate to the next bit
    addr <<= 1;
  }
}

// EWEN/ERAL/EWDS
// 93C56/93C66 - 10000xxxxxx (6 PULSES)
// 93C76/93C86 - 10000xxxxxxxx (8 PULSES)
void EepromEWEN(void) {  // EWEN 10011xxxx
  Eepromdisplay_Clear();
  EEP_CS_SET;
  Eeprom1();  // 1
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom1();  // 1
  Eeprom1();  // 1
  // 46 = 4x Trailing 0s for 16bit
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  // 56/66 = 6x Trailing 0s for 16bit
  if (jagEepSize > 0) {
    Eeprom0();  // 0
    Eeprom0();  // 0
  }
  // 76/86 = 8x Trailing 0s for 16bit
  if (jagEepSize > 2) {
    Eeprom0();  // 0
    Eeprom0();  // 0
  }
  EEP_CS_CLEAR;
  _delay_us(2);
#ifdef SERIAL_MONITOR
  Serial.println(F("ERASE ENABLED"));
#endif
}

void EepromERAL(void) {  // ERASE ALL 10010xxxx
  EEP_CS_SET;
  Eeprom1();  // 1
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom1();  // 1
  Eeprom0();  // 0
  // 46 = 4x Trailing 0s for 16bit
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  // 56/66 = 6x Trailing 0s for 16bit
  if (jagEepSize > 0) {
    Eeprom0();  // 0
    Eeprom0();  // 0
  }
  // 76/86 = 8x Trailing 0s for 16bit
  if (jagEepSize > 2) {
    Eeprom0();  // 0
    Eeprom0();  // 0
  }
  EEP_CS_CLEAR;
  jagEepromStatus();
#ifdef SERIAL_MONITOR
  Serial.println(F("ERASED ALL"));
#endif
}

void EepromEWDS(void) {  // DISABLE 10000xxxx
  Eepromdisplay_Clear();
  EEP_CS_SET;
  Eeprom1();  // 1
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  // 46 = 4x Trailing 0s for 16bit
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  Eeprom0();  // 0
  // 56/66 = 6x Trailing 0s for 16bit
  if (jagEepSize > 0) {
    Eeprom0();  // 0
    Eeprom0();  // 0
  }
  // 76/86 = 8x Trailing 0s for 16bit
  if (jagEepSize > 2) {
    Eeprom0();  // 0
    Eeprom0();  // 0
  }
  EEP_CS_CLEAR;
  _delay_us(2);
#ifdef SERIAL_MONITOR
  Serial.println(F("ERASE DISABLED"));
#endif
}

void jagEepromStatus(void) {  // CHECK READY/BUSY
  __asm__("nop\n\t"
          "nop\n\t");  // CS LOW for minimum 100ns
  EEP_CS_SET;
  boolean status = ((PINA & 0x20) >> 5);  // Check DO
  do {
    _delay_ms(1);
    status = ((PINA & 0x20) >> 5);
  } while (!status);  // status == 0 = BUSY
  EEP_CS_CLEAR;
}

#ifdef SERIAL_MONITOR
void EepromDisplay() {  // FOR SERIAL ONLY
  word eepEnd = int_pow(2, jagEepSize) * 128;
  for (word address = 0; address < eepEnd; address += 2) {
    EepromRead(address);
    if ((address % 16 == 0) && (address != 0))
      Serial.println(F(""));
    if (jagEepBuf[1] < 0x10)
      Serial.print(F("0"));
    Serial.print(jagEepBuf[1], HEX);
    Serial.print(F(" "));
    if (jagEepBuf[0] < 0x10)
      Serial.print(F("0"));
    Serial.print(jagEepBuf[0], HEX);
    Serial.print(F(" "));
  }
  Serial.println(F(""));
  Serial.println(F(""));
}
#endif
//*****************************************************************************
// EEPROM
// (0) 93C46 128B STANDARD
// (1) 93C56 256B AFTERMARKET
// (2) 93C66 512B AFTERMARKET
// (3) 93C76 1024B AFTERMARKET
// (4) 93C86 2048B AFTERMARKET - Battlesphere Gold
//*****************************************************************************
// Read EEPROM and save to the SD card
void readJagEEP() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".eep");
  println_Msg(F("Reading..."));

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sd.chdir();
  //  sprintf(folder, "JAG/SAVE/%s/%d", romName, foldern);
  sprintf(folder, "JAG/SAVE/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("SD ERROR"));

    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }
  word eepEnd = int_pow(2, jagEepSize) * 64;  // WORDS
  if (jagEepSize > 1) {                       // 93C66/93C76/93C86 - 256/512/1024 WORDS
    for (word currWord = 0; currWord < eepEnd; currWord += 256) {
      for (int i = 0; i < 256; i++) {
        EepromRead((currWord + i) * 2);
        sdBuffer[(i * 2)] = jagEepBuf[1];
        sdBuffer[(i * 2) + 1] = jagEepBuf[0];
      }
      myFile.write(sdBuffer, 512);
    }
  } else {  // 93C46/93C56 - 64/128 WORDS
    for (word currWord = 0; currWord < eepEnd; currWord++) {
      EepromRead(currWord * 2);
      sdBuffer[(currWord * 2)] = jagEepBuf[1];
      sdBuffer[(currWord * 2) + 1] = jagEepBuf[0];
    }
    myFile.write(sdBuffer, eepEnd * 2);
  }
  EepromresetArduino();
  // Close the file:
  myFile.close();
  println_Msg(F(""));
  print_Msg(F("Saved to "));
  println_Msg(folder);
  display_Update();
  wait();
}

// NOTE:  TO WRITE TO 93C76 & 93C86, MUST TIE PE (PROGRAM ENABLE) PIN 7 TO VCC
void writeJagEEP() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    EepromEWEN();  // ERASE/WRITE ENABLE
    EepromERAL();  // ERASE ALL
#ifdef SERIAL_MONITOR
    Serial.println(F("WRITING"));
#endif
    word eepEnd = int_pow(2, jagEepSize) * 64;  // WORDS
    if (jagEepSize > 1) {                       // 93C66/93C76/93C86
      for (word currWord = 0; currWord < eepEnd; currWord += 256) {
        myFile.read(sdBuffer, 512);
        for (int i = 0; i < 256; i++) {
          jagEepBuf[0] = sdBuffer[(i * 2)];
          jagEepBuf[1] = sdBuffer[(i * 2) + 1];
          EepromWrite((currWord + i) * 2);
        }
      }
    } else {  // 93C46/93C56
      myFile.read(sdBuffer, eepEnd * 2);
      for (word currWord = 0; currWord < eepEnd; currWord++) {
        jagEepBuf[0] = sdBuffer[currWord * 2];
        jagEepBuf[1] = sdBuffer[(currWord * 2) + 1];
        EepromWrite(currWord * 2);
      }
    }
    EepromEWDS();  // ERASE/WRITE DISABLE
    EepromresetArduino();
    // Close the file:
    myFile.close();
    println_Msg(F(""));
    println_Msg(F("DONE"));
    display_Update();
  } else {
    println_Msg(F("SD ERROR"));
    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }
}

//*****************************************************************************
// MEMORY TRACK NVRAM FLASH
//*****************************************************************************
void readJagID() {  // AT29C010 Flash ID "1FD5"
  // Switch to write
  dataOut_Jag();

  // ID command sequence
  writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
  writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
  writeBYTE_FLASH(0x15554, 0x90);  // 0x5555

  // Switch to read
  dataIn_Jag();

  // Read the two id bytes into a string
  sprintf(jagFlashID, "%02X%02X", readBYTE_FLASH(0x0000), readBYTE_FLASH(0x0004));

  resetFLASH();
}

void eraseFLASH() {  // Chip Erase (NOT NEEDED FOR WRITES)
  // Switch to write
  dataOut_Jag();

  println_Msg(F("Erasing..."));
  display_Update();

  // Erase command sequence
  writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
  writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
  writeBYTE_FLASH(0x15554, 0x80);  // 0x5555
  writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
  writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
  writeBYTE_FLASH(0x15554, 0x10);  // 0x5555

  // Wait for command to complete
  busyCheck();

  // Switch to read
  dataIn_Jag();
}

void resetFLASH() {
  // Switch to write
  dataOut_Jag();

  // Reset command sequence
  writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
  writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
  writeBYTE_FLASH(0x15554, 0xF0);  // 0x5555

  // Switch to read
  dataIn_Jag();
  delayMicroseconds(10);
}

void busyCheck() {
  // Switch to read
  dataIn_Jag();

  // Read register
  readBYTE_FLASH(0x0000);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while (((PINL >> 6) & 0x1) == 0) {  // IO6 = PORTL PL6
    // Setting CE(PH5) HIGH
    PORTH |= (1 << 5);

    // Leave CE high for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Setting CE(PH5) LOW
    PORTH &= ~(1 << 5);

    // Leave CE low for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Read register
    readBYTE_FLASH(0x0000);
  }

  // Switch to write
  dataOut_Jag();
}

byte readBYTE_FLASH(unsigned long myAddress) {
  SRCLR_CLEAR;
  SRCLR_SET;
  LATCH_CLEAR;
  shiftOutFAST((myAddress >> 16) & 0xFF);
  shiftOutFAST((myAddress >> 8) & 0xFF);
  shiftOutFAST(myAddress);
  LATCH_SET;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting CE(PH5) LOW
  PORTH &= ~(1 << 5);
  // Setting OEH(PH3) HIGH
  PORTH |= (1 << 3);
  // Setting OEL(PH4) LOW
  PORTH &= ~(1 << 4);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = PINL;  // D16..D23

  // Setting CE(PH5) HIGH
  PORTH |= (1 << 5);
  // Setting OEL(PH4) HIGH
  PORTH |= (1 << 4);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
}

byte readBYTE_MEMROM(unsigned long myAddress) {
  SRCLR_CLEAR;
  SRCLR_SET;
  LATCH_CLEAR;
  shiftOutFAST((myAddress >> 16) & 0xFF);
  shiftOutFAST((myAddress >> 8) & 0xFF);
  shiftOutFAST(myAddress);
  LATCH_SET;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting CE(PH5) LOW
  PORTH &= ~(1 << 5);
  // Setting OEH(PH3) LOW
  PORTH &= ~(1 << 3);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = PINF;  // D0..D7

  // Setting CE(PH5) HIGH
  PORTH |= (1 << 5);
  // Setting OEH(PH3) HIGH
  PORTH |= (1 << 3);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
}

void writeBYTE_FLASH(unsigned long myAddress, byte myData) {
  SRCLR_CLEAR;
  SRCLR_SET;
  LATCH_CLEAR;
  shiftOutFAST((myAddress >> 16) & 0xFF);
  shiftOutFAST((myAddress >> 8) & 0xFF);
  shiftOutFAST(myAddress);
  LATCH_SET;

  PORTL = myData;

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting OEL(PH4) HIGH
  PORTH |= (1 << 4);
  // Setting CE(PH5) LOW
  PORTH &= ~(1 << 5);
  // Switch WE(PH6) LOW
  PORTH &= ~(1 << 6);
  delayMicroseconds(10);

  // Switch WE(PH6) HIGH
  PORTH |= (1 << 6);
  // Setting CE(PH5) HIGH
  PORTH |= (1 << 5);
  delayMicroseconds(10);
}

void writeSECTOR_FLASH(unsigned long myAddress) {
  dataOut_Jag();

  // Enable command sequence
  writeBYTE_FLASH(0x15554, 0xAA);  // 0x5555
  writeBYTE_FLASH(0xAAA8, 0x55);   // 0x2AAA
  writeBYTE_FLASH(0x15554, 0xA0);  // 0x5555

  for (int i = 0; i < 128; i++) {
    SRCLR_CLEAR;
    SRCLR_SET;
    LATCH_CLEAR;
    shiftOutFAST((((myAddress + i) * 4) >> 16) & 0xFF);
    shiftOutFAST((((myAddress + i) * 4) >> 8) & 0xFF);
    shiftOutFAST((myAddress + i) * 4);  // (myAddress + i) * 4
    LATCH_SET;

    PORTL = sdBuffer[i];
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");

    // Setting OEL(PH4) HIGH
    //    PORTH |= (1 << 4);
    // Setting CE(PH5) LOW
    PORTH &= ~(1 << 5);
    // Switch WE(PH6) LOW
    PORTH &= ~(1 << 6);
    __asm__("nop\n\t"
            "nop\n\t");  // Minimum 90ns

    // Switch WE(PH6) HIGH
    PORTH |= (1 << 6);
    // Setting CE(PH5) HIGH
    PORTH |= (1 << 5);

    __asm__("nop\n\t"
            "nop\n\t");  // Minimum 100ns
  }
  delay(30);
}

//*****************************************************************************
// MEMORY TRACK ROM U1
// U1 ADDR PINS A0..A18 [USES A0..A17 = 0x20000 BYTES]
// U1 DATA PINS D0..D7
// /CE ON PIN 20A (CONTROLS BOTH U1 AND U2)
// /OEH ON PIN 23B (CONTROLS U1 ROM ONLY)
//*****************************************************************************
void readJagMEMORY() {
  // Set control
  dataIn_Jag();

  strcpy(fileName, romName);
  strcat(fileName, ".J64");

  // create a new folder
  sd.chdir();
  EEPROM_readAnything(0, foldern);
  //  sprintf(folder, "JAG/ROM/%s/%d", romName, foldern);
  sprintf(folder, "JAG/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving ROM to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("SD ERROR"));
    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }
  // Memory Track ROM Size 0x20000
  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(jagCartSize);
  draw_progressbar(0, totalProgressBar);
  for (unsigned long currBuffer = 0; currBuffer < jagCartSize; currBuffer += 512) {
    // Blink led
    if (currBuffer % 16384 == 0)
      blinkLED();
    for (int currByte = 0; currByte < 512; currByte++) {
      sdBuffer[currByte] = readBYTE_MEMROM(currBuffer + currByte);
    }
    myFile.write(sdBuffer, 512);
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  // Close the file:
  myFile.close();

  println_Msg(F("Press Button..."));
  wait();
}

//*****************************************************************************
// MEMORY TRACK FLASH U2
// AT29C010 = 0x20000 BYTES = 128 KB
// U2 ADDR PINS A2..A18
// U2 DATA PINS D16..D23
// /CE ON PIN 20A (CONTROLS BOTH U1 AND U2)
// /OEL ON PIN 22B (CONTROLS U2 FLASH ONLY)
//*****************************************************************************
void readJagFLASH() {
  dataIn_Jag();
  strcpy(fileName, romName);
  strcat(fileName, ".fla");

  // create a new folder for the save file
  sd.chdir();
  EEPROM_readAnything(0, foldern);
  //  sprintf(folder, "JAG/SAVE/%s/%d", romName, foldern);
  sprintf(folder, "JAG/SAVE/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving FLASH to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("SD ERROR"));
    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }
  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(jagflaSize);
  draw_progressbar(0, totalProgressBar);
  for (unsigned long currByte = 0; currByte < jagflaSize; currByte += 512) {
    // Blink led
    if (currByte % 16384 == 0)
      blinkLED();
    for (int i = 0; i < 512; i++) {
      sdBuffer[i] = readBYTE_FLASH((currByte + i) * 4);  // Address Shift A2..A18
    }
    myFile.write(sdBuffer, 512);
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  // Close the file:
  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/"));

  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

// AT29C010 - Write in 128 BYTE Secrors
// Write Enable then Write DATA
void writeJagFLASH() {
  dataOut_Jag();

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)(jagflaSize);
    draw_progressbar(0, totalProgressBar);
    for (unsigned long currByte = 0; currByte < jagflaSize; currByte += 128) {
      // Blink led
      if (currByte % 16384 == 0)
        blinkLED();
      // Load Data
      for (int i = 0; i < 128; i++) {
        sdBuffer[i] = myFile.read() & 0xFF;
      }
      writeSECTOR_FLASH(currByte);
      processedProgressBar += 128;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    // Close the file:
    myFile.close();
    println_Msg(F("WRITE COMPLETE"));
    display_Update();
  } else {
    println_Msg(F("SD ERROR"));
    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }
  dataIn_Jag();
}

unsigned long verifyJagFLASH() {
  dataIn_Jag();
  writeErrors = 0;

  println_Msg(F(""));
  println_Msg(F("Verifying..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currByte = 0; currByte < jagflaSize; currByte += 512) {
      for (int i = 0; i < 512; i++) {
        sdBuffer[i] = readBYTE_FLASH((currByte + i) * 4);
      }
      // Check sdBuffer content against file on sd card
      for (int j = 0; j < 512; j++) {
        if (myFile.read() != sdBuffer[j]) {
          writeErrors++;
        }
      }
    }
    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("SD ERROR"));
    println_Msg(F("Press Button to Reset"));
    display_Update();
    wait();
    resetArduino();
  }
  if (!writeErrors)
    println_Msg(F("WRITE VERIFIED"));
  else
    println_Msg(F("WRITE ERROR"));
  display_Update();
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}
//******************************************************************************
// WRITE CONFIRM
//******************************************************************************
void writeConfirm() {
  display_Clear();
  println_Msg(F("***** WARNING ******"));
  println_Msg(F("* YOU ARE ABOUT TO *"));
  println_Msg(F("* ERASE SAVE DATA! *"));
  println_Msg(F("********************"));
  wait();
  convertPgm(ConfirmMenu, 4);
  uint8_t resetMenu = question_box(F("CONTINUE?"), menuOptions, 2, 0);

  // wait for user choice to come back from the question box menu
  switch (resetMenu) {
    case 0:
      return;
    case 1:
      println_Msg(F("Press Button to Reset"));
      wait();
      resetArduino();
  }
}
#endif
//******************************************
// End of File
//******************************************
