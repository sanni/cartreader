//******************************************
// POKEMON MINI MODULE
//******************************************
#ifdef ENABLE_POKE
// Pokemon Mini
// Cartridge Pinout
// 32P 0.5mm pitch
// NOTE:  Console SMD connector - 33P 1.0mm pitch
//
//    TOP ROW                BOTTOM ROW
//             +----------+
// VCC (3.3V) -| B1       |
//             |       A1 |- VCC (3.3V)
//        A20 -| B2       |
//             |       A2 |- A9/A19
//     A8/A18 -| B3       |
//             |       A3 |- A7/A17
//     A6/A16 -| B4       |
//             |       A4 |- A5/A15
//     A4/A14 -| B5       |
//             |       A5 |- A3/A13
//     A2/A12 -| B6       |
//             |       A6 |- A1/A11
//     A0/A10 -| B7       |
//             |       A7 |- VCC (3.3V)
//       HALE -| B8       |
//             |       -- |
//       LALE -| B9       |
//             |       A8 |- GND
//         D0 -| B10      |
//             |       A9 |- D1
//         D2 -| B11      |
//             |      A10 |- D3
//         D4 -| B12      |
//             |      A11 |- D5
//         D6 -| B13      |
//             |      A12 |- D7
//         OE -| B14      |
//             |      A13 |- IRQ
//         WE -| B15      |
//             |      A14 |- CS
//     CARD_N -| B16      |
//             |      A15 |- GND
//        GND -| B17      |
//             +----------+
//
//                                           TOP
//      +----------------------------------------------------------------------------+
//      | B17  B16  B15  B14  B13  B12  B11  B10  B9  B8  B7  B6  B5  B4  B3  B2  B1 |
// LEFT |                                                                            | RIGHT
//      |    A15  A14  A13  A12  A11  A10  A9  A8  ---  A7  A6  A5  A4  A3  A2  A1   |
//      +----------------------------------------------------------------------------+
//                                          BOTTOM

// CONTROL PINS:
// HALE(PH0) - SNES RESET
// CS(PH3)   - SNES /CS
// LALE(PH4) - SNES /IRQ
// WE(PH5)   - SNES /WR
// OE(PH6)   - SNES /RD

// NOTE:  SET VOLTAGE TO 3.3V

//******************************************
//  Defines
//******************************************
#define HALE_ENABLE PORTH |= (1 << 0)  // LATCH HIGH ADDRESS
#define HALE_DISABLE PORTH &= ~(1 << 0)
#define LALE_ENABLE PORTH |= (1 << 4)  // LATCH LOW ADDRESS
#define LALE_DISABLE PORTH &= ~(1 << 4)
#define CS_ENABLE PORTH |= (1 << 3)  // CHIP SELECT
#define CS_DISABLE PORTH &= ~(1 << 3)
#define WE_ENABLE PORTH |= (1 << 5)  // WRITE ENABLE
#define WE_DISABLE PORTH &= ~(1 << 5)
#define OE_ENABLE PORTH |= (1 << 6)  // OUTPUT ENABLE
#define OE_DISABLE PORTH &= ~(1 << 6)

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptionsPOKE[] PROGMEM = { FSTRING_READ_ROM, FSTRING_RESET };

void pokeMenu() {
  convertPgm(menuOptionsPOKE, 2);
  uint8_t mainMenu = question_box(F("POKEMON MINI MENU"), menuOptions, 2, 0);

  switch (mainMenu) {
    case 0:
      // Read ROM
      sd.chdir("/");
      readROM_POKE();
      sd.chdir("/");
      break;

    case 1:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  SETUP
//******************************************
void setup_POKE() {
  // Request 3.3V
  setVoltage(VOLTS_SET_3V3);

  // Set Address Pins to Output
  // Pokemon Mini uses A0-A9 (DUAL A10-A19) + A20 (CONNECT TO SNES A10) [A11-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       LALE(PH0)   ---(PH1)  CS(PH3)    HALE(PH4)  WE(PH5)    OE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to LOW (DISABLE)
  //       LALE(PH0)    CS(PH3)     HALE(PH4)   WE(PH5)     OE(PH6)
  PORTH &= ~(1 << 0) & ~(1 << 3) & ~(1 << 4) & ~(1 << 5) & ~(1 << 6);

  // SET CS HIGH
  PORTH |= (1 << 3);  // CS HIGH (ENABLE)

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTH |= (1 << 1);  // CPUCLK(PH1)
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  strcpy(romName, "POKEMINI");

  mode = CORE_POKE;
}

//******************************************
// READ DATA
//******************************************
uint8_t readData_POKE(uint32_t addr) {
  // Address Lines A0-A20 = 1MB (ALL ROMS ARE 512K)
  // 100000 = 1 0000 0000 0000 0000 0000 = 100 0000 0000|00 0000 0000

  // High Address A10-A20
  PORTF = (addr >> 10) & 0xFF;  // A10-A17
  PORTK = (addr >> 18) & 0x07;  // A18-A20
  HALE_ENABLE;                  // LATCH HIGH ADDRESS
  NOP;
  NOP;
  HALE_DISABLE;

  // Low Address A0-A9
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0x03;  // A8-A9
  LALE_ENABLE;                 // LATCH LOW ADDRESS
  NOP;
  NOP;

  // Read Enable
  OE_ENABLE;  // OE HIGH
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Data
  uint8_t ret = PINC;

  // Read Disable
  OE_DISABLE;  // OE LOW
  NOP;
  NOP;
  NOP;
  NOP;

  LALE_DISABLE;

  return ret;
}

//******************************************
// WRITE DATA
//******************************************
void writeData_POKE(uint32_t addr, uint8_t data) {
  // High Address A10-A20
  PORTF = (addr >> 10) & 0xFF;  // A10-A17
  PORTK = (addr >> 18) & 0x07;  // A18-A20
  HALE_ENABLE;                  // LATCH HIGH ADDRESS
  NOP;
  NOP;
  HALE_DISABLE;

  // Low Address A0-A9
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0x03;  // A8-A9
  LALE_ENABLE;                 // LATCH LOW ADDRESS

  // Set Data to Output
  DDRC = 0xFF;
  PORTC = data;

  // Write Enable
  WE_ENABLE;  // WE HIGH
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Write Disable
  WE_DISABLE;  // OE LOW
  NOP;
  NOP;
  NOP;
  NOP;

  LALE_DISABLE;

  // Reset Data to Input
  DDRC = 0x00;
}

//******************************************
// READ ROM
//******************************************

void readROM_POKE() {
  createFolderAndOpenFile("POKE", "ROM", romName, "min");

  // read rom
  uint32_t progress = 0;
  draw_progressbar(0, 0x80000);
  for (uint32_t addr = 0; addr < 0x80000; addr += 512) {  // 512K
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_POKE(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
    progress += 512;
    draw_progressbar(progress, 0x80000);
  }
  myFile.close();

  // compare dump CRC with db values
  compareCRC("pkmn.txt", 0, 1, 0);
  println_Msg(FS(FSTRING_EMPTY));

  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}
#endif
