//******************************************
// COMMODORE 64 MODULE
//******************************************
#ifdef ENABLE_C64
// Commodore 64
// Cartridge Pinout
// 44P 2.54mm pitch connector
//
//   FRONT             BACK
//    SIDE             SIDE
//          +-------+
//     GND -| 1   A |- GND
//   +5VDC -| 2   B |- /ROMH
//   +5VDC -| 3   C |- /RESET
//    /IRQ -| 4   D |- /NMI
//     R/W -| 5   E |- PHI2
//  DOTCLK -| 6   F |- A15
//    /IO1 -| 7   H |- A14
//   /GAME -| 8   J |- A13
//  /EXROM -| 9   K |- A12
//    /IO2 -| 10  L |- A11
//   /ROML -| 11  M |- A10
//      BA -| 12  N |- A9
//    /DMA -| 13  P |- A8
//      D7 -| 14  R |- A7
//      D6 -| 15  S |- A6
//      D5 -| 16  T |- A5
//      D4 -| 17  U |- A4
//      D3 -| 18  V |- A3
//      D2 -| 19  W |- A2
//      D1 -| 20  X |- A1
//      D0 -| 21  Y |- A0
//     GND -| 22  Z |- GND
//          +-------+
//
//                                                 TOP
//       +---------------------------------------------------------------------------------------+
//       | 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22 |
// LEFT  |                                                                                       | RIGHT
//       | A   B   C   D   E   F   H   J   K   L   M   N   P   R   S   T   U   V   W   X   Y   Z |
//       +---------------------------------------------------------------------------------------+
//                                                BOTTOM
//
// CONTROL PINS:
// /RESET(PH0) - SNES RESET
// PHI2(PH1)   - SNES CPUCLK
// /GAME(PH3)  - SNES /CS
// /EXROM(PH4) - SNES /IRQ
// R/W(PH6)    - SNES /RD
// /ROML(PL0)  - SNES A16
// /ROMH(PL1)  - SNES A17
// /IO1(PL2)   - SNES A18
// /IO2(PL3)   - SNES A19

//******************************************
//  Defines
//******************************************
#define PHI2_ENABLE PORTH |= (1 << 1)
#define PHI2_DISABLE PORTH &= ~(1 << 1)
#define GAME_DISABLE PORTH |= (1 << 3)
#define GAME_ENABLE PORTH &= ~(1 << 3)
#define EXROM_DISABLE PORTH |= (1 << 4)
#define EXROM_ENABLE PORTH &= ~(1 << 4)
#define ROML_DISABLE PORTL |= (1 << 0)
#define ROML_ENABLE PORTL &= ~(1 << 0)
#define ROMH_DISABLE PORTL |= (1 << 1)
#define ROMH_ENABLE PORTL &= ~(1 << 1)
#define IO1_DISABLE PORTL |= (1 << 2)
#define IO1_ENABLE PORTL &= ~(1 << 2)
#define IO2_DISABLE PORTL |= (1 << 3)
#define IO2_ENABLE PORTL &= ~(1 << 3)

//******************************************
//  Supported Mappers
//******************************************
// Supported Mapper Array
// Format = {c64mapper,c64lo,c64hi}
static const byte PROGMEM c64mapsize[] = {
  0, 0, 3,   // Normal 4K/8K/16K + Ultimax 8K/16K
  1, 5, 5,   // Action Replay 32K                               [UNTESTED]
  2, 3, 3,   // KCS Power Cartridge 16K                         [UNTESTED]
  3, 6, 6,   // Final Cartridge III 64K                         [UNTESTED]
  4, 3, 3,   // Simons Basic 16K                                [UNTESTED]
  5, 7, 9,   // Ocean 128K/256K/512K
  6, 2, 2,   // Expert Cartridge 8K                             [UNTESTED]
  7, 7, 7,   // Fun Play, Power Play 128K                       [UNTESTED]
  8, 6, 6,   // Super Games 64K                                 [UNTESTED]
  9, 5, 5,   // Atomic Power 32K                                [UNTESTED]
  10, 2, 2,  // Epyx Fastload 8K                               [UNTESTED]
  11, 3, 3,  // Westermann Learning 16K                        [UNTESTED]
  12, 1, 1,  // Rex Utility 8K                                 [UNTESTED]
  13, 3, 3,  // Final Cartridge I 16K                          [UNTESTED]
  14, 6, 6,  // Magic Formel 64K                               [UNTESTED]
  15, 9, 9,  // C64 Game System, System 3 512K                 [UNTESTED]
  16, 3, 3,  // WarpSpeed 16K                                  [UNTESTED]
  17, 7, 7,  // Dinamic 128K                                   [UNTESTED]
  18, 4, 4,  // Zaxxon, Super Zaxxon (SEGA) 20K                [UNTESTED]
  19, 5, 7,  // Magic Desk, Domark, HES Australia 32K/64K/128K [UNTESTED]
  20, 6, 6,  // Super Snapshot 5 64K                           [UNTESTED]
  21, 6, 6,  // Comal-80 64K                                   [UNTESTED]
};

int C64[] = { 4, 8, 12, 16, 20, 32, 64, 128, 256, 512 };
byte c64lo = 0;  // Lowest Entry
byte c64hi = 9;  // Highest Entry

