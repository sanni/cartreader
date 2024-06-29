//******************************************
// MSX COMPUTER MODULE
//******************************************
#ifdef ENABLE_MSX
// MSX
// Cartridge Pinout
// 50P 2.54mm pitch connector
//
//       FRONT            BACK
//        SIDE            SIDE
//              +-------+
//        /CS2 -| 2   1 |- /CS1
//      /SLTSL -| 4   3 |- /CS12
//       /RFSH -| 6   5 |- RSV(NC)
//        /INT -| 8   7 |- /WAIT
//     /BUSDIR -| 10  9 |- /M1
//       /MERQ -| 12 11 |- /IORQ
//         /RD -| 14 13 |- /WR
//     RSV(NC) -| 16 15 |- /RESET
//         A15 -| 18 17 |- A9
//         A10 -| 20 19 |- A11
//          A6 -| 22 21 |- A7
//          A8 -| 24 23 |- A12
//          A13-| 26 25 |- A14
//          A0 -| 28 27 |- A1
//          A2 -| 30 29 |- A3
//          A4 -| 32 31 |- A5
//          D0 -| 34 33 |- D1
//          D2 -| 36 35 |- D3
//          D4 -| 38 37 |- D5
//          D6 -| 40 39 |- D7
//       CLOCK -| 42 41 |- GND
//         SW1 -| 44 43 |- GND
//         SW2 -| 46 45 |- +5V
//        +12V -| 48 47 |- +5V
//        -12V -| 50 49 |- SOUNDIN
//              +-------+
//
//                                           BACK
//      +----------------------------------------------------------------------------+
//      | 49 47 45 43 41 39 37 35 33 31 29 27 25 23 21 19 17 15 13 11  9  7  5  3  1 |
// LEFT |                                                                            | RIGHT
//      | 50 48 46 44 42 40 38 36 34 32 30 28 26 24 22 20 18 16 14 12 10  8  6  4  2 |
//      +----------------------------------------------------------------------------+
//                                           FRONT
//
// CONTROL PINS:
// /RESET(PH0) - SNES RESET
// CLOCK(PH1)  - SNES CPUCLK
// /SLTSL(PH3) - SNES /CS
// /MERQ(PH4)  - SNES /IRQ
// /WR(PH5)    - SNES /WR
// /RD(PH6)    - SNES /RD
// /CS1(PL0)   - SNES A16
// /CS2(PL1)   - SNES A17
// /CS12(PL2)  - SNES A18

//******************************************
//  Defines
//******************************************
#define CS1_DISABLE PORTL |= (1 << 0)  // ROM SELECT 4000-7FFF
#define CS1_ENABLE PORTL &= ~(1 << 0)
#define CS2_DISABLE PORTL |= (1 << 1)  // ROM SELECT 8000-BFFF
#define CS2_ENABLE PORTL &= ~(1 << 1)
#define CS12_DISABLE PORTL |= (1 << 2)  // ROM SELECT 4000-BFFF
#define CS12_ENABLE PORTL &= ~(1 << 2)
#define MERQ_DISABLE PORTH |= (1 << 4)
#define MERQ_ENABLE PORTH &= ~(1 << 4)

//******************************************
//  Supported Mappers
//******************************************
// Supported Mapper Array
// Format = {msxmapper,msxlo,msxhi,msxramlo,msxramhi}
static const byte PROGMEM msxmapsize[] = {
  0, 1, 4, 0, 0,   // No Mapper
  1, 4, 7, 0, 2,   // ASCII8                                 [sram r/w] 2K (Dires), 8K (Xanadu/Wizardry)
  2, 4, 8, 0, 2,   // ASCII16                                [sram r/w] 2K (Daisenryaku/Hydlide 2), 8K (A-Train)
  3, 4, 4, 0, 0,   // Cross Blaim (db-Soft) 64K
  4, 5, 5, 2, 2,   // Game Master 2 128K                     [sram r/w] 8K
  5, 8, 8, 3, 3,   // Halnote (HAL) 1024K                    [sram r/w] 16K
  6, 4, 4, 0, 0,   // Harry Fox (Micro Cabin) 64K
  7, 6, 8, 2, 4,   // Koei                                   [sram r/w] 8K (Nobunaga no Yabou - Zenkoku Ban), 32K (Sangokushi II)
  8, 4, 8, 0, 0,   // Konami without SCC "Konami4"
  9, 5, 8, 0, 0,   // Konami with SCC "Konami5"
  10, 4, 4, 0, 0,  // MSX-DOS2 64K
  11, 0, 4, 2, 2,  // PAC 0K/FM-PAC 64K                     [sram r/w] 8K
  12, 7, 7, 0, 0,  // R-Type 384K
  13, 5, 5, 0, 0,  // Super Lode Runner (Irem) 128K
};

