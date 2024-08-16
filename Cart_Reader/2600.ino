//******************************************
// ATARI 2600 MODULE
//******************************************
#if defined(ENABLE_2600)
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
static const byte PROGMEM a2600mapsize[] = {
  0x04, 5,  // Atari 32K with RAM (F4SC)
  0x06, 4,  // Atari 16K with RAM (F6SC)
  0x08, 2,  // Atari 8K with RAM (F8SC)
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
  0xF9, 2,  // "TP" Time Pilot 8K
  0x0A, 2,  // "UA" UA Ltd 8K
  0x3E, 5,  // Tigervision 32K with 32K RAM
  0x07, 6,  // X07 64K ROM
};

byte a2600mapcount = (sizeof(a2600mapsize) / sizeof(a2600mapsize[0])) / 2;
byte a2600mapselect;
int a2600index;

byte a2600[] = { 2, 4, 8, 12, 16, 32, 64 };
byte a2600mapper = 0;
byte a2600size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char a2600MenuItem3[] PROGMEM = "Set Mapper";
static const char* const menuOptions2600[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, a2600MenuItem3, FSTRING_RESET };

void setup_2600() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

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

  checkStatus_2600();
  strcpy(romName, "ATARI");

  mode = CORE_2600;
}

void a2600Menu() {
  convertPgm(menuOptions2600, 4);
  uint8_t mainMenu = question_box(F("ATARI 2600 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_2600();
      setup_2600();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_2600();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper
      setMapper_2600();
      checkStatus_2600();
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

uint8_t readData_2600(uint16_t addr)  // Add Input Pullup
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

void readSegment_2600(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    readDataArray_2600(addr, 512);
  }
}

void readDataArray_2600(uint16_t addr, uint16_t size) {
    for (uint16_t w = 0; w < size; w++) {
      sdBuffer[w] = readData_2600(addr + w);
    }
    myFile.write(sdBuffer, size);
}

void readSegmentF8_2600(uint16_t startaddr, uint16_t endaddr, uint16_t bankaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (uint16_t w = 0; w < 512; w++) {
      if (addr > 0x1FF9)  // SET BANK ADDRESS FOR 0x1FFA-0x1FFF
        readData_2600(bankaddr);
      uint8_t temp = readData_2600(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readSegmentE7_2600(uint8_t start, uint8_t end) {
  for (uint8_t x = start; x <= end; x++) {
    readData_2600(0x1FE0 + x);
    readSegment_2600(0x1000, 0x1800);
  }
}

void readSegmentFx_2600(bool hasRAM, uint16_t size) {
  if(hasRAM) {
    outputFF_2600(0x100); // Skip 0x1000-0x10FF RAM
    readDataArray_2600(0x1100, 0x100);
  } else {
    readSegment_2600(0x1000, 0x1200);
  }
  readSegment_2600(0x1200, 0x1E00);
  // Split Read of Last 0x200 bytes
  readDataArray_2600(0x1E00, size);
}

void readSegmentTigervision_2600(uint8_t banks) {
  for (uint8_t x = 0; x < banks; x++) {
    writeData3F_2600(0x3F, x);
    readSegment_2600(0x1000, 0x1800);
  }
  readSegment_2600(0x1800, 0x2000);
}

void outputFF_2600(uint16_t size) {
  memset(sdBuffer, 0xFF, size * sizeof(sdBuffer[0]));
  myFile.write(sdBuffer, size);
}

void writeData_2600(uint16_t addr, uint8_t data) {
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

void writeData3F_2600(uint16_t addr, uint8_t data) {
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

// E7 Mapper Check - Check Bank for FFs
boolean checkE7(uint16_t bank) {
  writeData_2600(0x1800, 0xFF);
  readData_2600(0x1FE0 + bank);
  uint32_t testdata = ((uint32_t)readData_2600(0x1000) << 24) | ((uint32_t)readData_2600(0x1001) << 16) | (readData_2600(0x1002) << 8) | (readData_2600(0x1003));
  return (testdata == 0xFFFFFFFF);
}

void readROM_2600() {
  createFolderAndOpenFile("ATARI", "ROM", romName, "a26");

  // ROM Start 0xF000
  // Address A12-A0 = 0x1000 = 1 0000 0000 0000 = 4KB
  // Read Start 0x1000

  switch (a2600mapper) {
    case 0x20:  // 2K Standard 2KB
      readSegment_2600(0x1000, 0x1800);
      break;

    case 0x3F:  // 3F Mapper 8KB
      readSegmentTigervision_2600(3);
      break;

    case 0x3E:  // 3E Mapper 32KB ROM 32K RAM
      readSegmentTigervision_2600(15);
      break;    

    case 0x40:  // 4K Default 4KB
      readSegment_2600(0x1000, 0x2000);
      break;

    case 0xC0:  // CV Mapper 2KB
      readSegment_2600(0x1800, 0x2000);
      break;

    case 0xD0:  // DPC Mapper 10KB
      // 8K ROM
      for (int x = 0; x < 0x2; x++) {
        readData_2600(0x1FF8 + x);
        // Split Read of 1st 0x200 bytes
        // 0x0000-0x0080 are DPC Registers (Random on boot)
        outputFF_2600(0x80); // Output 0xFFs for Registers
        readDataArray_2600(0x1080, 0x180);
        // Read Segment
        readSegment_2600(0x1200, 0x1800);
        // 0x1000-0x1080 are DPC Registers (Random on boot)
        outputFF_2600(0x80); // Output 0xFFs for Registers
        readDataArray_2600(0x1880, 0x180);
        // Read Segment
        readSegment_2600(0x1A00, 0x1E00);
        // Split Read of Last 0x200 bytes
        readDataArray_2600(0x1E00, 0x1F8);
        for (int z = 0; z < 8; z++) {
          // Set Bank to ensure 0x1FFA-0x1FFF is correct
          readData_2600(0x1FF8 + x);
          sdBuffer[z] = readData_2600(0x1FF8 + z);
        }
        myFile.write(sdBuffer, 8);
      }

      // 2K DPC Internal Graphics ROM
      // Read Registers 0x1008-0x100F (Graphics 0x1008-0x100C)
      // Write Registers LSB 0x1050-0x1057 AND MSB 0x1058-0x105F

      // Set Data Fetcher 0 Limits
      writeData_2600(0x1040, 0xFF);  // MAX for Data Fetcher 0
      writeData_2600(0x1048, 0x00);  // MIN for Data Fetcher 0
      // Set Data Fetcher 0 Counter (0x7FF)
      writeData_2600(0x1050, 0xFF);  // LSB for Data Fetcher 0
      writeData_2600(0x1058, 0x07);  // MSB for Data Fetcher 0
      // Set Data Fetcher 1 Counter (0x7FF)
      writeData_2600(0x1051, 0xFF);  // LSB for Data Fetcher 1
      writeData_2600(0x1059, 0x07);  // MSB for Data Fetcher 1
      for (int x = 0; x < 0x800; x += 512) {
        for (int y = 0; y < 512; y++) {
          sdBuffer[y] = readData_2600(0x1008);  // Data Fetcher 0
          readData_2600(0x1009);                // Data Fetcher 1
        }
        myFile.write(sdBuffer, 512);
      }
      break;

    case 0xE0:  // E0 Mapper 8KB
      for (int x = 0; x < 0x7; x++) {
        readData_2600(0x1FE0 + x);
        readSegment_2600(0x1000, 0x1400);
      }
      readSegment_2600(0x1C00, 0x2000);
      break;

    case 0xE7: // E7 Mapper 8KB/12KB/16KB
      writeData_2600(0x1800, 0xFF);
      // Check Bank 0 - If 0xFFs then Bump 'n' Jump
      if (checkE7(0)) { // Bump 'n' Jump 8K
        readSegmentE7_2600(4, 6); // Banks 4-6
      }
      // Check Bank 3 - If 0xFFs then BurgerTime
      else if (checkE7(3)) { // BurgerTime 12K
        readSegmentE7_2600(0, 1); // Banks 0+1
        readSegmentE7_2600(4, 6); // Banks 4-6
      }
      else { // Masters of the Universe (or Unknown Cart) 16K
        readSegmentE7_2600(0, 6); // Banks 0-6
      }
      readSegment_2600(0x1800, 0x2000); // Bank 7
      break;

    case 0xF0:  // F0 Mapper 64KB
      for (int x = 0; x < 0x10; x++) {
        readData_2600(0x1FF0);
        readSegment_2600(0x1000, 0x2000);
      }
      break;

    case 0x04: // F4SC Mapper 32KB \w RAM
    case 0xF4: // F4 Mapper 32KB
      for (int x = 0; x < 8; x++) {
        readData_2600(0x1FF4 + x);
        readSegmentFx_2600(a2600mapper == 0x04, 0x1F4);
        for (int z = 0; z < 12; z++) {
          // Set Bank to ensure 0x1FFC-0x1FFF is correct
          readData_2600(0x1FF4 + x);
          sdBuffer[z] = readData_2600(0x1FF4 + z);
        }
        myFile.write(sdBuffer, 12);
      }
      break;

    case 0x06: // F6SC Mapper 16KB \w RAM
    case 0xF6: // F6 Mapper 16KB
      for (int w = 0; w < 4; w++) {
        readData_2600(0x1FF6 + w);
        readSegmentFx_2600(a2600mapper == 0x06, 0x1F6);
        // Bank Registers 0x1FF6-0x1FF9
        for (int y = 0; y < 4; y++){
          readData_2600(0x1FFF); // Reset Bank
          sdBuffer[y] = readData_2600(0x1FF6 + y);
        }
        // End of Bank 0x1FFA-0x1FFF
        readData_2600(0x1FFF); // Reset Bank
        readData_2600(0x1FF6 + w); // Set Bank
        for (int z = 4; z < 10; z++) {
          sdBuffer[z] = readData_2600(0x1FF6 + z); // 0x1FFA-0x1FFF
        }
        myFile.write(sdBuffer, 10);
      }
      readData_2600(0x1FFF); // Reset Bank
      break;

    case 0x08: // F8SC Mapper 8KB \w RAM
    case 0xF8: // F8 Mapper 8KB
      for (int w = 0; w < 2; w++) {
        readData_2600(0x1FF8 + w);
        readSegmentFx_2600(a2600mapper == 0x08, 0x1F8);
        // Bank Registers 0x1FF8-0x1FF9
        for (int y = 0; y < 2; y++){
          readData_2600(0x1FFF); // Reset Bank
          sdBuffer[y] = readData_2600(0x1FF8 + y);
        }
        // End of Bank 0x1FFA-0x1FFF
        readData_2600(0x1FFF); // Reset Bank
        readData_2600(0x1FF8 + w); // Set Bank
        for (int z = 2; z < 8; z++) {
          sdBuffer[z] = readData_2600(0x1FF8 + z); // 0x1FFA-0x1FFF
        }
        myFile.write(sdBuffer, 8);
      }
      readData_2600(0x1FFF); // Reset Bank
      break;

    case 0xF9: // Time Pilot Mapper 8KB
      // Bad implementation of the F8 Mapper
      // kevtris swapped the bank order - swapped banks may not match physical ROM data
      // Bankswitch code uses 0x1FFC and 0x1FF9
      for (int w = 3; w >= 0; w -= 3) {
        readData_2600(0x1FF9 + w);
        readSegment_2600(0x1000, 0x1E00);
        // Split Read of Last 0x200 bytes
        readDataArray_2600(0x1E00, 0x1F9);
        readData_2600(0x1FFF); // Reset Bank
        sdBuffer[0] = readData_2600(0x1FF9);
        // End of Bank 0x1FFA-0x1FFF
        readData_2600(0x1FFF); // Reset Bank
        readData_2600(0x1FF9 + w); // Set Bank
        for (int z = 1; z < 7; z++) {
          sdBuffer[z] = readData_2600(0x1FF9 + z); // 0x1FFA-0x1FFF
        }
        myFile.write(sdBuffer, 7);
      }
      // Reset Bank
      readData_2600(0x1FF9);
      readData_2600(0x1FFF);
      readData_2600(0x1FFC);
      break;

    case 0xFA:  // FA Mapper 12KB
      for (int x = 0; x < 0x3; x++) {
        writeData_2600(0x1FF8 + x, 0x1);  // Set Bank with D0 HIGH
        readSegment_2600(0x1000, 0x1E00);
        // Split Read of Last 0x200 bytes
        readDataArray_2600(0x1E00, 0x1F8);
        for (int z = 0; z < 8; z++) {
          // Set Bank to ensure 0x1FFB-0x1FFF is correct
          writeData_2600(0x1FF8 + x, 0x1);  // Set Bank with D0 HIGH
          sdBuffer[z] = readData_2600(0x1FF8 + z);
        }
        myFile.write(sdBuffer, 8);
      }
      break;

    case 0xFE:  // FE Mapper 8KB
      for (int x = 0; x < 0x2; x++) {
        writeData_2600(0x01FE, 0xF0 ^ (x << 5));
        writeData_2600(0x01FF, 0xF0 ^ (x << 5));
        readSegment_2600(0x1000, 0x2000);
      }
      break;

    case 0x0A:  // UA Mapper 8KB
      readData_2600(0x220);
      readSegment_2600(0x1000, 0x2000);
      readData_2600(0x240);
      readSegment_2600(0x1000, 0x2000);
      break;

    case 0x07:  // X07 Mapper 64K
      for (int x = 0; x < 16; x++) {
        readData_2600(0x080D | (x << 4));
        readSegment_2600(0x1000, 0x2000);
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

void println_Mapper2600(byte mapper) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  if (mapper == 0x04)
    println_Msg(F("F4SC"));
  else if (mapper == 0x06)
    println_Msg(F("F6SC"));
  else if (mapper == 0x08)
    println_Msg(F("F8SC"));
  else if (mapper == 0x20)
    println_Msg(F("2K"));
  else if (mapper == 0x40)
    println_Msg(F("4K"));
  else if (mapper == 0x0A)
    println_Msg(F("UA"));
  else if (mapper == 0xC0)
    println_Msg(F("CV"));
  else if (mapper == 0xD0)
    println_Msg(F("DPC"));
  else if (mapper == 0xF9)
    println_Msg(F("TP"));
  else if (mapper == 0x07)
    println_Msg(F("X07"));
  else
    println_Msg(mapper, HEX);
#else
  if (mapper == 0x04)
    Serial.println(F("F4SC"));
  else if (mapper == 0x06)
    Serial.println(F("F6SC"));
  else if (mapper == 0x08)
    Serial.println(F("F8SC"));
  else if (mapper == 0x20)
    Serial.println(F("2K"));
  else if (mapper == 0x40)
    Serial.println(F("4K"));
  else if (mapper == 0x0A)
    Serial.println(F("UA"));
  else if (mapper == 0xC0)
    Serial.println(F("CV"));
  else if (mapper == 0xD0)
    Serial.println(F("DPC"));
  else if (mapper == 0xF9)
    Serial.println(F("TP"));
  else if (mapper == 0x07)
    Serial.println(F("X07"));
  else
    Serial.println(mapper, HEX);
#endif
}

void checkStatus_2600() {
  EEPROM_readAnything(7, a2600mapper);
  EEPROM_readAnything(8, a2600size);
  if (a2600size > 6) {
    a2600size = 1;  // default 4KB
    EEPROM_writeAnything(8, a2600size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("ATARI 2600 READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Mapper2600(a2600mapper);
  print_Msg(FS(FSTRING_ROM_SIZE));
  if (a2600mapper == 0xD0)
    print_Msg(F("10"));
  else
    print_Msg(a2600[a2600size]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("MAPPER:   "));
  println_Mapper2600(a2600mapper);

  Serial.print(FS(FSTRING_ROM_SIZE));
  if (a2600mapper == 0xD0)
    Serial.print(F("10"));
  else
    Serial.print(a2600[a2600size]);
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// SET MAPPER
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printMapperSelection_2600(int index) {
  display_Clear();
  print_Msg(FS(FSTRING_MAPPER));
  a2600index = index * 2;
  a2600mapselect = pgm_read_byte(a2600mapsize + a2600index);
  println_Mapper2600(a2600mapselect);
}
#endif

void setMapper_2600() {
  byte new2600mapper;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  navigateMenu(0, a2600mapcount - 1, &printMapperSelection_2600);
  new2600mapper = a2600mapselect;

  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  println_Mapper2600(new2600mapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  Serial.println(F("SUPPORTED MAPPERS:"));
  Serial.println(F("0 = F4SC [Atari 32K \w RAM]"));
  Serial.println(F("1 = F6SC [Atari 16K \w RAM]"));
  Serial.println(F("2 = F8SC [Atari 8K \w RAM]"));
  Serial.println(F("3 = 2K [Standard 2K]"));
  Serial.println(F("4 = 3F [Tigervision 8K]"));
  Serial.println(F("5 = 4K [Standard 4K]"));
  Serial.println(F("6 = CV [Commavid]"));
  Serial.println(F("7 = DPC [Pitfall II]"));
  Serial.println(F("8 = E0 [Parker Bros]"));
  Serial.println(F("9 = E7 [M-Network]"));
  Serial.println(F("10 = F0 [Megaboy]"));
  Serial.println(F("11 = F4 [Atari 32K]"));
  Serial.println(F("12 = F6 [Atari 16K]"));
  Serial.println(F("13 = F8 [Atari 8K]"));
  Serial.println(F("14 = FA [CBS RAM Plus]"));
  Serial.println(F("15 = FE [Activision]"));
  Serial.println(F("16 = TP [Time Pilot 8K]"));
  Serial.println(F("17 = UA [UA Ltd]"));
  Serial.println(F("18 = 3E [Tigervision 32K \w RAM]"));
  Serial.println(F("19 = 07 [X07 64K]"));
  Serial.print(F("Enter Mapper [0-19]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  a2600index = newmap.toInt() * 2;
  new2600mapper = pgm_read_byte(a2600mapsize + a2600index);
#endif
  EEPROM_writeAnything(7, new2600mapper);
  a2600mapper = new2600mapper;

  a2600size = pgm_read_byte(a2600mapsize + a2600index + 1);
  EEPROM_writeAnything(8, a2600size);
}

//******************************************
// CART SELECT CODE
//******************************************
void readDataLine_2600(FsFile& database, void* gameMapper) {
  // Read mapper with three ascii character and subtract 48 to convert to decimal
  (*(byte*)gameMapper) = ((database.read() - 48) * 100) + ((database.read() - 48) * 10) + (database.read() - 48);

  // Skip rest of line
  database.seekCur(2);
}

void setCart_2600() {
  //go to root
  sd.chdir();

  byte gameMapper;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("2600.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLine_2600, &gameMapper)) {
      EEPROM_writeAnything(7, gameMapper);
      for (int i = 0; i < a2600mapcount; i++) {
        a2600index = i * 2;
        if (gameMapper == pgm_read_byte(a2600mapsize + a2600index)) {
          a2600size = pgm_read_byte(a2600mapsize + a2600index + 1);
          EEPROM_writeAnything(8, a2600size);
          break;
        }
      }
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
//******************************************
// End of File
//******************************************