byte c64mapcount = 22;  // (sizeof(mapsize)/sizeof(mapsize[0])) / 3;
boolean c64mapfound = false;
byte c64mapselect;
int c64index;

byte c64mapper;
byte newc64mapper;
byte c64size;
byte newc64size;
uint8_t c64banks;
byte c64port;  // exrom+game
byte newc64port;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE
// 12 PORT STATE - EXROM/GAME

//******************************************
//  MENU
//******************************************
// Base Menu
static const char* const menuOptionsC64[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void c64Menu() {
  convertPgm(menuOptionsC64, 4);
  uint8_t mainMenu = question_box(F("C64 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_C64();
      wait();
      setup_C64();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_C64();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper + Size + Ports
      setMapper_C64();
      checkMapperSize_C64();
      setROMSize_C64();
      setPorts_C64();
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  SETUP
//******************************************
void setup_C64() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // C64 uses A0-A15
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23 - Use A16-A19 for /ROML, /ROMH, /IO1, /IO2
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       /RST(PH0)  PHI2(PH1) /GAME(PH3) /EXROM(PH4) ---(PH5)   R/W(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       /RST(PH0)  PHI2(PH1) /GAME(PH3) /EXROM(PH4) ---(PH5)   R/W(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set /GAME(PH3) and /EXROM(PH4) to LOW
  PORTH &= ~(1 << 3) & ~(1 << 4);

  // Set /ROML, /ROMH, /IO1, /IO2 to HIGH
  PORTL = 0xFF;  // A16-A23 (A16 = /ROML, A17 = /ROMH, A18 = /IO1, A19 = /IO2)

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_C64();
  strcpy(romName, "C64");

  mode = CORE_C64;
}

//******************************************
// READ DATA
//******************************************
uint8_t readData_C64(uint16_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;

  // Set /GAME(PH3) + /EXROM(PH4) to LOW
  //  PORTH &= ~(1 << 3) & ~(1 << 4);

  // Set R/W(PH6) to HIGH
  PORTH |= (1 << 6);  // R/W HIGH (READ)
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // Set /GAME(PH3) + /EXROM(PH4) to HIGH
  //  PORTH |= (1 << 3) | (1 << 4);
  //  NOP; NOP;

  return ret;
}

void readSegment_C64(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_C64(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readSegmentSmall_C64(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 256) {
    for (int w = 0; w < 256; w++) {
      uint8_t temp = readData_C64(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 256);
  }
}

//******************************************
// WRITE DATA
//******************************************
void writeData_C64(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;

  DDRC = 0xFF;  // Set to Output
  PORTC = data;
  NOP;
  NOP;
  NOP;

  // Set R/W(PH6) to LOW
  PORTH &= ~(1 << 6);  // R/W LOW (WRITE)
  NOP;
  NOP;
  NOP;

  // Set R/W(PH6) to HIGH
  PORTH |= (1 << 6);
  NOP;
  NOP;

  DDRC = 0x00;  // Reset to Input
}

void bankSwitch_C64(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;
  NOP;

  DDRC = 0xFF;  // Set to Output
  PORTC = data;
  NOP;
  NOP;
  NOP;

  // Latch Bank Data
  PHI2_DISABLE;                      // PHI2 LOW
  if (((addr >> 8) & 0xFF) == 0xDF)  // 0xDFxx
    IO2_ENABLE;
  else if (((addr >> 8) & 0xFF) == 0xDE)  // 0xDExx
    IO1_ENABLE;
  PORTH &= ~(1 << 6);  // R/W LOW (WRITE)
  NOP;
  NOP;
  NOP;
  PORTH |= (1 << 6);  // R/W HIGH (READ)
  PHI2_ENABLE;        // PHI2 HIGH
  IO2_DISABLE;
  IO1_DISABLE;

  DDRC = 0x00;  // Reset to Input
}

//******************************************
// PORT STATE
//******************************************
void enablePorts_C64() {
  if (c64port == 0) {  // 0 = 00 = EXROM LOW/GAME LOW
    EXROM_ENABLE;
    GAME_ENABLE;
  } else if (c64port == 1) {  // 1 = 01 = EXROM LOW/GAME HIGH
    EXROM_ENABLE;
    GAME_DISABLE;
  } else if (c64port == 2) {  // 2 = 10 = EXROM HIGH/GAME LOW
    EXROM_DISABLE;
    GAME_ENABLE;
  } else {  // c64port == 3 = 11 = EXROM HIGH/GAME HIGH
    EXROM_DISABLE;
    GAME_DISABLE;
  }
}

void disablePorts_C64() {  // EXROM HIGH/GAME HIGH
  EXROM_DISABLE;
  GAME_DISABLE;
}

//******************************************
// READ ROM
//******************************************
// ADDRESS RANGES
// $8000-$9FFF/$A0000-$BFFF/$E000-$FFFF
// NORMAL (EXROM LOW/GAME LOW): ROML = $8000, ROMH = $A000
// ULTIMAX (EXROM HIGH/GAME LOW/): ROML = $8000, ROMH = $E000
// GAME HIGH/EXROM LOW: ROML = $8000

void readROM_C64() {
  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "C64/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(sd_error_STR);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  switch (c64mapper) {
    case 0:  // Normal (4K/8K/16K) & Ultimax (8K/16K)
      // ULTIMAX CARTS
      if (c64port == 2) {   // 2 = 10 = EXROM HIGH/GAME LOW
        GAME_ENABLE;        // LOW
        EXROM_DISABLE;      // HIGH
        if (c64size > 1) {  // 16K [NO ROML FOR 8K]
          ROML_ENABLE;
          readSegment_C64(0x8000, 0xA000);  // 8K
          ROML_DISABLE;
        }
        ROMH_ENABLE;
        readSegment_C64(0xE000, 0x10000);  // +8K = 8K/16K
        ROMH_DISABLE;
      } else {              // NORMAL CARTS
        enablePorts_C64();  // MOST CARTS EXROM LOW/GAME LOW BUT VARIATIONS EXIST
        ROML_ENABLE;
        readSegment_C64(0x8000, 0x9000);  // 4K
        if (c64size > 0)
          readSegment_C64(0x9000, 0xA000);  // +4K = 8K
        ROML_DISABLE;
        if (c64size > 1) {
          ROMH_ENABLE;
          readSegment_C64(0xA000, 0xC000);  // +8K = 16K
          ROMH_DISABLE;
        }
      }
      disablePorts_C64();
      break;

    case 1:          // Action Replay (32K)
      GAME_DISABLE;  // HIGH
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      for (int x = 0; x < 4; x++) {
        bankSwitch_C64(0xDE00, x << 3);   // Switch Bank using D3-D4
        readSegment_C64(0x8000, 0xA000);  // 8K *4 = 32K
      }
      ROML_DISABLE;
      disablePorts_C64();
      break;

    case 2:          // KCS Power Cartridge (16K)
    case 11:         // Westermann Learning (16K)
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      readSegment_C64(0x8000, 0xA000);  // 8K
      ROML_DISABLE;
      ROMH_ENABLE;
      readSegment_C64(0xA000, 0xC000);  // +8K = 16K
      ROMH_DISABLE;
      disablePorts_C64();
      break;

    case 3:           // Final Cartridge III (64K)
      GAME_DISABLE;   // HIGH
      EXROM_DISABLE;  // HIGH
      for (int x = 0; x < 4; x++) {
        bankSwitch_C64(0xDFFF, 0x40 + x);  // Switch Bank using $DFFF
        ROML_ENABLE;
        readSegment_C64(0x8000, 0xA000);  // 8K
        ROML_DISABLE;
        ROMH_ENABLE;
        readSegment_C64(0xA000, 0xC000);  // +8K = 16K
        ROMH_DISABLE;
      }
      break;

    case 4:          // Simons Basic (16K)
      GAME_DISABLE;  // HIGH
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      readSegment_C64(0x8000, 0xA000);  // 8K
      ROML_DISABLE;
      ROMH_ENABLE;
      bankSwitch_C64(0xDE00, 0x1);      // Switch Bank to ROM
      readSegment_C64(0xA000, 0xC000);  // +8K = 16K
      ROMH_DISABLE;
      disablePorts_C64();
      break;

    // Ocean Bank 1/B (Single Chip) Selection Notes (Luigi Di Fraia):
    // 128 KiB cartridges (all known titles): bits 0-3 at $DE00 and /ROML (single 128 KiB chip with A16 on pin 22, rather than the /OE signal)
    // 256 KiB cartridges (just "Chase H.Q. II"): bits 0-4 at $DE00 and /ROML (single 256 KiB chip)
    // 512 KiB cartridges (just "Terminator 2"):  bits 0-5 at $DE00 and /ROML (single 512 KiB chip)

    // Ocean 256K ROM Start Data
    // Single Chip:
    // Chase HQ II          09 80 63 80 C3 C2 CD 38 30 4C 63 80 4C C8 80 80
    // Space Gun            09 80 09 80 C3 C2 CD 38 30 78 A2 FF 9A A9 E7 85
    // Two Chip:
    // Robocop 2            09 80 75 80 C3 C2 CD 38 30 4C 75 80 4C FA 80 80
    // Shadow of the Beast  09 80 83 81 C3 C2 CD 38 30 4C 83 81 4C 76 82 80
    // Read 0x8002 to determine whether Single Chip or Two Chip
    // IF 0x75 OR 0x83, THEN Two Chip ELSE Single Chip

    case 5:          // Ocean 128K/256K/512K
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      bankSwitch_C64(0xDE00, 0);  // Reset Bank 0
      uint8_t checkOcean = readData_C64(0x8002);
      ROML_DISABLE;
      if ((c64size == 8) && ((checkOcean == 0x75) || (checkOcean == 0x83))) {  // Two Chip 256K
        // Robocop 2 + Shadow of the Beast
        println_Msg(F("TWO CHIP"));
        display_Update();
        ROML_ENABLE;
        for (int x = 0; x < 16; x++) {
          bankSwitch_C64(0xDE00, x);        // Switch Bank using D0-D3
          readSegment_C64(0x8000, 0xA000);  // 8K * 16 = 128K
        }
        ROML_DISABLE;
        ROMH_ENABLE;
        for (int x = 0; x < 16; x++) {
          bankSwitch_C64(0xDE00, x);        // Switch Bank using D0-D3
          readSegment_C64(0xA000, 0xC000);  // 8K * 16 = +128K = 256K
        }
        ROMH_DISABLE;
      } else {  // Single Chip 128K/256K/512K
        println_Msg(F("SINGLE CHIP"));
        display_Update();
        ROML_ENABLE;
        c64banks = C64[c64size] / 8;
        for (int x = 0; x < c64banks; x++) {
          bankSwitch_C64(0xDE00, x);        // Switch Bank using D0-D5
          readSegment_C64(0x8000, 0xA000);  // 8K * Banks = 128K/256K/512K
        }
        ROML_DISABLE;
      }
      disablePorts_C64();
      break;

    case 6:           // Expert Cartridge (8K)
      GAME_DISABLE;   // HIGH
      EXROM_DISABLE;  // HIGH
      ROML_ENABLE;
      readSegment_C64(0x8000, 0xA000);  // 8K
      ROML_DISABLE;
      break;

    case 7:          // Fun Play, Power Play (128K)
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      for (int x = 0; x < 8; x++) {
        bankSwitch_C64(0xDE00, x * 8);    // Switch Bank 0-8
        readSegment_C64(0x8000, 0xA000);  // 8K * 8 = 64K
      }
      ROML_DISABLE;
      ROMH_ENABLE;
      for (int x = 0; x < 8; x++) {
        bankSwitch_C64(0xDE00, (x * 8) + 1);  // Switch Bank 9-15
        readSegment_C64(0x8000, 0xA000);      // 8K * 8 = +64K = 128K
      }
      ROMH_DISABLE;
      bankSwitch_C64(0xDE00, 0x86);  // Reset ROM
      disablePorts_C64();
      break;

    case 8:          // Super Games (64K)
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      for (int x = 0; x < 4; x++) {
        bankSwitch_C64(0xDF00, x);  // Switch Bank
        ROML_ENABLE;
        readSegment_C64(0x8000, 0xA000);  // 8K
        ROML_DISABLE;
        ROMH_ENABLE;
        readSegment_C64(0xA000, 0xC000);  // +8K = 16K
        ROMH_DISABLE;
      }
      disablePorts_C64();
      break;

    case 9:          // Atomic Power (32K)
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      for (int x = 0; x < 4; x++) {
        bankSwitch_C64(0xDE00, x << 3);   // Switch Bank using D3-D4
        readSegment_C64(0x8000, 0xA000);  // 8K
      }
      ROML_DISABLE;
      disablePorts_C64();
      break;

    case 10:          // Epyx Fastload (8K)
      GAME_DISABLE;   // HIGH
      EXROM_DISABLE;  // HIGH
      ROML_ENABLE;
      bankSwitch_C64(0xDE00, 0);             // Read IO1 - Trigger Access
      readSegment_C64(0x8000, 0x9E00);       // 7680 Bytes
      readSegmentSmall_C64(0x9E00, 0x9F00);  // +256 Bytes = 7936 Bytes
      bankSwitch_C64(0xDF00, 0);             // Read IO2 - Access Last 256 Bytes
      readSegmentSmall_C64(0x9F00, 0xA000);  // +256 Bytes = 8K
      ROML_DISABLE;
      break;

    case 12:         // Rex Utility (8K)
      GAME_DISABLE;  // HIGH
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      bankSwitch_C64(0xDFC0, 0);        // Enable ROM
      readSegment_C64(0x8000, 0xA000);  // 8K
      ROML_DISABLE;
      disablePorts_C64();
      break;

    case 13:                      // Final Cartridge I (16K)
      GAME_DISABLE;               // HIGH
      EXROM_DISABLE;              // HIGH
      bankSwitch_C64(0xDF00, 0);  // Enable ROM
      ROML_ENABLE;
      readSegment_C64(0x8000, 0xA000);  // 8K
      ROML_DISABLE;
      ROMH_ENABLE;
      readSegment_C64(0xA000, 0xC000);  // +8K = 16K
      ROMH_DISABLE;
      break;

    case 14:         // Magic Formel (64K)
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      ROMH_ENABLE;
      for (int x = 0; x < 8; x++) {
        bankSwitch_C64(0xDF00 + x, 0);     // Switch Bank using A0-A2
        readSegment_C64(0xE000, 0x10000);  // 8K * 8 = 64K
      }
      ROMH_DISABLE;
      disablePorts_C64();
      break;

    case 15:          // C64 Game System, System 3 (512K)
      GAME_ENABLE;    // LOW
      EXROM_DISABLE;  // HIGH
      ROML_ENABLE;
      for (int x = 0; x < 64; x++) {
        bankSwitch_C64(0xDE00 + x, 0);    // Switch Bank using A0-A4
        readSegment_C64(0x8000, 0xA000);  // 8K * 64 = 512K
      }
      disablePorts_C64();
      break;

    case 16:         // WarpSpeed (16K)
      GAME_ENABLE;   // LOW
      EXROM_ENABLE;  // LOW
      ROML_ENABLE;
      readSegment_C64(0x8000, 0xA000);  // 8K
      ROML_DISABLE;
      ROMH_ENABLE;
      readSegment_C64(0xA000, 0xC000);  // +8K = 16K
      ROMH_DISABLE;
      disablePorts_C64();
      break;

    case 17:          // Dinamic (128K) - Luigi Di Fraia has opposite PORT states - GAME_DISABLE/EXROM_ENABLE
      GAME_ENABLE;    // LOW
      EXROM_DISABLE;  // HIGH
      ROML_ENABLE;
      for (int x = 0; x < 16; x++) {
        bankSwitch_C64(0xDE00 + x, 0);    // Switch Bank using A0-A4
        readSegment_C64(0x8000, 0xA000);  // 8K * 16 = 128K
      }
      ROML_DISABLE;
      disablePorts_C64();
      break;

    case 18:          // Zaxxon, Super Zaxxon (SEGA) (20K)
      GAME_DISABLE;   // HIGH
      EXROM_DISABLE;  // HIGH
      ROML_ENABLE;
      readSegment_C64(0x8000, 0x9000);  // 4K
      ROML_DISABLE;
      ROMH_ENABLE;
      readSegment_C64(0xA000, 0xC000);  // +8K = 12K
      ROMH_DISABLE;
      // Switch Bank
      readData_C64(0x9000);
      ROMH_ENABLE;
      readSegment_C64(0xA000, 0xC000);  // +8K = 20K
      ROMH_DISABLE;
      break;

    case 19:          // Magic Desk, Domark, HES Australia (32K/64K/128K)
      GAME_ENABLE;    // LOW
      EXROM_DISABLE;  // HIGH
      ROML_ENABLE;
      c64banks = C64[c64size] / 8;
      for (int x = 0; x < c64banks; x++) {
        bankSwitch_C64(0xDE00, x);        // Switch Bank using D0-D3
        readSegment_C64(0x8000, 0xA000);  // 8K * Banks = 32K/64K/128K
      }
      ROML_DISABLE;
      disablePorts_C64();
      break;

    case 20:  // Super Snapshot 5 (64K)
      GAME_DISABLE;
      EXROM_DISABLE;
      for (int x = 0; x < 4; x++) {
        int bank = (((x & 2) << 3) | (0 << 3) | ((x & 1) << 2));
        bankSwitch_C64(0xDE00, bank);  // Switch Bank using D2-D4 (D3 == 0 Enable ROM)
        ROML_ENABLE;
        readSegment_C64(0x8000, 0xA000);  // 8K
        ROML_DISABLE;
        ROMH_ENABLE;
        readSegment_C64(0xA000, 0xC000);  // +8K = 16K
        ROMH_DISABLE;
      }
      break;

    case 21:  // Comal-80 (64K)
      GAME_DISABLE;
      EXROM_DISABLE;
      for (int x = 0; x < 4; x++) {
        bankSwitch_C64(0xDE00, x + 0x80);  // Switch Bank
        ROML_ENABLE;
        readSegment_C64(0x8000, 0xA000);  // 8K
        ROML_DISABLE;
        ROMH_ENABLE;
        readSegment_C64(0xA000, 0xC000);  // +8K = 16K
        ROMH_DISABLE;
      }
  }
  myFile.close();

  unsigned long crcsize = C64[c64size] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// MAPPER CODE
//******************************************
void setMapper_C64() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  uint8_t b = 0;
  int i = 0;
  // Check Button Status
#if defined(ENABLE_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
  if (buttonVal1 == LOW) {             // Button Pressed
    while (1) {                        // Scroll Mapper List
#if defined(ENABLE_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
      boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
      if (buttonVal1 == HIGH) {        // Button Released
        // Correct Overshoot
        if (i == 0)
          i = c64mapcount - 1;
        else
          i--;
        break;
      }
      display_Clear();
      print_Msg(F("Mapper: "));
      c64index = i * 3;
      c64mapselect = pgm_read_byte(c64mapsize + c64index);
      println_Msg(c64mapselect);
      printMapper_C64(c64mapselect);
      display_Update();
      if (i == (c64mapcount - 1))
        i = 0;
      else
        i++;
      delay(250);
    }
  }

  display_Clear();
  print_Msg(F("Mapper: "));
  c64index = i * 3;
  c64mapselect = pgm_read_byte(c64mapsize + c64index);
  println_Msg(c64mapselect);
  printMapper_C64(c64mapselect);
  println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
  print_STR(press_to_change_STR, 1);
  print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
  print_STR(rotate_to_change_STR, 1);
  print_STR(press_to_select_STR, 1);
#endif
  display_Update();

  while (1) {
    b = checkButton();
    if (b == 2) {  // Previous Mapper (doubleclick)
      if (i == 0)
        i = c64mapcount - 1;
      else
        i--;

      display_Clear();
      print_Msg(F("Mapper: "));
      c64index = i * 3;
      c64mapselect = pgm_read_byte(c64mapsize + c64index);
      println_Msg(c64mapselect);
      printMapper_C64(c64mapselect);
      println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();

    }
    if (b == 1) {  // Next Mapper (press)
      if (i == (c64mapcount - 1))
        i = 0;
      else
        i++;

      display_Clear();
      print_Msg(F("Mapper: "));
      c64index = i * 3;
      c64mapselect = pgm_read_byte(c64mapsize + c64index);
      println_Msg(c64mapselect);
      printMapper_C64(c64mapselect);
      println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();

    }
    if (b == 3) {  // Long Press - Execute (hold)
      newc64mapper = c64mapselect;
      break;
    }
  }
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(newc64mapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  c64mapfound = false;
  printMapper_C64(0);
  Serial.print(F("Enter Mapper [0-22]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newc64mapper = newmap.toInt();
  for (int i = 0; i < c64mapcount; i++) {
    c64index = i * 3;
    c64mapselect = pgm_read_byte(c64mapsize + c64index);
    if (newc64mapper == c64mapselect)
      c64mapfound = true;
  }
  if (c64mapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(FSTRING_EMPTY);
    newc64mapper = 0;
    goto setmapper;
  }
#endif
  EEPROM_writeAnything(7, newc64mapper);
  c64mapper = newc64mapper;
}

void checkMapperSize_C64() {
  for (int i = 0; i < c64mapcount; i++) {
    c64index = i * 3;
    byte mapcheck = pgm_read_byte(c64mapsize + c64index);
    if (mapcheck == c64mapper) {
      c64lo = pgm_read_byte(c64mapsize + c64index + 1);
      c64hi = pgm_read_byte(c64mapsize + c64index + 2);
      break;
    }
  }
}

//******************************************
// SET ROM SIZE
//******************************************
void setROMSize_C64() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (c64lo == c64hi)
    newc64size = c64lo;
  else {
    uint8_t b = 0;
    int i = c64lo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(C64[i]);
    println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
    print_STR(press_to_change_STR, 1);
    print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
    print_STR(rotate_to_change_STR, 1);
    print_STR(press_to_select_STR, 1);
#endif
    display_Update();

    while (1) {
      b = checkButton();
      if (b == 2) {  // Previous (doubleclick)
        if (i == c64lo)
          i = c64hi;
        else
          i--;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(C64[i]);
        println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
        print_STR(press_to_change_STR, 1);
        print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
        print_STR(rotate_to_change_STR, 1);
        print_STR(press_to_select_STR, 1);
#endif
        display_Update();
      }
      if (b == 1) {  // Next (press)
        if (i == c64hi)
          i = c64lo;
        else
          i++;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(C64[i]);
        println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
        print_STR(press_to_change_STR, 1);
        print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
        print_STR(rotate_to_change_STR, 1);
        print_STR(press_to_select_STR, 1);
#endif
        display_Update();
      }
      if (b == 3) {  // Long Press - Execute (hold)
        newc64size = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(C64[newc64size]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (c64lo == c64hi)
    newc64size = c64lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (c64hi - c64lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(C64[i + c64lo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newc64size = sizeROM.toInt() + c64lo;
    if (newc64size > c64hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FSTRING_EMPTY);
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(C64[newc64size]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newc64size);
  c64size = newc64size;
}

//******************************************
// SET PORT STATE
//******************************************
void setPorts_C64()
{
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  uint8_t b = 0;
  int i = 0;

  display_Clear();
  print_Msg(F("Port State: "));
  println_Msg(i);
  if (i == 0) {
    println_Msg(F("EXROM LOW/GAME LOW"));
  }
  else if (i == 1) {
    println_Msg(F("EXROM LOW/GAME HIGH"));
  }
  else if (i == 2) {
    println_Msg(F("EXROM HIGH/GAME LOW"));
  }
  else if (i == 3) {
    println_Msg(F("EXROM HIGH/GAME HIGH"));
  }
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(F("Press to Change"));
  println_Msg(F("Hold to Select"));
  display_Update();

  while (1) {
    b = checkButton();
    if (b == 2) { // Previous (doubleclick)
      if (i == 0)
        i = 3;
      else
        i--;

      display_Clear();
      print_Msg(F("Port State: "));
      println_Msg(i);
      if (i == 0) {
        println_Msg(F("EXROM LOW/GAME LOW"));
      }
      else if (i == 1) {
        println_Msg(F("EXROM LOW/GAME HIGH"));
      }
      else if (i == 2) {
        println_Msg(F("EXROM HIGH/GAME LOW"));
      }
      else if (i == 3) {
        println_Msg(F("EXROM HIGH/GAME HIGH"));
      }
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("Press to Change"));
      println_Msg(F("Hold to Select"));
      display_Update();

    }
    if (b == 1) { // Next (press)
      if (i == 3)
        i = 0;
      else
        i++;

      display_Clear();
      print_Msg(F("Port State: "));
      println_Msg(i);
      if (i == 0) {
        println_Msg(F("EXROM LOW/GAME LOW"));
      }
      else if (i == 1) {
        println_Msg(F("EXROM LOW/GAME HIGH"));
      }
      else if (i == 2) {
        println_Msg(F("EXROM HIGH/GAME LOW"));
      }
      else if (i == 3) {
        println_Msg(F("EXROM HIGH/GAME HIGH"));
      }
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("Press to Change"));
      println_Msg(F("Hold to Select"));
      display_Update();

    }
    if (b == 3) { // Long Press - Execute (hold)
      newc64port = i;
      break;
    }
  }
  display.setCursor(0, 56); // Display selection at bottom
  print_Msg(F("PORT STATE "));
  println_Msg(newc64port);
  display_Update();
  delay(1000);
#else
setrom:
  String sizeROM;
  Serial.print(F("Select Port State [0-3]:"));
  Serial.println(F("0 = EXROM LOW/GAME LOW"));
  Serial.println(F("1 = EXROM LOW/GAME HIGH"));
  Serial.println(F("2 = EXROM HIGH/GAME LOW"));
  Serial.println(F("3 = EXROM HIGH/GAME HIGH"));
  Serial.print(F("Enter Port State: "));
  while (Serial.available() == 0) {}
  sizeROM = Serial.readStringUntil('\n');
  Serial.println(sizeROM);
  newc64port = sizeROM.toInt();
  if (newc64port > 3) {
    Serial.println(F("INVALID STATE"));
    Serial.println(FSTRING_EMPTY);
    goto setrom;
  }
  Serial.print(F("Port State = "));
  Serial.println(newc64port);
#endif
  EEPROM_writeAnything(12, newc64port);
  c64port = newc64port;
}

//******************************************
// CHECK STATUS
//******************************************
void checkStatus_C64() {
  EEPROM_readAnything(7, c64mapper);
  EEPROM_readAnything(8, c64size);
  EEPROM_readAnything(12, c64port);
  if (c64mapper > 21) {
    c64mapper = 0;
    EEPROM_writeAnything(7, c64mapper);
  }
  if (c64size > 9) {
    c64size = 0;
    EEPROM_writeAnything(8, c64size);
  }
  if (c64port > 3) {
    c64port = 0;
    EEPROM_writeAnything(12, c64port);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("C64 READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:     "));
  println_Msg(c64mapper);
  printMapper_C64(c64mapper);
  print_Msg(F("ROM SIZE:   "));
  print_Msg(C64[c64size]);
  println_Msg(F("K"));
  print_Msg(F("PORT STATE: "));
  println_Msg(c64port);
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:     "));
  Serial.println(c64mapper);
  Serial.print(F("CURRENT ROM SIZE:   "));
  Serial.print(C64[c64size]);
  Serial.println(F("K"));
  Serial.print(F("CURRENT PORT STATE: "));
  Setial.println(c64port);
  Serial.println(FSTRING_EMPTY);
#endif
}

void printMapper_C64(byte c64maplabel) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  switch (c64maplabel) {
    case 0:
      println_Msg(F("NORMAL/ULTIMAX"));
      break;
    case 1:
      println_Msg(F("ACTION REPLAY"));
      break;
    case 2:
      println_Msg(F("KCS POWER CARTRIDGE"));
      break;
    case 3:
      println_Msg(F("FINAL CARTRIDGE III"));
      break;
    case 4:
      println_Msg(F("SIMONS BASIC"));
      break;
    case 5:
      println_Msg(F("OCEAN"));
      break;
    case 6:
      println_Msg(F("EXPERT CARTRIDGE"));
      break;
    case 7:
      println_Msg(F("FUN PLAY/POWER PLAY"));
      break;
    case 8:
      println_Msg(F("SUPER GAMES"));
      break;
    case 9:
      println_Msg(F("ATOMIC POWER"));
      break;
    case 10:
      println_Msg(F("EPYX FASTLOAD"));
      break;
    case 11:
      println_Msg(F("WESTERMANN LEARNING"));
      break;
    case 12:
      println_Msg(F("REX UTILITY"));
      break;
    case 13:
      println_Msg(F("FINAL CARTRIDGE I"));
      break;
    case 14:
      println_Msg(F("MAGIC FORMEL"));
      break;
    case 15:
      println_Msg(F("C64 GAME SYSTEM"));
      break;
    case 16:
      println_Msg(F("WARPSPEED"));
      break;
    case 17:
      println_Msg(F("DINAMIC"));
      break;
    case 18:
      println_Msg(F("ZAXXON/SUPER ZAXXON"));
      break;
    case 19:
      println_Msg(F("MAGIC DESK/DOMARK"));
      break;
    case 20:
      println_Msg(F("SUPER SNAPSHOT"));
      break;
    case 21:
      println_Msg(F("COMAL-80"));
      break;
  }
#else
  Serial.println(F("0 = NORMAL/ULTIMAX"));
  Serial.println(F("1 = ACTION REPLAY"));
  Serial.println(F("2 = KCS POWER CARTRIDGE"));
  Serial.println(F("3 = FINAL CARTRIDGE III"));
  Serial.println(F("4 = SIMONS BASIC"));
  Serial.println(F("5 = OCEAN"));
  Serial.println(F("6 = EXPERT CARTRIDGE"));
  Serial.println(F("7 = FUN PLAY/POWER PLAY"));
  Serial.println(F("8 = SUPER GAMES"));
  Serial.println(F("9 = ATOMIC POWER"));
  Serial.println(F("10 = EPYX FASTLOAD"));
  Serial.println(F("11 = WESTERMANN LEARNING"));
  Serial.println(F("12 = REX UTILITY"));
  Serial.println(F("13 = FINAL CARTRIDGE I"));
  Serial.println(F("14 = MAGIC FORMEL"));
  Serial.println(F("15 = C64 GAME SYSTEM"));
  Serial.println(F("16 = WARPSPEED"));
  Serial.println(F("17 = DINAMIC"));
  Serial.println(F("18 = ZAXXON/SUPER ZAXXON"));
  Serial.println(F("19 = MAGIC DESK/DOMARK/HES AUSTRALIA"));
  Serial.println(F("20 = SUPER SNAPSHOT"));
  Serial.println(F("21 = COMAL-80"));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
FsFile c64csvFile;
char c64game[47];                   // title
char c64mm[3];                      // mapper
char c64rr[3];                      // romsize
char c64pp[3];                      // port state (exrom+game)
char c64ll[4];                      // linelength (previous line)
unsigned long c64csvpos;            // CSV File Position
char c64cartCSV[] = "c64cart.txt";  // CSV List
char c64csvEND[] = "EOF";           // CSV End Marker for scrolling

bool readLine_C64(FsFile& f, char* line, size_t maxLen) {
  for (size_t n = 0; n < maxLen; n++) {
    int c = f.read();
    if (c < 0 && n == 0) return false;  // EOF
    if (c < 0 || c == '\n') {
      line[n] = 0;
      return true;
    }
    line[n] = c;
  }
  return false;  // line too long
}

bool readVals_C64(char* c64game, char* c64mm, char* c64rr, char* c64pp, char* c64ll) {
  char line[54];
  c64csvpos = c64csvFile.position();
  if (!readLine_C64(c64csvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(c64game, comma);
    else if (x == 1)
      strcpy(c64mm, comma);
    else if (x == 2)
      strcpy(c64rr, comma);
    else if (x == 3)
      strcpy(c64pp, comma);
    else if (x == 4)
      strcpy(c64ll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_C64() {
  bool buttonreleased = 0;
  bool cartselected = 0;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F(" HOLD TO FAST CYCLE"));
  display_Update();
#else
  Serial.println(F("HOLD BUTTON TO FAST CYCLE"));
#endif
  delay(2000);
#if defined(ENABLE_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
  if (buttonVal1 == LOW) {         // Button Held - Fast Cycle
    while (1) {                    // Scroll Game List
      while (readVals_C64(c64game, c64mm, c64rr, c64pp, c64ll)) {
        if (strcmp(c64csvEND, c64game) == 0) {
          c64csvFile.seek(0);  // Restart
        } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(FS(FSTRING_EMPTY));
          println_Msg(c64game);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(c64game);
#endif
#if defined(ENABLE_OLED)
          buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
          boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
          if (buttonVal1 == HIGH) {        // Button Released
            buttonreleased = 1;
            break;
          }
          if (buttonreleased) {
            buttonreleased = 0;  // Reset Flag
            break;
          }
        }
      }
#if defined(ENABLE_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
      boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
      if (buttonVal1 == HIGH)          // Button Released
        break;
    }
  }
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display.setCursor(0, 56);
  println_Msg(F("FAST CYCLE OFF"));
  display_Update();
#else
  Serial.println(FSTRING_EMPTY);
  Serial.println(F("FAST CYCLE OFF"));
  Serial.println(F("PRESS BUTTON TO STEP FORWARD"));
  Serial.println(F("DOUBLE CLICK TO STEP BACK"));
  Serial.println(F("HOLD TO SELECT"));
  Serial.println(FSTRING_EMPTY);
#endif
  while (readVals_C64(c64game, c64mm, c64rr, c64pp, c64ll)) {
    if (strcmp(c64csvEND, c64game) == 0) {
      c64csvFile.seek(0);  // Restart
    } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(c64game);
      display.setCursor(0, 48);
#if defined(ENABLE_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();
#else
      Serial.print(F("CART TITLE:"));
      Serial.println(c64game);
#endif
      while (1) {  // Single Step
        uint8_t b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(c64ll, NULL, 10);
          c64csvpos -= prevline;
          c64csvFile.seek(c64csvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          newc64mapper = strtol(c64mm, NULL, 10);
          newc64size = strtol(c64rr, NULL, 10);
          newc64port = strtol(c64pp, NULL, 10);
          EEPROM_writeAnything(7, newc64mapper);
          EEPROM_writeAnything(8, newc64size);
          EEPROM_writeAnything(12, newc64port);
          cartselected = 1;  // SELECTION MADE
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
          println_Msg(F("SELECTION MADE"));
          display_Update();
#else
          Serial.println(F("SELECTION MADE"));
#endif
          break;
        }
      }
      if (cartselected) {
        cartselected = 0;  // Reset Flag
        return true;
      }
    }
  }
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(F("END OF FILE"));
  display_Update();
#else
  Serial.println(F("END OF FILE"));
#endif

  return false;
}

void checkCSV_C64() {
  if (getCartListInfo_C64()) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(c64game);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: M"));
    print_Msg(newc64mapper);
    print_Msg(F("/R"));
    print_Msg(newc64size);
    print_Msg(F("/P"));
    print_Msg(newc64port);
    display_Update();
#else
    Serial.println(FSTRING_EMPTY);
    Serial.println(F("CART SELECTED"));
    Serial.println(c64game);
    // Display Settings
    Serial.print(F("CODE: M"));
    Serial.print(newc64mapper);
    Serial.print(F("/R"));
    Serial.print(newc64size);
    Serial.print(F("/P"));
    Serial.print(newc64port);
    Serial.println(FSTRING_EMPTY);
#endif
  } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display.setCursor(0, 56);
    println_Msg(F("NO SELECTION"));
    display_Update();
#else
    Serial.println(F("NO SELECTION"));
#endif
  }
}

void setCart_C64() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(c64cartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "C64/CSV");
  sd.chdir(folder);  // Switch Folder
  c64csvFile = sd.open(c64cartCSV, O_READ);
  if (!c64csvFile) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_C64();
    }
  }
  checkCSV_C64();

  c64csvFile.close();
}
#endif