// MSX1 = 8,16,32,128,256
// MSX2 = 32,64,128,256,512,1024
int MSX[] = { 0, 8, 16, 32, 64, 128, 256, 512, 1024 };
byte msxlo = 0;  // Lowest Entry
byte msxhi = 8;  // Highest Entry

byte MSXRAM[] = { 0, 2, 8, 16, 32 };
byte msxramlo = 0;  // Lowest Entry
byte msxramhi = 4;  // Highest Entry

byte msxmapcount = 14;  // (sizeof(mapsize)/sizeof(mapsize[0])) / 5;
boolean msxmapfound = false;
byte msxmapselect;
int msxindex;

byte msxmapper;
byte msxsize;
byte msxramsize;
uint8_t msxbanks;
byte chipselect;

boolean srambit5 = false;
boolean srambit6 = false;
boolean srambit7 = false;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE
// 10 RAM SIZE

//******************************************
//  MENU
//******************************************
// Base Menu
static const char msxMenuItem2[] PROGMEM = "Read Cart";
static const char msxMenuItem3[] PROGMEM = "Set Mapper + Size";
static const char msxMenuItem4[] PROGMEM = "Write SRAM";
static const char* const menuOptionsMSX[] PROGMEM = { FSTRING_SELECT_CART, msxMenuItem2, msxMenuItem3, msxMenuItem4, FSTRING_RESET };

