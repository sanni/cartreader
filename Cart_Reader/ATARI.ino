//******************************************
// ATARI 2600 MODULE
//******************************************
#if defined(enable_ATARI)
// Atari 2600
// Cartridge Pinout
// 24P 2.54mm pitch connector
//
//                            LABEL SIDE
//
//         GND +5V  A8  A9  A11 A10 A12 D7  D6  D5  D4  D3
//       +--------------------------------------------------+
//       |  24  23  22  21  20  19  18  17  16  15  14  13  |
// LEFT  |                                                  | RIGHT
//       |   1   2   3   4   5   6   7   8   9  10  11  12  |
//       +--------------------------------------------------+
//          A7  A6  A5  A4  A3  A2  A1  A0  D0  D1  D2  GND
//
//                           BOTTOM SIDE

// Cart Configurations
// Format = {mapper,romsize}
static const byte PROGMEM atarimapsize[] = {
  0x20, 0,  // 2K
  0x3F, 2,  // Tigervision 8K
  0x40, 1,  // 4K [DEFAULT]
  0xC0, 0,  // "CV" Commavid 2K
  0xD0, 2,  // "DPC" Pitfall II 10K
  0xE0, 2,  // Parker Bros 8K
  0xE7, 4,  // M-Network 16K
  0xF0, 6,  // Megaboy 64K
  0xF4, 5,  // Atari 32K
  0xF6, 4,  // Atari 16K
  0xF8, 2,  // Atari 8K
  0xFA, 3,  // CBS RAM Plus 12K
  0xFE, 2,  // Activision 8K
  0x0A, 2,  // "UA" UA Ltd 8K
};

//byte atarimapcount = 14;
byte atarimapcount = (sizeof(atarimapsize) / sizeof(atarimapsize[0])) / 2;

byte atarimapselect;
int atariindex;

byte ATARI[] = { 2, 4, 8, 12, 16, 32, 64 };
byte atarimapper = 0;
byte newatarimapper;
byte atarisize;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char atariMenuItem1[] PROGMEM = "Select Cart";
static const char atariMenuItem2[] PROGMEM = "Read ROM";
static const char atariMenuItem3[] PROGMEM = "Set Mapper";
static const char atariMenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsATARI[] PROGMEM = { atariMenuItem1, atariMenuItem2, atariMenuItem3, atariMenuItem4 };

void setup_ATARI() {
  // Set Address Pins to Output
  // Atari 2600 uses A0-A12 [A13-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output [UNUSED]
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH [UNUSED]
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_ATARI();
  strcpy(romName, "ATARI");

  mode = mode_ATARI;
}

