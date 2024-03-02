//******************************************
// ATARI 7800 MODULE
//******************************************
#ifdef ENABLE_7800
// Atari 7800
// Cartridge Pinout
// 32P 2.54mm pitch (USE 36P CONNECTOR)
//
//              RIGHT
//            +-------+
//       R/W -|  1 32 |- CLK
//      HALT -|  2 31 |- IRQ
//            |-------|
//        D3 -|  3 30 |- GND
// L      D4 -|  4 29 |- D2     B
// A      D5 -|  5 28 |- D1     O
// B      D6 -|  6 27 |- D0     T
// E      D7 -|  7 26 |- A0     T
// L     A12 -|  8 25 |- A1     O
//       A10 -|  9 24 |- A2     M
// S     A11 -| 10 23 |- A3
// I      A9 -| 11 22 |- A4     S
// D      A8 -| 12 21 |- A5     I
// E     +5V -| 13 20 |- A6     D
//       GND -| 14 19 |- A7     E
//            |-------|
//       A13 -| 15 18 |- AUD
//       A14 -| 16 17 |- A15
//            +-------+
//              LEFT
//
//                                     LABEL SIDE
//
//         A14 A13  GND +5V  A8  A9 A11 A10 A12  D7  D6  D5  D4  D3  HLT R/W
//       +--------------------------------------------------------------------+
//       |  16  15 | 14  13  12  11  10   9   8   7   6   5   4   3 |  2   1  |
// LEFT  |         |                                                |         | RIGHT
//       |  17  18 | 19  20  21  22  23  24  25  26  27  28  29  30 | 31  32  |
//       +--------------------------------------------------------------------+
//         A15 AUD   A7  A6  A5  A4  A3  A2  A1  A0  D0  D1  D2 GND  IRQ CLK
//
//                                    BOTTOM SIDE

// CONTROL PINS:
// CLK(PH1) - SNES CPUCLK
// R/W(PH6) - SNES /RD

//******************************************
//  Supported Mappers
//******************************************
// Supported Mapper Array
// Format = {mapper,romsizelo,romsizehi}
static const byte PROGMEM a7800mapsize[] = {
  0, 0, 2,  // Standard 16K/32K/48K [7816/7832/7848]
  1, 4, 4,  // SuperGame 128K [78SG]
  2, 5, 5,  // SuperGame - Alien Brigade/Crossbow 144K [78S9]
  3, 3, 3,  // F-18 Hornet 64K [78AB]
  4, 4, 4,  // Double Dragon/Rampage 128K [78AC]
  5, 3, 3,  // Realsports Baseball/Tank Command/Tower Toppler/Waterski 64K [78S4]
  6, 3, 3,  // Karateka (PAL) 64K [78S4 Variant]
};

byte a7800mapcount = 7;  // (sizeof(a7800mapsize) / sizeof(a7800mapsize[0])) / 3;
byte a7800mapselect;
int a7800index;

byte a7800[] = { 16, 32, 48, 64, 128, 144 };
byte a7800lo = 0;  // Lowest Entry
byte a7800hi = 5;  // Highest Entry
byte a7800mapper = 0;
byte new7800mapper;
byte a7800size;
byte new7800size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptions7800[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void setup_7800() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Atari 7800 uses A0-A15 [A16-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH3)   ---(PH4)   ---(PH5)   R/W(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)    ---(PH3)   ---(PH4)   ---(PH5)   R/W(PH6)
  PORTH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

#ifdef ENABLE_CLOCKGEN
  // Adafruit Clock Generator

  initializeClockOffset();

  if (!i2c_found) {
    display_Clear();
    print_FatalError(F("Clock Generator not found"));
  }

  // Set Eeprom clock to 1Mhz
  clockgen.set_freq(200000000ULL, SI5351_CLK1);

  // Start outputting Eeprom clock
  clockgen.output_enable(SI5351_CLK1, 1);  // Eeprom clock

  // Wait for clock generator
  clockgen.update_status();

#else
  // Set CLK(PH1) to Output 
  DDRH |= (1 << 1);
  // Output a high signal CLK(PH1)
  PORTH |= (1 << 1);
#endif

  checkStatus_7800();
  strcpy(romName, "ATARI");

  mode = CORE_7800;
}