void msxMenu() {
  convertPgm(menuOptionsMSX, 5);
  uint8_t mainMenu = question_box(F("MSX MENU"), menuOptions, 5, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_MSX();
      setup_MSX();
      break;

    case 1:
      // Read ROM + Read RAM
      sd.chdir("/");
      readROM_MSX();
      readRAM_MSX();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper + Size
      setMapper_MSX();
      checkMapperSize_MSX();
      setROMSize_MSX();
      setRAMSize_MSX();
      break;

    case 3:
      // Write RAM
      writeRAM_MSX();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 4:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  SETUP
//******************************************
void setup_MSX() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // MSX uses A0-A15
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23 - Use A16-A18 for /CS1, /CS2, /CS12
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       /RST(PH0) CLOCK(PH1) /SLTSL(PH3) /MERQ(PH4) /WR(PH5)   /RD(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       /RST(PH0) CLOCK(PH1) /SLTSL(PH3) /MERQ(PH4) /WR(PH5)   /RD(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set /SLTSL(PH3) to LOW
  PORTH &= ~(1 << 3);

  // Set /CS1, /CS2, /CS12 to HIGH
  PORTL = 0xFF;  // A16-A23 (A16 = /CS1, A17 = /CS2, A18 = /CS12)

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_MSX();
  strcpy(romName, "MSX");

  mode = CORE_MSX;
}

//******************************************
// READ DATA
//******************************************
uint8_t readData_MSX(uint16_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;

  // Set /SLTSL(PH3) to LOW
  //  PORTH &= ~(1 << 3);

  // Set /RD to LOW
  PORTH &= ~(1 << 6);  // /RD LOW (ENABLE)
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // Pull /RD to HIGH
  PORTH |= (1 << 6);  // /RD HIGH (DISABLE)

  // Set /SLTSL(PH3) to HIGH
  //  PORTH |= (1 << 3);
  //  NOP; NOP;

  return ret;
}

void readSegment_MSX(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_MSX(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// WRITE DATA
//******************************************
void writeData_MSX(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;

  DDRC = 0xFF;  // Set to Output
  PORTC = data;
  NOP;
  NOP;
  NOP;

  // Set /WR(PH5) to LOW
  PORTH &= ~(1 << 5);
  NOP;
  NOP;
  NOP;

  // Set /WR(PH5) to HIGH
  PORTH |= (1 << 5);
  NOP;
  NOP;

  DDRC = 0x00;  // Reset to Input
}

//******************************************
// CS CODE
//******************************************
void setCS()  // Set CS Line
{
  chipselect = 0;
  for (int x = 0; x < 4; x++) {
    uint8_t check0 = readData_MSX(0x4000);
    uint8_t check1 = readData_MSX(0x4001);
    if ((check0 == 0x41) && (check1 == 0x42))
      break;
    chipselect++;
    enableCS();
  }
}

void checkCS()  // Check for 2nd Chip
{
  if (chipselect == 1) {
    uint8_t check0 = readData_MSX(0x8000);
    uint8_t check1 = readData_MSX(0x8001);
    if ((check0 == 0x41) && (check1 == 0x42)) {
      disableCS();
      chipselect = 2;
      enableCS();
    }
  }
}

void enableCS() {
  if (chipselect == 1)
    CS1_ENABLE;
  else if (chipselect == 2)
    CS2_ENABLE;
  else if (chipselect == 3)
    CS12_ENABLE;
}

void disableCS() {
  CS1_DISABLE;
  CS2_DISABLE;
  CS12_DISABLE;
}

//******************************************
// READ ROM
//******************************************
void readROM_MSX() {
  if (msxsize == 0) {
    display_Clear();
    println_Msg(F("ROM SIZE 0K"));
    display_Update();
  } else {
    createFolderAndOpenFile("MSX", "ROM", romName, "bin");

    switch (msxmapper) {
      case 0:  // No Mapper
        disableCS();
        if (msxsize == 4)                   // 64K
          readSegment_MSX(0x0000, 0x4000);  // +16K
        setCS();
        readSegment_MSX(0x4000, 0x6000);  // 8K
        if (msxsize > 1)
          readSegment_MSX(0x6000, 0x8000);  // +8K = 16K
        if (msxsize > 2) {
          checkCS();                        // Check for 2nd Chip
          readSegment_MSX(0x8000, 0xC000);  // +16K = 32K
        }
        disableCS();
        if (msxsize == 4)                    // 64K
          readSegment_MSX(0xC000, 0x10000);  // +16K
        break;

      case 1:  // ASCII8 (64K/128K/256K/512K)
      case 7:  // Koei (256K/512K/1024K)
        msxbanks = int_pow(2, msxsize - 1);
        for (int x = 0; x < msxbanks; x += 4) {
          writeData_MSX(0x6000, x);
          readSegment_MSX(0x4000, 0x6000);  // 8K Init Bank 0
          writeData_MSX(0x6800, x + 1);
          readSegment_MSX(0x6000, 0x8000);  // 8K Init Bank 0
          writeData_MSX(0x7000, x + 2);
          readSegment_MSX(0x8000, 0xA000);  // 8K Init Bank 0
          writeData_MSX(0x7800, x + 3);
          readSegment_MSX(0xA000, 0xC000);  // 8K Init Bank 0
        }
        break;

      case 2:  // ASCII16 (64K/128K/256K/512K)
        msxbanks = int_pow(2, msxsize - 1) / 2;
        for (int x = 0; x < msxbanks; x += 2) {
          writeData_MSX(0x6000, x);
          readSegment_MSX(0x4000, 0x8000);  // 16K Init Bank 0
          writeData_MSX(0x7000, x + 1);
          readSegment_MSX(0x8000, 0xC000);  // 16K Init Bank 0
        }
        break;

      case 3:  // Cross Blaim (64K)
        CS1_ENABLE;
        readSegment_MSX(0x4000, 0x8000);  // 16K Fixed Bank 0
        CS1_DISABLE;
        CS2_ENABLE;
        for (int x = 1; x < 4; x++) {
          writeData_MSX(0x4045, x);
          readSegment_MSX(0x8000, 0xC000);  // 16K Init Bank 1
        }
        CS2_DISABLE;
        break;

      case 4:                             // Game Master 2 (128K)
        readSegment_MSX(0x4000, 0x6000);  // 8K Fixed Bank 0
        writeData_MSX(0x6000, 1);         // Set Bank 1 for subsequent reads
        readSegment_MSX(0x6000, 0x8000);  // 8K Init Bank 1
        for (int x = 2; x < 16; x += 2) {
          writeData_MSX(0x8000, x);
          readSegment_MSX(0x8000, 0xA000);  // 8K
          writeData_MSX(0xA000, x + 1);
          readSegment_MSX(0xA000, 0xC000);  // 8K
        }
        break;

      case 5:  // HAL Note (1024K)
        MERQ_ENABLE;
        // Dummy Read - Needed to prevent random bytes
        for (int y = 0; y < 32; y++) {
          writeData_MSX(0x4FFF, y);
          for (uint32_t addr = 0x4000; addr < 0x6000; addr++)
            readData_MSX(addr);  // Dummy Read
        }
        // READ
        for (int x = 0; x < 128; x++) {
          writeData_MSX(0x4FFF, x);
          readSegment_MSX(0x4000, 0x6000);  // 8K Init Bank 0
        }
        MERQ_DISABLE;
        break;

      case 6:  // Harry Fox (64K)
        CS1_ENABLE;
        writeData_MSX(0x6000, 0);
        readSegment_MSX(0x4000, 0x8000);  // 16K Init Bank 0
        CS1_DISABLE;

        CS2_ENABLE;
        writeData_MSX(0x7000, 0);
        readSegment_MSX(0x8000, 0xC000);  // 16K Init Bank 1
        CS2_DISABLE;

        CS1_ENABLE;
        writeData_MSX(0x6000, 1);
        readSegment_MSX(0x4000, 0x8000);  // 16K
        CS1_DISABLE;

        CS2_ENABLE;
        writeData_MSX(0x7000, 1);
        readSegment_MSX(0x8000, 0xC000);  // 16K
        CS2_DISABLE;
        break;

      case 8:                             // Konami MegaROM without SCC
        readSegment_MSX(0x4000, 0x6000);  // 8K Fixed Bank 0
        readSegment_MSX(0x6000, 0x8000);  // 8K Init Bank 1
        msxbanks = int_pow(2, msxsize - 1);
        for (int x = 2; x < msxbanks; x += 2) {
          writeData_MSX(0x8000, x);
          readSegment_MSX(0x8000, 0xA000);  // 8K
          writeData_MSX(0xA000, x + 1);
          readSegment_MSX(0xA000, 0xC000);  // 8K
        }
        break;

      case 9:  // Konami MegaROM with SCC
        msxbanks = int_pow(2, msxsize - 1);
        for (int x = 0; x < msxbanks; x += 4) {
          writeData_MSX(0x5000, x);
          readSegment_MSX(0x4000, 0x6000);  // 8K
          writeData_MSX(0x7000, x + 1);
          readSegment_MSX(0x6000, 0x8000);  // 8K
          writeData_MSX(0x9000, x + 2);
          readSegment_MSX(0x8000, 0xA000);  // 8K
          writeData_MSX(0xB000, x + 3);
          readSegment_MSX(0xA000, 0xC000);  // 8K
        }
        break;

      case 10:  // MSX-DOS2 (64K)
        MERQ_ENABLE;
        CS1_ENABLE;
        for (int x = 0; x < 4; x++) {
          writeData_MSX(0x7FFE, x);         // Official v2.20
          readSegment_MSX(0x4000, 0x8000);  // 16K Init Bank 0
        }
        CS1_DISABLE;
        MERQ_DISABLE;
        break;

      case 11:  // FM-PAC (64K)
        CS1_ENABLE;
        for (int x = 0; x < 4; x++) {
          writeData_MSX(0x7FF7, x);
          readSegment_MSX(0x4000, 0x8000);  // 16K
        }
        CS1_DISABLE;
        break;

      case 12:  // R-TYPE (384K)
        for (int x = 0; x < 23; x++) {
          writeData_MSX(0x7000, x);
          readSegment_MSX(0x8000, 0xC000);  // 16K Init Bank 0
        }
        readSegment_MSX(0x4000, 0x8000);  // 16K Init Bank 0F
        break;

      case 13:  // Super Lode Runner (128K)
        MERQ_ENABLE;
        CS2_ENABLE;
        for (int x = 0; x < 8; x++) {
          writeData_MSX(0x0000, x);
          readSegment_MSX(0x8000, 0xC000);  // 16K Init Bank 0
        }
        CS2_DISABLE;
        MERQ_DISABLE;
        break;
    }
    myFile.close();

    printCRC(fileName, NULL, 0);

    println_Msg(FS(FSTRING_EMPTY));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  }
}

//******************************************
// TEST/CHECK RAM
//******************************************
boolean testRAM(byte enable1, byte enable2) {
  boolean testbit = false;
  for (int x = 0; x < 0x10; x++) {  // Test 16 Bytes
    writeData_MSX(0x7000, enable1);
    byte test1 = readData_MSX(0x8000 + x);
    writeData_MSX(0x7000, enable2);
    byte test2 = readData_MSX(0x8000 + x);
    if (test1 == test2)
      testbit = true;
    else {
      testbit = false;
      break;
    }
  }
  return testbit;
}

void checkRAM() {
  // Test carts to identify SRAM Enable Bits (Bit 5/6/7)
  // Bit 5 Test
  srambit5 = testRAM(0x20, 0xB0);
  if (!srambit5) {
    // Bit 6 Test
    srambit6 = testRAM(0x40, 0xC0);
    if (!srambit6) {
      // Bit 7 Test
      srambit7 = testRAM(0x80, 0xF0);
      if (!srambit7) {
        display_Clear();
        print_Error(F("SRAM FAILED - CHECK BATTERY"));
      }
    }
  }
}

//******************************************
// READ RAM
//******************************************
void readRAM_MSX() {
  if (msxramsize == 0) {
    display_Clear();
    println_Msg(F("RAM SIZE 0K"));
    display_Update();
  } else {
    strcpy(fileName, romName);
    strcat(fileName, ".srm");

    if (msxsize == 0) {
      // create a new folder for storing ram file
      EEPROM_readAnything(0, foldern);
      sprintf(folder, "MSX/RAM/%d", foldern);
      sd.mkdir(folder, true);
      sd.chdir(folder);
    }

    display_Clear();
    print_STR(saving_to_STR, 0);
    print_Msg(folder);
    println_Msg(F("/..."));
    display_Update();

    // open file on sdcard
    if (!myFile.open(fileName, O_RDWR | O_CREAT))
      print_FatalError(sd_error_STR);

    if (msxsize == 0) {
      // write new folder number back to EEPROM
      foldern++;
      EEPROM_writeAnything(0, foldern);
    }

    switch (msxmapper) {
      case 1:  // ASCII8 (2K/8K)
        // ASCII8 carts use different SRAM Enable Bits
        // Bit 4 (0x10) - Dires (2K)
        // Bit 5 (0x20) - Xanadu
        // Bit 7 (0x80) - Wizardry
        if (msxramsize == 1) {              // Dires (2K)
          writeData_MSX(0x7000, 0x10);      // Bit 4
          readSegment_MSX(0x8000, 0x8800);  // 2K
        } else {
          // Combine SRAM Enable Bit 5 + Bit 7 = 0xA0
          writeData_MSX(0x7000, 0xA0);      // Bit 5 + Bit 7
          readSegment_MSX(0x8000, 0xA000);  // 8K
        }
        writeData_MSX(0x7000, 0);  // SRAM Disable
        break;

      case 2:                               // ASCII16 (2K/8K)
        writeData_MSX(0x7000, 0x10);        // Bit 4 Enable
        readSegment_MSX(0x8000, 0x8800);    // 2K - Hydlide 2 & Daisenryaku (2K)
        if (msxramsize == 2)                // A-Train (8K)
          readSegment_MSX(0x8800, 0xA000);  // +6K = 8K
        writeData_MSX(0x7000, 0);           // SRAM Disable
        break;

      case 4:                             // Game Master 2 (8K)
        writeData_MSX(0xA000, 0x10);      // Bit 4 Enable, Bit 5 SRAM Segment 0
        readSegment_MSX(0xB000, 0xC000);  // 4K
        writeData_MSX(0xA000, 0x30);      // Bit 4 Enable, Bit 5 SRAM Segment 1
        readSegment_MSX(0xB000, 0xC000);  // 4K
        writeData_MSX(0xA000, 0);         // SRAM Disable
        break;

      case 5:  // HAL Note (16K)
        MERQ_ENABLE;
        writeData_MSX(0x4FFF, 0x80);      // Bit 7 Enable
        readSegment_MSX(0x0000, 0x4000);  // 16K
        writeData_MSX(0x4FFF, 0);         // SRAM Disable
        MERQ_DISABLE;
        break;

      case 7:  // Koei (8K/32K) Nobunaga no Yabou - Zenkoku Ban (8K) & Sangokushi II (32K)
        // Koei carts use different SRAM Enable Bits
        // Bit 5 (0x20) - Nobunaga no Yabou - Zenkoku Ban MSX
        // Bit 6 (0x40) - Nobunaga no Yabou - Zenkoku Ban MSX2
        // Bit 7 (0x80) - Sangokushi II
        // Use Combined Bits:  0xA0 (Bit 5 + Bit 7) and 0xC0 (Bit 6 + Bit 7)
        // Note:  Combined 0xE0 (Bit 5 + Bit 6 + Bit 7) does not work
        checkRAM();
        if (srambit6)
          writeData_MSX(0x7000, 0xC0);  // Bit 6 + Bit 7 Enable
        else
          writeData_MSX(0x7000, 0xA0);    // Bit 5 + Bit 7 Enable
        readSegment_MSX(0x8000, 0xA000);  // 8K
        if (msxramsize > 2) {
          for (int x = 1; x < 4; x++) {
            if (srambit6)
              writeData_MSX(0x7000, 0xC0 + x);  // Bit 6 + Bit 7 Enable
            else
              writeData_MSX(0x7000, 0xA0 + x);  // Bit 5 + Bit 7 Enable
            readSegment_MSX(0x8000, 0xA000);    // 8K
          }
        }
        writeData_MSX(0x7000, 0);  // SRAM Disable
        break;

      case 11:                            // PAC/FM-PAC (8K)
        writeData_MSX(0x5FFE, 0x4D);      // SRAM Enable Step 1
        writeData_MSX(0x5FFF, 0x69);      // SRAM Enable Step 2
        readSegment_MSX(0x4000, 0x6000);  // 8K
        writeData_MSX(0x5FFE, 0);         // SRAM Disable
        break;
    }
    myFile.close();

    printCRC(fileName, NULL, 0);

    println_Msg(FS(FSTRING_EMPTY));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  }
}

//******************************************
// WRITE RAM
//******************************************
void writeRAM_MSX() {
  display_Clear();

  if (msxramsize == 0) {
    print_Error(F("RAM SIZE 0K"));
  } else {
    fileBrowser(F("Select RAM File"));
    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);
    display_Clear();
    println_Msg(F("Writing File: "));
    println_Msg(filePath);
    println_Msg(fileName);
    display_Update();

    //open file on sd card
    if (myFile.open(filePath, O_READ)) {

      switch (msxmapper) {
        case 1:                                                                                    // ASCII8 (2K/8K)
          for (word address = 0x0; address < (0x800 * msxramsize * msxramsize); address += 512) {  // 2K/8K
            if (msxramsize == 1)
              writeData_MSX(0x7000, 0x10);  // Bit 4
            else
              writeData_MSX(0x7000, 0xA0);  // Bit 5 + Bit 7
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              writeData_MSX(0x8000 + address + x, sdBuffer[x]);
            }
          }
          writeData_MSX(0x7000, 0);  // SRAM Disable
          break;

        case 2:                                                                                    // ASCII16 (2K/8K)
          writeData_MSX(0x7000, 0x10);                                                             // Bit 4 Enable
          for (word address = 0x0; address < (0x800 * msxramsize * msxramsize); address += 512) {  // 2K/8K
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              writeData_MSX(0x8000 + address + x, sdBuffer[x]);
            }
          }
          writeData_MSX(0x7000, 0);  // SRAM Disable
          break;

        case 4:  // Game Master 2 (8K)
          for (int y = 0; y < 2; y++) {
            writeData_MSX(0xA000, 0x10 + (y * 0x20));                     // Bit 4 Enable, Bit 5 SRAM Segment 0/1
            for (word address = 0x0; address < 0x1000; address += 512) {  // 4K
              myFile.read(sdBuffer, 512);
              for (int x = 0; x < 512; x++) {
                writeData_MSX(0xB000 + address + x, sdBuffer[x]);
              }
            }
          }
          writeData_MSX(0xA000, 0);  // SRAM Disable
          break;

        case 5:  // HAL Note (16K)
          MERQ_ENABLE;
          writeData_MSX(0x4FFF, 0x80);                                // Bit 7 Enable
          for (word address = 0; address < 0x4000; address += 512) {  // 16K
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              writeData_MSX(address + x, sdBuffer[x]);
            }
          }
          writeData_MSX(0x4FFF, 0);  // SRAM Disable
          MERQ_DISABLE;
          break;

        case 7:  // Koei (8K/32K)
          checkRAM();
          if (srambit6)
            writeData_MSX(0x7000, 0xC0);  // Bit 6 + Bit 7 Enable
          else
            writeData_MSX(0x7000, 0xA0);                                // Bit 5 + Bit 7 Enable
          for (word address = 0x0; address < 0x2000; address += 512) {  // 8K
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              writeData_MSX(0x8000 + address + x, sdBuffer[x]);
            }
          }
          if (msxramsize > 2) {
            for (int y = 1; y < 4; y++) {
              if (srambit6)
                writeData_MSX(0x7000, 0xC0 + y);  // Bit 6 + Bit 7 Enable
              else
                writeData_MSX(0x7000, 0xA0 + y);                            // Bit 5 + Bit 7 Enable
              for (word address = 0x0; address < 0x2000; address += 512) {  // 8K
                myFile.read(sdBuffer, 512);
                for (int x = 0; x < 512; x++) {
                  writeData_MSX(0x8000 + address + x, sdBuffer[x]);
                }
              }
            }
          }
          writeData_MSX(0x7000, 0);  // SRAM Disable
          break;

        case 11:                                                        // PAC/FM-PAC (8K)
          writeData_MSX(0x5FFE, 0x4D);                                  // SRAM Enable Step 1
          writeData_MSX(0x5FFF, 0x69);                                  // SRAM Enable Step 2
          for (word address = 0x0; address < 0x2000; address += 512) {  // 8K
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              writeData_MSX(0x4000 + address + x, sdBuffer[x]);
            }
          }
          writeData_MSX(0x5FFE, 0);  // SRAM Disable
          break;
      }
      myFile.close();

      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("RAM FILE WRITTEN!"));
      display_Update();

    } else {
      print_FatalError(sd_error_STR);
    }
  }
  sd.chdir();          // root
  filePath[0] = '\0';  // Reset filePath
}