void atariMenu() {
  convertPgm(menuOptionsATARI, 4);
  uint8_t mainMenu = question_box(F("ATARI 2600 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_ATARI();
      wait();
      setup_ATARI();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_ATARI();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper
      setMapper_ATARI();
      checkStatus_ATARI();
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

uint8_t readData_ATARI(uint16_t addr)  // Add Input Pullup
{
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A12
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0x00;   // Set to Input
  PORTC = 0xFF;  // Input Pullup
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

void readSegment_ATARI(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_ATARI(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readSegmentF8_ATARI(uint16_t startaddr, uint16_t endaddr, uint16_t bankaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      if (addr > 0x1FF9)  // SET BANK ADDRESS FOR 0x1FFA-0x1FFF
        readData_ATARI(bankaddr);
      uint8_t temp = readData_ATARI(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void writeData_ATARI(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A12
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

  DDRC = 0x00;  // Reset to Input
}

void writeData3F_ATARI(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A12
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

  // Address (0x1000);
  PORTF = 0x00;  // A0-A7
  PORTK = 0x10;  // A8-A12
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0x00;  // Reset to Input
}

void readROM_ATARI() {
  strcpy(fileName, romName);
  strcat(fileName, ".a26");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ATARI/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(create_file_STR);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  // ROM Start 0xF000
  // Address A12-A0 = 0x1000 = 1 0000 0000 0000 = 4KB
  // Read Start 0x1000

  switch (atarimapper) {
    case 0x20:  // 2K Standard 2KB
      readSegment_ATARI(0x1000, 0x1800);
      break;

    case 0x3F:  // 3F Mapper 8KB
      for (int x = 0; x < 0x3; x++) {
        writeData3F_ATARI(0x3F, x);
        readSegment_ATARI(0x1000, 0x1800);
      }
      readSegment_ATARI(0x1800, 0x2000);
      break;

    case 0x40:  // 4K Default 4KB
      readSegment_ATARI(0x1000, 0x2000);
      break;

    case 0xC0:  // CV Mapper 2KB
      readSegment_ATARI(0x1800, 0x2000);
      break;

    case 0xD0:  // DPC Mapper 10KB
      // 8K ROM
      for (int x = 0; x < 0x2; x++) {
        readData_ATARI(0x1FF8 + x);
        // Split Read of 1st 0x200 bytes
        // 0x0000-0x0080 are DPC Registers (Random on boot)
        for (int y = 0; y < 0x80; y++) {
          sdBuffer[y] = 0xFF;  // Output 0xFFs for Registers
        }
        myFile.write(sdBuffer, 128);
        for (int z = 0; z < 0x180; z++) {
          sdBuffer[z] = readData_ATARI(0x1080 + z);
        }
        myFile.write(sdBuffer, 384);
        // Read Segment
        readSegment_ATARI(0x1200, 0x1800);
        // 0x1000-0x1080 are DPC Registers (Random on boot)
        for (int y = 0; y < 0x80; y++) {
          sdBuffer[y] = 0xFF;  // Output 0xFFs for Registers
        }
        myFile.write(sdBuffer, 128);
        for (int z = 0; z < 0x180; z++) {
          sdBuffer[z] = readData_ATARI(0x1880 + z);
        }
        myFile.write(sdBuffer, 384);
        // Read Segment
        readSegment_ATARI(0x1A00, 0x1E00);
        // Split Read of Last 0x200 bytes
        for (int y = 0; y < 0x1F8; y++) {
          sdBuffer[y] = readData_ATARI(0x1E00 + y);
        }
        myFile.write(sdBuffer, 504);
        for (int z = 0; z < 8; z++) {
          // Set Bank to ensure 0x1FFA-0x1FFF is correct
          readData_ATARI(0x1FF8 + x);
          sdBuffer[z] = readData_ATARI(0x1FF8 + z);
        }
        myFile.write(sdBuffer, 8);
      }

      // 2K DPC Internal Graphics ROM
      // Read Registers 0x1008-0x100F (Graphics 0x1008-0x100C)
      // Write Registers LSB 0x1050-0x1057 AND MSB 0x1058-0x105F

      // Set Data Fetcher 0 Limits
      writeData_ATARI(0x1040, 0xFF);  // MAX for Data Fetcher 0
      writeData_ATARI(0x1048, 0x00);  // MIN for Data Fetcher 0
      // Set Data Fetcher 0 Counter (0x7FF)
      writeData_ATARI(0x1050, 0xFF);  // LSB for Data Fetcher 0
      writeData_ATARI(0x1058, 0x07);  // MSB for Data Fetcher 0
      // Set Data Fetcher 1 Counter (0x7FF)
      writeData_ATARI(0x1051, 0xFF);  // LSB for Data Fetcher 1
      writeData_ATARI(0x1059, 0x07);  // MSB for Data Fetcher 1
      for (int x = 0; x < 0x800; x += 512) {
        for (int y = 0; y < 512; y++) {
          sdBuffer[y] = readData_ATARI(0x1008);  // Data Fetcher 0
          readData_ATARI(0x1009);                // Data Fetcher 1
        }
        myFile.write(sdBuffer, 512);
      }
      break;

    case 0xE0:  // E0 Mapper 8KB
      for (int x = 0; x < 0x7; x++) {
        readData_ATARI(0x1FE0 + x);
        readSegment_ATARI(0x1000, 0x1400);
      }
      readSegment_ATARI(0x1C00, 0x2000);
      break;

    case 0xE7:  // E7 Mapper 16KB
      writeData_ATARI(0x1800, 0xFF);
      for (int x = 0; x < 0x7; x++) {
        readData_ATARI(0x1FE0 + x);
        readSegment_ATARI(0x1000, 0x1800);
      }
      readSegment_ATARI(0x1800, 0x2000);
      break;

    case 0xF0:  // F0 Mapper 64KB
      for (int x = 0; x < 0x10; x++) {
        readData_ATARI(0x1FF0);
        readSegment_ATARI(0x1000, 0x2000);
      }
      break;

    case 0xF4:  // F4 Mapper 32KB
      for (int x = 0; x < 0x8; x++) {
        readData_ATARI(0x1FF4 + x);
        readSegment_ATARI(0x1000, 0x2000);
      }
      break;

    case 0xF6:  // F6 Mapper 16KB
      for (int x = 0; x < 0x4; x++) {
        readData_ATARI(0x1FF6 + x);
        readSegment_ATARI(0x1000, 0x2000);
      }
      break;

    case 0xF8:  // F8 Mapper 8KB
      for (int x = 0; x < 0x2; x++) {
        readData_ATARI(0x1FF8 + x);
        readSegment_ATARI(0x1000, 0x1E00);
        // Split Read of Last 0x200 bytes
        for (int y = 0; y < 0x1F8; y++) {
          sdBuffer[y] = readData_ATARI(0x1E00 + y);
        }
        myFile.write(sdBuffer, 504);
        for (int z = 0; z < 8; z++) {
          // Set Bank to ensure 0x1FFA-0x1FFF is correct
          readData_ATARI(0x1FF8 + x);
          sdBuffer[z] = readData_ATARI(0x1FF8 + z);
        }
        myFile.write(sdBuffer, 8);
      }
      break;

    case 0xFA:  // FA Mapper 12KB
      for (int x = 0; x < 0x3; x++) {
        writeData_ATARI(0x1FF8 + x, 0x1);  // Set Bank with D0 HIGH
        readSegment_ATARI(0x1000, 0x1E00);
        // Split Read of Last 0x200 bytes
        for (int y = 0; y < 0x1F8; y++) {
          sdBuffer[y] = readData_ATARI(0x1E00 + y);
        }
        myFile.write(sdBuffer, 504);
        for (int z = 0; z < 8; z++) {
          // Set Bank to ensure 0x1FFB-0x1FFF is correct
          writeData_ATARI(0x1FF8 + x, 0x1);  // Set Bank with D0 HIGH
          sdBuffer[z] = readData_ATARI(0x1FF8 + z);
        }
        myFile.write(sdBuffer, 8);
      }
      break;

    case 0xFE:  // FE Mapper 8KB
      for (int x = 0; x < 0x2; x++) {
        writeData_ATARI(0x01FE, 0xF0 ^ (x << 5));
        writeData_ATARI(0x01FF, 0xF0 ^ (x << 5));
        readSegment_ATARI(0x1000, 0x2000);
      }
      break;

    case 0x0A:  // UA Mapper 8KB
      readData_ATARI(0x220);
      readSegment_ATARI(0x1000, 0x2000);
      readData_ATARI(0x240);
      readSegment_ATARI(0x1000, 0x2000);
      break;
  }
  myFile.close();

  unsigned long crcsize = ATARI[atarisize] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(F(""));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void checkStatus_ATARI() {
  EEPROM_readAnything(7, atarimapper);
  EEPROM_readAnything(8, atarisize);
  if (atarisize > 6) {
    atarisize = 1;  // default 4KB
    EEPROM_writeAnything(8, atarisize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("ATARI 2600 READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("MAPPER:   "));
  if (atarimapper == 0x20)
    println_Msg(F("2K"));
  else if (atarimapper == 0x40)
    println_Msg(F("4K"));
  else if (atarimapper == 0x0A)
    println_Msg(F("UA"));
  else if (atarimapper == 0xC0)
    println_Msg(F("CV"));
  else if (atarimapper == 0xD0)
    println_Msg(F("DPC"));
  else
    println_Msg(atarimapper, HEX);
  print_Msg(F("ROM SIZE: "));
  if (atarimapper == 0xD0)
    print_Msg(F("10"));
  else
    print_Msg(ATARI[atarisize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("MAPPER:   "));
  if (atarimapper == 0x20)
    Serial.println(F("2K"));
  else if (atarimapper == 0x40)
    Serial.println(F("4K"));
  else if (atarimapper == 0x0A)
    Serial.println(F("UA"));
  else if (atarimapper == 0xC0)
    Serial.println(F("CV"));
  else if (atarimapper == 0xD0)
    Serial.println(F("DPC"));
  else
    Serial.println(atarimapper, HEX);
  Serial.print(F("ROM SIZE: "));
  if (atarimapper == 0xD0)
    Serial.print(F("10"));
  else
    Serial.print(ATARI[atarisize]);
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// SET MAPPER
//******************************************

void setMapper_ATARI() {
#if (defined(enable_OLED) || defined(enable_LCD))
  int b = 0;
  int i = 0;
  // Check Button Status
#if defined(enable_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif

  if (buttonVal1 == LOW) {  // Button Pressed
    while (1) {             // Scroll Mapper List
#if defined(enable_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
      buttonVal1 = (PING & (1 << 2));      //PG2
#endif
      if (buttonVal1 == HIGH) {  // Button Released
        // Correct Overshoot
        if (i == 0)
          i = atarimapcount - 1;
        else
          i--;
        break;
      }
      display_Clear();
      print_Msg(F("Mapper: "));
      atariindex = i * 2;
      atarimapselect = pgm_read_byte(atarimapsize + atariindex);
      if (atarimapselect == 0x20)
        println_Msg(F("2K"));
      else if (atarimapselect == 0x40)
        println_Msg(F("4K"));
      else if (atarimapselect == 0x0A)
        println_Msg(F("UA"));
      else if (atarimapselect == 0xC0)
        println_Msg(F("CV"));
      else if (atarimapselect == 0xD0)
        println_Msg(F("DPC"));
      else
        println_Msg(atarimapselect, HEX);
      display_Update();
      if (i == (atarimapcount - 1))
        i = 0;
      else
        i++;
      delay(250);
    }
  }

  display_Clear();
  print_Msg(F("Mapper: "));
  atariindex = i * 2;
  atarimapselect = pgm_read_byte(atarimapsize + atariindex);
  if (atarimapselect == 0x20)
    println_Msg(F("2K"));
  else if (atarimapselect == 0x40)
    println_Msg(F("4K"));
  else if (atarimapselect == 0x0A)
    println_Msg(F("UA"));
  else if (atarimapselect == 0xC0)
    println_Msg(F("CV"));
  else if (atarimapselect == 0xD0)
    println_Msg(F("DPC"));
  else
    println_Msg(atarimapselect, HEX);
  println_Msg(F(""));
#if defined(enable_OLED)
  print_STR(press_to_change_STR, 1);
  print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
  print_STR(rotate_to_change_STR, 1);
  print_STR(press_to_select_STR, 1);
#endif
  display_Update();

  while (1) {
    b = checkButton();
    if (b == 2) {  // Previous Mapper (doubleclick)
      if (i == 0)
        i = atarimapcount - 1;
      else
        i--;

      // Only update display after input because of slow LCD library
      display_Clear();
      print_Msg(F("Mapper: "));
      atariindex = i * 2;
      atarimapselect = pgm_read_byte(atarimapsize + atariindex);
      if (atarimapselect == 0x20)
        println_Msg(F("2K"));
      else if (atarimapselect == 0x40)
        println_Msg(F("4K"));
      else if (atarimapselect == 0x0A)
        println_Msg(F("UA"));
      else if (atarimapselect == 0xC0)
        println_Msg(F("CV"));
      else if (atarimapselect == 0xD0)
        println_Msg(F("DPC"));
      else
        println_Msg(atarimapselect, HEX);
      println_Msg(F(""));
#if defined(enable_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();
    }
    if (b == 1) {  // Next Mapper (press)
      if (i == (atarimapcount - 1))
        i = 0;
      else
        i++;

      // Only update display after input because of slow LCD library
      display_Clear();
      print_Msg(F("Mapper: "));
      atariindex = i * 2;
      atarimapselect = pgm_read_byte(atarimapsize + atariindex);
      if (atarimapselect == 0x20)
        println_Msg(F("2K"));
      else if (atarimapselect == 0x40)
        println_Msg(F("4K"));
      else if (atarimapselect == 0x0A)
        println_Msg(F("UA"));
      else if (atarimapselect == 0xC0)
        println_Msg(F("CV"));
      else if (atarimapselect == 0xD0)
        println_Msg(F("DPC"));
      else
        println_Msg(atarimapselect, HEX);
      println_Msg(F(""));
#if defined(enable_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();
    }
    if (b == 3) {  // Long Press - Execute (hold)
      newatarimapper = atarimapselect;
      break;
    }
  }
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  if (newatarimapper == 0x20)
    println_Msg(F("2K"));
  else if (newatarimapper == 0x40)
    println_Msg(F("4K"));
  if (newatarimapper == 0x0A)
    print_Msg(F("UA"));
  else if (newatarimapper == 0xC0)
    print_Msg(F("CV"));
  else if (newatarimapper == 0xD0)
    println_Msg(F("DPC"));
  else
    print_Msg(newatarimapper, HEX);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  Serial.println(F("SUPPORTED MAPPERS:"));
  Serial.println(F("0 = 2K [Standard 2K]"));
  Serial.println(F("1 = 3F [Tigervision]"));
  Serial.println(F("2 = 4K [Standard 4K]"));
  Serial.println(F("3 = CV [Commavid]"));
  Serial.println(F("4 = DPC [Pitfall II]"));
  Serial.println(F("5 = E0 [Parker Bros]"));
  Serial.println(F("6 = E7 [M-Network]"));
  Serial.println(F("7 = F0 [Megaboy]"));
  Serial.println(F("8 = F4 [Atari 32K]"));
  Serial.println(F("9 = F6 [Atari 16K]"));
  Serial.println(F("10 = F8 [Atari 8K]"));
  Serial.println(F("11 = FA [CBS RAM Plus]"));
  Serial.println(F("12 = FE [Activision]"));
  Serial.println(F("13 = UA [UA Ltd]"));
  Serial.print(F("Enter Mapper [0-13]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  atariindex = newmap.toInt() * 2;
  newatarimapper = pgm_read_byte(atarimapsize + atariindex);
#endif
  EEPROM_writeAnything(7, newatarimapper);
  atarimapper = newatarimapper;

  atarisize = pgm_read_byte(atarimapsize + atariindex + 1);
  EEPROM_writeAnything(8, atarisize);
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile ataricsvFile;
char atarigame[36];                     // title
char atarimm[4];                        // mapper
char atarill[4];                        // linelength (previous line)
unsigned long ataricsvpos;              // CSV File Position
char ataricartCSV[] = "ataricart.txt";  // CSV List
char ataricsvEND[] = "EOF";             // CSV End Marker for scrolling

bool readLine_ATARI(FsFile& f, char* line, size_t maxLen) {
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

bool readVals_ATARI(char* atarigame, char* atarimm, char* atarill) {
  char line[42];
  ataricsvpos = ataricsvFile.position();
  if (!readLine_ATARI(ataricsvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(atarigame, comma);
    else if (x == 1)
      strcpy(atarimm, comma);
    else if (x == 2)
      strcpy(atarill, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_ATARI() {
  bool buttonreleased = 0;
  bool cartselected = 0;
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F(" HOLD TO FAST CYCLE"));
  display_Update();
#else
  Serial.println(F("HOLD BUTTON TO FAST CYCLE"));
#endif
  delay(2000);
#if defined(enable_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
  if (buttonVal1 == LOW) {  // Button Held - Fast Cycle
    while (1) {             // Scroll Game List
      while (readVals_ATARI(atarigame, atarimm, atarill)) {
        if (strcmp(ataricsvEND, atarigame) == 0) {
          ataricsvFile.seek(0);  // Restart
        } else {
#if (defined(enable_OLED) || defined(enable_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(F(""));
          println_Msg(atarigame);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(atarigame);
#endif
#if defined(enable_OLED)
          buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
          buttonVal1 = (PING & (1 << 2));  //PG2
#endif
          if (buttonVal1 == HIGH) {  // Button Released
            buttonreleased = 1;
            break;
          }
          if (buttonreleased) {
            buttonreleased = 0;  // Reset Flag
            break;
          }
        }
      }
#if defined(enable_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
      buttonVal1 = (PING & (1 << 2));      //PG2
#endif
      if (buttonVal1 == HIGH)  // Button Released
        break;
    }
  }
#if (defined(enable_OLED) || defined(enable_LCD))
  display.setCursor(0, 56);
  println_Msg(F("FAST CYCLE OFF"));
  display_Update();
#else
  Serial.println(F(""));
  Serial.println(F("FAST CYCLE OFF"));
  Serial.println(F("PRESS BUTTON TO STEP FORWARD"));
  Serial.println(F("DOUBLE CLICK TO STEP BACK"));
  Serial.println(F("HOLD TO SELECT"));
  Serial.println(F(""));
#endif
  while (readVals_ATARI(atarigame, atarimm, atarill)) {
    if (strcmp(ataricsvEND, atarigame) == 0) {
      ataricsvFile.seek(0);  // Restart
    } else {
#if (defined(enable_OLED) || defined(enable_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(F(""));
      println_Msg(atarigame);
      display.setCursor(0, 48);
#if defined(enable_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();
#else
      Serial.print(F("CART TITLE:"));
      Serial.println(atarigame);
#endif
      while (1) {  // Single Step
        int b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(atarill, NULL, 10);
          ataricsvpos -= prevline;
          ataricsvFile.seek(ataricsvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          newatarimapper = strtol(atarimm, NULL, 10);
          EEPROM_writeAnything(7, newatarimapper);
          cartselected = 1;  // SELECTION MADE
#if (defined(enable_OLED) || defined(enable_LCD))
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
#if (defined(enable_OLED) || defined(enable_LCD))
  println_Msg(F(""));
  println_Msg(F("END OF FILE"));
  display_Update();
#else
  Serial.println(F("END OF FILE"));
#endif

  return false;
}

void checkCSV_ATARI() {
  if (getCartListInfo_ATARI()) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(F(""));
    println_Msg(atarigame);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: "));
    println_Msg(newatarimapper, HEX);
    display_Update();
#else
    Serial.println(F(""));
    Serial.println(F("CART SELECTED"));
    Serial.println(atarigame);
    // Display Settings
    Serial.print(F("CODE: "));
    Serial.println(newatarimapper, HEX);
    Serial.println(F(""));
#endif
  } else {
#if (defined(enable_OLED) || defined(enable_LCD))
    display.setCursor(0, 56);
    println_Msg(F("NO SELECTION"));
    display_Update();
#else
    Serial.println(F("NO SELECTION"));
#endif
  }
}

void checkSize_ATARI() {
  EEPROM_readAnything(7, atarimapper);
  for (int i = 0; i < atarimapcount; i++) {
    atariindex = i * 2;
    if (atarimapper == pgm_read_byte(atarimapsize + atariindex)) {
      atarisize = pgm_read_byte(atarimapsize + atariindex + 1);
      EEPROM_writeAnything(8, atarisize);
      break;
    }
  }
}

void setCart_ATARI() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(ataricartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "ATARI/CSV");
  sd.chdir(folder);  // Switch Folder
  ataricsvFile = sd.open(ataricartCSV, O_READ);
  if (!ataricsvFile) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_ATARI();
    }
  }
  checkCSV_ATARI();
  ataricsvFile.close();

  checkSize_ATARI();
}
#endif
//******************************************
// End of File
//******************************************