void a7800Menu() {
  convertPgm(menuOptions7800, 4);
  uint8_t mainMenu = question_box(F("ATARI 7800 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_7800();
      wait();
      setup_7800();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_7800();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper + Size
      setMapper_7800();
      checkMapperSize_7800();
      setROMSize_7800();
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

uint8_t readData_7800(uint32_t addr)  // Add Input Pullup
{
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  //  DDRC = 0x00; // Set to Input
  PORTC = 0xFF;  // Input Pullup
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  return ret;
}

void readSegment_7800(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_7800(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void writeData_7800(uint32_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0xFF;  // Set to Output
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  PORTC = data;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  PORTH &= ~(1 << 6);  // R/W(PH6) LOW = WRITE
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  PORTH |= (1 << 6);  // R/W(PH6) HIGH = READ

  DDRC = 0x00;  // Reset to Input
}

// Activision Bankswitch - Double Dragon/Rampage 128K
void bankSwitch_7800(uint32_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  PORTH &= ~(1 << 6);            // R/W(PH6) LOW = WRITE
  for (int y = 0; y < 8; y++) {  // Pulse Clock to latch data into PAL16 Chip
    PORTH &= ~(1 << 1);          // CLK(PH1) LOW
    NOP;
    PORTH |= (1 << 1);  // CLK(PH1) HIGH
    NOP;
  }

  PORTH |= (1 << 6);  // R/W(PH6) HIGH = READ

  DDRC = 0x00;  // Reset to Input
}

//******************************************
// READ ROM
//******************************************

void readROM_7800() {
  strcpy(fileName, romName);
  strcat(fileName, ".a78");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "7800/ROM/%d", foldern);
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

  switch (a7800mapper) {
    case 0:  // Standard 16K/32K/48K [7816/7832/7848]
      if (a7800size > 1)
        readSegment_7800(0x4000, 0x8000);  // +16K = 48K
      if (a7800size > 0)
        readSegment_7800(0x8000, 0xC000);  // +16K = 32K
      // Base 16K
      readSegment_7800(0xC000, 0x10000);  // 16K
      break;

    case 1:  // SuperGame 128K [78SG]
      for (int x = 0; x < 7; x++) {
        writeData_7800(0x8000, x);         // Banks 0-6
        readSegment_7800(0x8000, 0xC000);  // 16K * 7 = 112K
      }
      readSegment_7800(0xC000, 0x10000);  // Bank 7 +16 = 128K
      break;

    case 2:                              // SuperGame - Alien Brigade/Crossbow 144K [78S9]
      readSegment_7800(0x4000, 0x8000);  // 16K
      for (int x = 0; x < 8; x++) {
        writeData_7800(0x8000, x);         // Banks 0-7
        readSegment_7800(0x8000, 0xC000);  // 16K * 8 = +128K = 144K
      }
      break;

    case 3:                                // F-18 Hornet 64K [78AB]
      for (int x = 1; x >= 0; x--) {       // Bank Order Inverted = Bank 1 then Bank 0
        writeData_7800(0x8000, x);         // Banks 0-1
        readSegment_7800(0x4000, 0x8000);  // 16K * 2 = 32K
      }
      readSegment_7800(0x8000, 0x10000);  // +32K = 64K
      break;

    case 4:  // Double Dragon/Rampage 128K [78AC]
      /*
        // THIS CODE OUTPUTS ROMS THAT SHOULD MATCH THE CURRENT NO-INTRO DATABASE
        // BUT THE ROMS INCLUDED IN THE NO-INTRO DATABASE WERE DONE INCORRECTLY
        // THE UPPER AND LOWER HALVES OF EACH 16K BANK ARE IN THE WRONG ORDER
        // THE OLD BANKSWITCH INFORMATION IS INCORRECT/INCOMPLETE
        // Double Dragon (NTSC) = CRC32 AA265865 BAD
        // Double Dragon (PAL)  = CRC32 F29ABDB2 BAD
        // Rampage (NTSC)       = CRC32 39A316AA BAD
        for (int x = 0; x < 8; x++) {
        bankSwitch_7800(0xFF80 + x); // Activision Bankswitch
        readSegment_7800(0xA000, 0xE000); // 16K * 8 = 128K
        }
      */
      // THIS CODE OUTPUTS PROPER ROMS THAT SHOULD MATCH MAME
      // THE UPPER AND LOWER HALVES OF EACH 16K BANK ARE IN THE CORRECT ORDER
      // Double Dragon (NTSC) = CRC32 F20773D5
      // Double Dragon (PAL)  = CRC32 4D634BF5
      // Rampage (NTSC)       = CRC32 D2876EE2
      for (int x = 0; x < 8; x++) {
        bankSwitch_7800(0xFF80 + x);       // Activision Bankswitch
        readSegment_7800(0xC000, 0xE000);  // 8K * 8 = 64K
        readSegment_7800(0xA000, 0xC000);  // 8K * 8 = +64K = 128K
      }
      break;

    case 5:  // Realsports Baseball/Tank Command/Tower Toppler/Waterski 64K [78S4]
      for (int x = 0; x < 4; x++) {
        writeData_7800(0x8000, x);
        readSegment_7800(0x8000, 0xC000);  // 16K * 4 = 64K
      }
      break;

    case 6:  // Karateka (PAL) 64K [78S4 Variant]
      for (int x = 4; x < 8; x++) {
        writeData_7800(0x8000, x);
        readSegment_7800(0x8000, 0xC000);  // 16K * 4 = 64K
      }
      break;
  }
  myFile.close();

  unsigned long crcsize = a7800[a7800size] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void checkMapperSize_7800() {
  for (int i = 0; i < a7800mapcount; i++) {
    a7800index = i * 3;
    byte mapcheck = pgm_read_byte(a7800mapsize + a7800index);
    if (mapcheck == a7800mapper) {
      a7800lo = pgm_read_byte(a7800mapsize + a7800index + 1);
      a7800hi = pgm_read_byte(a7800mapsize + a7800index + 2);
      break;
    }
  }
}

void setROMSize_7800() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (a7800lo == a7800hi)
    new7800size = a7800lo;
  else {
    uint8_t b = 0;
    int i = a7800lo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(a7800[i]);
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
        if (i == a7800lo)
          i = a7800hi;
        else
          i--;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(a7800[i]);
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
        if (i == a7800hi)
          i = a7800lo;
        else
          i++;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(a7800[i]);
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
        new7800size = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(a7800[new7800size]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (a7800lo == a7800hi)
    new7800size = a7800lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (a7800hi - a7800lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(a7800[i + a7800lo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    new7800size = sizeROM.toInt() + a7800lo;
    if (new7800size > a7800hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FSTRING_EMPTY);
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(a7800[new7800size]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, new7800size);
  a7800size = new7800size;
}

void checkStatus_7800() {
  EEPROM_readAnything(7, a7800mapper);
  EEPROM_readAnything(8, a7800size);
  if (a7800mapper > 6) {
    a7800mapper = 0;  // default
    EEPROM_writeAnything(7, a7800mapper);
  }
  if (a7800size > 5) {
    a7800size = 0;  // default 16KB
    EEPROM_writeAnything(8, a7800size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("ATARI 7800 READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Msg(a7800mapper);
  if (a7800mapper == 0)
    println_Msg(F("STANDARD"));
  else if (a7800mapper == 1)
    println_Msg(F("SUPERGAME [78SG]"));
  else if (a7800mapper == 2)
    println_Msg(F("SUPERGAME [78S9]"));
  else if (a7800mapper == 3)
    println_Msg(F("F-18 HORNET [78AB]"));
  else if (a7800mapper == 4)
    println_Msg(F("DBLDRGN/RMPG [78AC]"));
  else if (a7800mapper == 5)
    println_Msg(F("BASEBALL/ETC [78S4]"));
  else if (a7800mapper == 6)
    println_Msg(F("KARATEKA(PAL)[78S4]"));
  print_Msg(F("ROM SIZE: "));
  print_Msg(a7800[a7800size]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("MAPPER:   "));
  Serial.println(a7800mapper);
  if (a7800mapper == 0)
    Serial.println(F("Standard [7816/7832/7848]"));
  else if (a7800mapper == 1)
    Serial.println(F("SuperGame 128K [78SG]"));
  else if (a7800mapper == 2)
    Serial.println(F("SuperGame 144K [78S9] - Alien Brigade/Crossbow"));
  else if (a7800mapper == 3)
    Serial.println(F("F-18 Hornet 64K [78AB]"));
  else if (a7800mapper == 4)
    Serial.println(F("Double Dragon/Rampage [78AC]"));
  else if (a7800mapper == 5)
    Serial.println(F("Realsports Baseball/Tank Command/Tower Toppler/Waterski [78S4]"));
  else if (a7800mapper == 6)
    Serial.println(F("Karateka (PAL) [78S4 Variant]"));
  Serial.print(F("ROM SIZE: "));
  Serial.print(A7800[a7800size]);
  Serial.println(F("K"));
  Serial.println(FSTRING_EMPTY);
#endif
}

//******************************************
// SET MAPPER
//******************************************

void setMapper_7800() {
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
          i = a7800mapcount - 1;
        else
          i--;
        break;
      }
      display_Clear();
      print_Msg(F("Mapper: "));
      a7800index = i * 3;
      a7800mapselect = pgm_read_byte(a7800mapsize + a7800index);
      println_Msg(a7800mapselect);
      if (a7800mapselect == 0)
        println_Msg(F("STANDARD"));
      else if (a7800mapselect == 1)
        println_Msg(F("SUPERGAME 128K[78SG]"));
      else if (a7800mapselect == 2)
        println_Msg(F("SUPERGAME 144K[78S9]"));
      else if (a7800mapselect == 3)
        println_Msg(F("F-18 HORNET   [78AB]"));
      else if (a7800mapselect == 4)
        println_Msg(F("DBLDRGN/RAMPG [78AC]"));
      else if (a7800mapselect == 5)
        println_Msg(F("BASEBALL/ETC  [78S4]"));
      else if (a7800mapselect == 6)
        println_Msg(F("KARATEKA(PAL) [78S4]"));
      display_Update();
      if (i == (a7800mapcount - 1))
        i = 0;
      else
        i++;
      delay(250);
    }
  }

 display_Clear();
      print_Msg(F("Mapper: "));
      a7800index = i * 3;
      a7800mapselect = pgm_read_byte(a7800mapsize + a7800index);
      println_Msg(a7800mapselect);
      if (a7800mapselect == 0)
        println_Msg(F("STANDARD"));
      else if (a7800mapselect == 1)
        println_Msg(F("SUPERGAME 128K[78SG]"));
      else if (a7800mapselect == 2)
        println_Msg(F("SUPERGAME 144K[78S9]"));
      else if (a7800mapselect == 3)
        println_Msg(F("F-18 HORNET   [78AB]"));
      else if (a7800mapselect == 4)
        println_Msg(F("DBLDRGN/RAMPG [78AC]"));
      else if (a7800mapselect == 5)
        println_Msg(F("BASEBALL/ETC  [78S4]"));
      else if (a7800mapselect == 6)
        println_Msg(F("KARATEKA(PAL) [78S4]"));
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
        i = a7800mapcount - 1;
      else
        i--;

      display_Clear();
      print_Msg(F("Mapper: "));
      a7800index = i * 3;
      a7800mapselect = pgm_read_byte(a7800mapsize + a7800index);
      println_Msg(a7800mapselect);
      if (a7800mapselect == 0)
        println_Msg(F("STANDARD"));
      else if (a7800mapselect == 1)
        println_Msg(F("SUPERGAME 128K[78SG]"));
      else if (a7800mapselect == 2)
        println_Msg(F("SUPERGAME 144K[78S9]"));
      else if (a7800mapselect == 3)
        println_Msg(F("F-18 HORNET   [78AB]"));
      else if (a7800mapselect == 4)
        println_Msg(F("DBLDRGN/RAMPG [78AC]"));
      else if (a7800mapselect == 5)
        println_Msg(F("BASEBALL/ETC  [78S4]"));
      else if (a7800mapselect == 6)
        println_Msg(F("KARATEKA(PAL) [78S4]"));
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
      if (i == (a7800mapcount - 1))
        i = 0;
      else
        i++;

      display_Clear();
      print_Msg(F("Mapper: "));
      a7800index = i * 3;
      a7800mapselect = pgm_read_byte(a7800mapsize + a7800index);
      println_Msg(a7800mapselect);
      if (a7800mapselect == 0)
        println_Msg(F("STANDARD"));
      else if (a7800mapselect == 1)
        println_Msg(F("SUPERGAME 128K[78SG]"));
      else if (a7800mapselect == 2)
        println_Msg(F("SUPERGAME 144K[78S9]"));
      else if (a7800mapselect == 3)
        println_Msg(F("F-18 HORNET   [78AB]"));
      else if (a7800mapselect == 4)
        println_Msg(F("DBLDRGN/RAMPG [78AC]"));
      else if (a7800mapselect == 5)
        println_Msg(F("BASEBALL/ETC  [78S4]"));
      else if (a7800mapselect == 6)
        println_Msg(F("KARATEKA(PAL) [78S4]"));
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
      new7800mapper = a7800mapselect;
      break;
    }
  }
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(new7800mapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  Serial.println(F("SUPPORTED MAPPERS:"));
  Serial.println(F("0 = Standard [7816/7832/7848]"));
  Serial.println(F("1 = SuperGame 128K [78SG]"));
  Serial.println(F("2 = SuperGame 144K [78S9] - Alien Brigade/Crossbow"));
  Serial.println(F("3 = F-18 Hornet 64K [78AB]"));
  Serial.println(F("4 = Double Dragon/Rampage [78AC]"));
  Serial.println(F("5 = Realsports Baseball/Tank Command/Tower Toppler/Waterski [78S4]"));
  Serial.println(F("6 = Karateka (PAL) [78S4 Variant]"));
  Serial.print(F("Enter Mapper [0-6]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  a7800index = newmap.toInt() * 3;
  new7800mapper = pgm_read_byte(a7800mapsize + a7800index);
#endif
  EEPROM_writeAnything(7, new7800mapper);
  a7800mapper = new7800mapper;

  a7800size = pgm_read_byte(a7800mapsize + a7800index + 1);
  EEPROM_writeAnything(8, a7800size);
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile a7800csvFile;
char a7800game[36];                    // title
char a7800mm[3];                       // mapper
char a7800rr[3];                       // mapper
char a7800ll[4];                       // linelength (previous line)
unsigned long a7800csvpos;             // CSV File Position
char a7800cartCSV[] = "7800.txt";      // CSV List
char a7800csvEND[] = "EOF";            // CSV End Marker for scrolling

bool readLine_7800(FsFile& f, char* line, size_t maxLen) {
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

bool readVals_7800(char* a7800game, char* a7800mm, char* a7800rr, char* a7800ll) {
  char line[44];
  a7800csvpos = a7800csvFile.position();
  if (!readLine_7800(a7800csvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(a7800game, comma);
    else if (x == 1)
      strcpy(a7800mm, comma);
    else if (x == 2)
      strcpy(a7800rr, comma);
    else if (x == 3)
      strcpy(a7800ll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_7800() {
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
      while (readVals_7800(a7800game, a7800mm, a7800rr, a7800ll)) {
        if (strcmp(a7800csvEND, a7800game) == 0) {
          a7800csvFile.seek(0);  // Restart
        } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(FS(FSTRING_EMPTY));
          println_Msg(a7800game);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(a7800game);
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
  while (readVals_7800(a7800game, a7800mm, a7800rr, a7800ll)) {
    if (strcmp(a7800csvEND, a7800game) == 0) {
      a7800csvFile.seek(0);  // Restart
    } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(a7800game);
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
      Serial.println(a7800game);
#endif
      while (1) {  // Single Step
        uint8_t b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(a7800ll, NULL, 10);
          a7800csvpos -= prevline;
          a7800csvFile.seek(a7800csvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          new7800mapper = strtol(a7800mm, NULL, 10);
          new7800size = strtol(a7800rr, NULL, 10);
          EEPROM_writeAnything(7, new7800mapper);
          EEPROM_writeAnything(8, new7800size);
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

void checkCSV_7800() {
  if (getCartListInfo_7800()) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(a7800game);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: M"));
    print_Msg(new7800mapper);
    print_Msg(F("/R"));
    println_Msg(new7800size);
    display_Update();
#else
    Serial.println(FSTRING_EMPTY);
    Serial.println(F("CART SELECTED"));
    Serial.println(a7800game);
    // Display Settings
    Serial.print(F("CODE: M"));
    Serial.print(new7800mapper);
    Serial.print(F("/R"));
    Serial.println(new7800size);
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

void checkSize_7800() {
  EEPROM_readAnything(7, a7800mapper);
  for (int i = 0; i < a7800mapcount; i++) {
    a7800index = i * 3;
    if (a7800mapper == pgm_read_byte(a7800mapsize + a7800index)) {
      a7800size = pgm_read_byte(a7800mapsize + a7800index + 1);
      EEPROM_writeAnything(8, a7800size);
      break;
    }
  }
}

void setCart_7800() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(a7800cartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "7800/CSV");
  sd.chdir(folder);  // Switch Folder
  a7800csvFile = sd.open(a7800cartCSV, O_READ);
  if (!a7800csvFile) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_7800();
    }
  }
  checkCSV_7800();
  a7800csvFile.close();
}
#endif