//******************************************
// MAPPER CODE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printMapperSelection_MSX(int index) {
  display_Clear();
  print_Msg(FS(FSTRING_MAPPER));
  msxindex = index * 5;
  msxmapselect = pgm_read_byte(msxmapsize + msxindex);
  println_Msg(msxmapselect);
  printMapper(msxmapselect);
}
#endif

void setMapper_MSX() {
  byte newmsxmapper;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  navigateMenu(0, msxmapcount - 1, &printMapperSelection_MSX);
  newmsxmapper = msxmapselect;
  
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(newmsxmapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  msxmapfound = false;
  printMapper(0);
  Serial.print(F("Enter Mapper [0-13]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newmsxmapper = newmap.toInt();
  for (int i = 0; i < msxmapcount; i++) {
    msxindex = i * 5;
    msxmapselect = pgm_read_byte(msxmapsize + msxindex);
    if (newmsxmapper == msxmapselect)
      msxmapfound = true;
  }
  if (msxmapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(FS(FSTRING_EMPTY));
    newmsxmapper = 0;
    goto setmapper;
  }
#endif
  EEPROM_writeAnything(7, newmsxmapper);
  msxmapper = newmsxmapper;
}

void checkMapperSize_MSX() {
  for (int i = 0; i < msxmapcount; i++) {
    msxindex = i * 5;
    byte mapcheck = pgm_read_byte(msxmapsize + msxindex);
    if (mapcheck == msxmapper) {
      msxlo = pgm_read_byte(msxmapsize + msxindex + 1);
      msxhi = pgm_read_byte(msxmapsize + msxindex + 2);
      msxramlo = pgm_read_byte(msxmapsize + msxindex + 3);
      msxramhi = pgm_read_byte(msxmapsize + msxindex + 4);
      break;
    }
  }
}

//******************************************
// SET ROM SIZE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_MSX(uint8_t index) {
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(MSX[index]);
}
#endif

void setROMSize_MSX() {
  byte newmsxsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (msxlo == msxhi)
    newmsxsize = msxlo;
  else {
    uint8_t b = 0;
    int i = msxlo;

    printRomSize_MSX(i);
    printInstructions();

    while (1) {
      b = checkButton();
      if (b == 2) {             // Previous (doubleclick)
        if (msxmapper == 11) {  // PAC/FM-PAC 0K/64K
          if (i == msxlo)
            i = msxhi;  // 64K
          else
            i = msxlo;  // 0K
        } else {
          if (i == msxlo)
            i = msxhi;
          else
            i--;

          // Only update display after input because of slow LCD library
          printRomSize_MSX(i);
          printInstructions();
        }
      }
      if (b == 1) {             // Next (press)
        if (msxmapper == 11) {  // PAC/FM-PAC 0K/64K
          if (i == msxlo)
            i = msxhi;  // 64K
          else
            i = msxlo;  // 0K
        } else {
          if (i == msxhi)
            i = msxlo;
          else
            i++;

          // Only update display after input because of slow LCD library
          printRomSize_MSX(i);
          printInstructions();
        }
      }
      if (b == 3) {  // Long Press - Execute (hold)
        newmsxsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  if (msxmapper == 12)  // R-Type
    print_Msg(F("384"));
  else
    print_Msg(MSX[newmsxsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (msxlo == msxhi)
    newmsxsize = msxlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (msxhi - msxlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      if (msxmapper == 12)  // R-Type
        Serial.print(F("384"));
      else
        Serial.print(MSX[i + msxlo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newmsxsize = sizeROM.toInt() + msxlo;
    if (msxmapper == 11) {  // PAC/FM-PAC 0K/64K
      if ((newmsxromsize > 0) && (newmsxromsize < 4)) {
        Serial.println(F("SIZE NOT SUPPORTED"));
        Serial.println(FS(FSTRING_EMPTY));
        goto setrom;
      }
    }
    if (newmsxsize > msxhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  if (msxmapper == 12)  // R-Type
    Serial.print(F("384"));
  else
    Serial.print(MSX[newmsxsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newmsxsize);
  msxsize = newmsxsize;
}

//******************************************
// SET RAM SIZE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRamSize_MSX(uint8_t index) {
    display_Clear();
    print_Msg(F("RAM Size: "));
    println_Msg(MSXRAM[index]);
}
#endif

void setRAMSize_MSX() {
  byte newmsxramsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (msxramlo == msxramhi)
    newmsxramsize = msxramlo;
  else {
    uint8_t b = 0;
    int i = msxramlo;

    printRamSize_MSX(i);
    printInstructions();

    while (1) {
      b = checkButton();
      if (b == 2) {            // Previous (doubleclick)
        if (msxmapper == 7) {  // Koei 8K/32K
          if (i == msxramlo)
            i = msxramhi;  // 32K
          else
            i = msxramlo;  // 8K
        } else {
          if (i == msxramlo)
            i = msxramhi;
          else
            i--;

          // Only update display after input because of slow LCD library
          printRamSize_MSX(i);
          printInstructions();
        }
      }
      if (b == 1) {            // Next (press)
        if (msxmapper == 7) {  // Koei 8K/32K
          if (i == msxramlo)
            i = msxramhi;  // 32K
          else
            i = msxramlo;  // 8K
        } else {
          if (i == msxramhi)
            i = msxramlo;
          else
            i++;

          // Only update display after input because of slow LCD library
          printRamSize_MSX(i);
          printInstructions();
        }
      }
      if (b == 3) {  // Long Press - Execute (hold)
        newmsxramsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("RAM SIZE "));
  print_Msg(MSXRAM[newmsxramsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (msxramlo == msxramhi)
    newmsxramsize = msxramlo;
  else {
setram:
    String sizeRAM;
    for (int i = 0; i < (msxramhi - msxramlo + 1); i++) {
      Serial.print(F("Select RAM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(MSXRAM[i + msxramlo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter RAM Size: "));
    while (Serial.available() == 0) {}
    sizeRAM = Serial.readStringUntil('\n');
    Serial.println(sizeRAM);
    newmsxramsize = sizeRAM.toInt() + msxramlo;
    if (msxmapper == 7) {        // Koei 8K/32K
      if (newmsxramsize == 3) {  // 16K
        Serial.println(F("SIZE NOT SUPPORTED"));
        Serial.println(FS(FSTRING_EMPTY));
        goto setram;
      }
    }
    if (newmsxramsize > msxramhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setram;
    }
  }
  Serial.print(F("RAM Size = "));
  Serial.print(MSXRAM[newmsxramsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(10, newmsxramsize);
  msxramsize = newmsxramsize;
}

//******************************************
// CHECK STATUS
//******************************************
void checkStatus_MSX() {
  EEPROM_readAnything(7, msxmapper);
  EEPROM_readAnything(8, msxsize);
  EEPROM_readAnything(10, msxramsize);
  if (msxmapper > 13) {
    msxmapper = 0;
    EEPROM_writeAnything(7, msxmapper);
  }
  if (msxsize > 8) {
    msxsize = 0;
    EEPROM_writeAnything(8, msxsize);
  }
  if (msxramsize > 4) {
    msxramsize = 0;
    EEPROM_writeAnything(10, msxramsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("MSX READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Msg(msxmapper);
  printMapper(msxmapper);
  print_Msg(FS(FSTRING_ROM_SIZE));
  if (msxmapper == 12)  // R-Type
    print_Msg(F("384"));
  else
    print_Msg(MSX[msxsize]);
  println_Msg(F("K"));
  print_Msg(F("RAM SIZE: "));
  print_Msg(MSXRAM[msxramsize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:   "));
  Serial.println(msxmapper);
  Serial.print(F("CURRENT ROM SIZE: "));
  if (msxmapper == 12)  // R-Type
    Serial.print(F("384"));
  else
    Serial.print(MSX[msxsize]);
  Serial.println(F("K"));
  Serial.print(F("CURRENT RAM SIZE: "));
  Serial.print(MSXRAM[msxramsize]);
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

void printMapper(byte msxmaplabel) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  switch (msxmaplabel) {
    case 0:
      println_Msg(F("NONE"));
      break;
    case 1:
      println_Msg(F("ASCII8"));
      break;
    case 2:
      println_Msg(F("ASCII16"));
      break;
    case 3:
      println_Msg(F("CROSS BLAIM"));
      break;
    case 4:
      println_Msg(F("GAME MASTER 2"));
      break;
    case 5:
      println_Msg(F("HAL NOTE"));
      break;
    case 6:
      println_Msg(F("HARRY FOX YUKI"));
      break;
    case 7:
      println_Msg(F("KOEI"));
      break;
    case 8:
      println_Msg(F("KONAMI"));
      break;
    case 9:
      println_Msg(F("KONAMI SCC"));
      break;
    case 10:
      println_Msg(F("MSX-DOS2"));
      break;
    case 11:
      println_Msg(F("PAC/FM-PAC"));
      break;
    case 12:
      println_Msg(F("R-TYPE"));
      break;
    case 13:
      println_Msg(F("SUPER LODE RUNNER"));
      break;
  }
#else
  Serial.println(F("0 = NONE"));
  Serial.println(F("1 = ASCII8"));
  Serial.println(F("2 = ASCII16"));
  Serial.println(F("3 = CROSS BLAIM"));
  Serial.println(F("4 = GAME MASTER 2"));
  Serial.println(F("5 = HAL NOTE"));
  Serial.println(F("6 = HARRY FOX YUKI"));
  Serial.println(F("7 = KOEI"));
  Serial.println(F("8 = KONAMI"));
  Serial.println(F("9 = KONAMI SCC"));
  Serial.println(F("10 = MSX-DOS2"));
  Serial.println(F("11 = PAC/FM-PAC"));
  Serial.println(F("12 = R-TYPE"));
  Serial.println(F("13 = SUPER LODE RUNNER"));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
struct database_entry_MSX {
  byte gameMapper;
  byte gameSize;
  byte ramSize;
};

void readDataLine_MSX(FsFile& database, void* entry) {
  database_entry_MSX* castEntry = (database_entry_MSX*)entry;
  // Read mapper
  castEntry->gameMapper = ((database.read() - 48) * 10) + (database.read() - 48);

  // Skip over semicolon
  database.seekCur(1);

  // Read rom size
  castEntry->gameSize = database.read() - 48;

  // Skip over semicolon
  database.seekCur(1);

  // Read ram size
  castEntry->ramSize = database.read() - 48;

  // Skip rest of line
  database.seekCur(2);
}

void setCart_MSX() {
  //go to root
  sd.chdir();

  struct database_entry_MSX entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("msxcart.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLine_MSX, &entry)) {
      EEPROM_writeAnything(7, entry.gameMapper);
      EEPROM_writeAnything(8, entry.gameSize);
      EEPROM_writeAnything(10, entry.ramSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
