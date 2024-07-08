//******************************************
// TEXAS INSTRUMENTS TI-99 MODULE
//******************************************
#ifdef ENABLE_TI99
// Texas Instruments TI-99
// Cartridge Pinout
// 36P 2.54mm pitch connector
//
//                   RIGHT
//                 +-------+
//            Vss -| 36 35 |- GND
//          ROMS* -| 34 33 |- Vss
//            WE* -| 32 31 |- GR
//             A4 -| 30 29 |- -5V     B
//             A5 -| 28 27 |- GRC     O
//    T        A6 -| 26 25 |- DBIN    T
//    O        A3 -| 24 23 |- A14     T
//    P        A7 -| 22 21 |- GS*     O
//             A8 -| 20 19 |- +5V     M
//    S        A9 -| 18 17 |- D0
//    I       A10 -| 16 15 |- D1      S
//    D       A11 -| 14 13 |- D2      I
//    E       A12 -| 12 11 |- D3      D
//            A13 -| 10 9  |- D4      E
//        A15/OUT -|  8 7  |- D5
//          CRUIN -|  6 5  |- D6
//        CRUCLK* -|  4 3  |- D7
//            GND -|  2 1  |- RESET
//                 +-------+
//                   LEFT
//
//                                         TOP SIDE
//
//             CRU CRU A15/
//         GND CLK* IN OUT A13 A12 A11 A10  A9  A8  A7  A3  A6  A5  A4 WE* ROMS* VSS
//       +--------------------------------------------------------------------------+
//       |   2   4   6   8  10  12  14  16  18  20  22  24  26  28  30  32  34  36  |
// LEFT  |                                                                          | RIGHT
//       |   1   3   5   7   9  11  13  15  17  19  21  23  25  27  29  31  33  35  |
//       +--------------------------------------------------------------------------+
//         RST  D7  D6  D5  D4  D3  D2  D1  D0 +5V GS* A14 DBIN GRC -5V GR VSS GND  
//
//                                        BOTTOM SIDE

// CONTROL PINS:
// RESET(PH0) - SNES RESET
// GRC(PH1)   - SNES CPUCLK [GROM CLOCK]
// /GS(PH3)   - SNES /CS    [GROM SELECT]
// DBIN(PH4)  - SNES /IRQ   [READ MEMORY/M(GROM DIRECTION)]
// /WE(PH5)   - SNES /WR
// /ROMS(PH6) - SNES /RD    [ROM SELECT]
// GR(PL0)    - SNES A16    [GROM READY]

// /GS = LOW FOR ADDR $9800-$9FFF
// /ROMS = LOW FOR ADDR $6000-$7FFF

// NOTE: TI-99 ADDRESS AND DATA BUS ARE BIG-ENDIAN
// LEAST SIGNIFICANT IS BIT 7 AND MOST SIGNIFICANT IS BIT 0
// PCB ADAPTER WIRED FOR DIFFERENCE

// FOR GROM-ONLY BOARDS, TOP PINS DO NOT EXIST
// DATA PINS ARE MULTIPLEXED TO HANDLE BOTH ADDRESS AND DATA
// D7-D0 = AD7-AD0

//******************************************
// DEFINES
//******************************************
#define DISABLE_GROM  PORTH |= (1 << 3) // GROM SELECT 9800-9FFF
#define ENABLE_GROM   PORTH &= ~(1 << 3)
#define DISABLE_ROM PORTH |= (1 << 6) // ROM SELECT 6000-7FFF
#define ENABLE_ROM  PORTH &= ~(1 << 6)

#define GROM_WRITE PORTH &= ~(1 << 4) // DBIN/M LOW
#define GROM_READ  PORTH |= (1 << 4) // DBIN/M HIGH
#define GROM_DATA PORTF &= ~(1 << 1) // A14/MO LOW
#define GROM_ADDR PORTF |= (1 << 1); // A14/MO HIGH

#define GRC_HI PORTH |= (1 << 1)
#define GRC_LOW PORTH &= ~(1 << 1)

//******************************************
// VARIABLES
//******************************************
// Cart Configurations
// Format = {mapper,gromlo,gromhi,romlo,romhi}
static const byte PROGMEM ti99mapsize [] = {
0,0,5,0,4, // Normal Carts (GROM 0K/6K/12K/18K/24K/30K + ROM 0K/4K/8K/12K/16K)
1,1,3,4,4, // MBX (GROM 6K/12K/18K + ROM 16K)
2,1,1,4,4, // TI-CALC (GROM 6K + ROM 16K)
};

byte ti99mapcount = 3; // (sizeof(mapsize)/sizeof(mapsize[0])) / 5;
boolean ti99mapfound = false;
byte ti99mapselect;
int ti99index;

//int GROM[] = {0,8,16,24,32,40}; // padded sizes
int GROM[] = {0,6,12,18,24,30};
byte gromlo = 0; // Lowest Entry
byte gromhi = 5; // Highest Entry

int CROM[] = {0,4,8,12,16};
byte cromlo = 0; // Lowest Entry
byte cromhi = 4; // Highest Entry

byte ti99mapper;
byte newti99mapper;
byte gromsize;
byte newgromsize;
byte cromsize;
byte newcromsize;
byte grommap;
byte newgrommap;

//boolean MBX = 0; // Normal/MBX

// EEPROM MAPPING
// 07 MAPPER
// 08 GROM SIZE
// 09 CROM SIZE
// 13 GROM MAP

//******************************************
// MENU
//******************************************
// Base Menu
static const char ti99MenuItem2[] PROGMEM = "Read Complete Cart";
static const char ti99MenuItem3[] PROGMEM = "Read GROM";
static const char ti99MenuItem6[] PROGMEM = "Set GROM Map";
static const char* const menuOptionsTI99[] PROGMEM = { FSTRING_SELECT_CART, ti99MenuItem2, ti99MenuItem3, FSTRING_READ_ROM, FSTRING_SET_SIZE, ti99MenuItem6, FSTRING_RESET };

void ti99Menu()
{
  convertPgm(menuOptionsTI99, 7);
  uint8_t mainMenu = question_box(F("TI-99 MENU"), menuOptions, 7, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_TI99();
      setup_TI99();
      break;

    case 1:
      // Read Complete Cart
      CreateROMFolder_TI99();
      readGROM_TI99();
      readCROM_TI99();
      FinishROMFolder_TI99();
      break;

    case 2:
      // Read GROM
      CreateROMFolder_TI99();
      readGROM_TI99();
      FinishROMFolder_TI99();
      break;

    case 3:
      // Read ROM
      CreateROMFolder_TI99();
      readCROM_TI99();
      FinishROMFolder_TI99();
      break;

    case 4:
      // Set Mapper + Sizes
      setMapper_TI99();
      checkMapperSize_TI99();
      setGROMSize_TI99();
      setCROMSize_TI99();
      break;

    case 5:
      // Set GROM Map
      setGROMMap_TI99();
      break;

    case 6:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_TI99()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // TI-99 uses A0-A15
  // SNES A0-A7 [TI-99 A15-A8]
  DDRF = 0xFF;
  // SNES A8-A15 [TI-99 A7-A0]
  DDRK = 0xFF;
  // SNES A16-A23 - Use A16 for GR
  DDRL = 0xFE; // A16 to Input

  // Set Control Pins to Output
  //       RST(PH0)   GRC(PH1)   /GS(PH3)   DBIN(PH4)  /WE(PH5)  /ROMS(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       RST(PH0)   GRC(PH1)   /GS(PH3)   DBIN(PH4)  /WE(PH5)  /ROMS(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused PORTL Pins HIGH except GR(PL0)
  PORTL |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6)| (1 << 7);

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTJ |= (1 << 0); // TIME(PJ0)

  // Set Reset LOW
  PORTH &= ~(1 << 0);

  checkStatus_TI99();
  strcpy(romName, "TI99");

  mode = CORE_TI99;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readROM_TI99(uint16_t addr) // Add Input Pullup
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // DDRC = 0x00; // Set to Input
  PORTC = 0xFF; // Input Pullup
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

void readSegment_TI99(uint16_t startaddr, uint16_t endaddr)
{
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readROM_TI99(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void setupGROM()
{
  // Setup GROM
  DISABLE_GROM;
  GROM_READ; // DBIN(M) HIGH = READ
  GROM_ADDR; // A14(MO) HIGH = ADDR
  GROM_DATA; // A14(MO) LOW = DATA
}

void pulseGRC(int times)
{
  for (int i = 0; i < (times * 2); i++) {
    PORTH ^= (1 << 1);
    // NOP (62.5ns) x 20 = 1250ns = 1.25us
//    NOP; NOP; NOP; NOP; NOP; // 20 NOPs = 20 x 62.5ns = 1250ns x 2 = 2.5us = 400 kHz
//    NOP; NOP; NOP; NOP; NOP;
//    NOP; NOP; NOP; NOP; NOP;
//    NOP; NOP; NOP; NOP; NOP;
    // Switch to 100 kHz due to some GROMs not reading out properly at faster speeds
    delayMicroseconds(5); // 5us x 2 = 10us = 100 kHz
  }
}

void checkGRC()
{
  byte statusGR = PINL & 0x1;
  pulseGRC(1);
  while (!statusGR) {
    statusGR = PINL & 0x1;
    pulseGRC(1);
  }
}

void pulseGROM(uint16_t addr) // Controlled Pulse Code
{
  // GRC starts here
  GRC_LOW; // START LOW
  pulseGRC(16);
  ENABLE_GROM;
  pulseGRC(8);
  DISABLE_GROM;
  // Write Base
  GROM_WRITE; // DBIN(M) LOW = WRITE
  GROM_ADDR; // A14(MO) HIGH = ADDR
  DDRC = 0xFF; // Output
  PORTC = (addr >> 8) & 0xFF;
  pulseGRC(16);
  ENABLE_GROM;
  pulseGRC(4); // this works
//  checkGRC(); // Disable checkGRC() otherwise Q-bert hangs (GROM 7 at 0xE000 with NO GROMs at 0x6000/0x8000/0xA000/0xC000)
  pulseGRC(2); // Replace checkGRC() with 2 pulse cycles
  pulseGRC(10);
  DISABLE_GROM;
  pulseGRC(16);
  PORTC = addr & 0xFF;
  pulseGRC(16);
  ENABLE_GROM;
  pulseGRC(4); // this works
//  checkGRC(); // Disable checkGRC() otherwise Q-bert hangs (GROM 7 at 0xE000 with NO GROMs at 0x6000/0x8000/0xA000/0xC000)
  pulseGRC(2); // Replace checkGRC() with 2 pulse cycles
  pulseGRC(10);
  DISABLE_GROM;
  pulseGRC(16);
  GROM_READ; // DBIN(M) HIGH = READ
  GROM_DATA; // A14(MO) LOW = DATA
  DDRC = 0x00;
  pulseGRC(16);
}

void readSegmentGROM_TI99(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    pulseGROM(addr); //
    for (int y = 0; y < 512; y++) {
      ENABLE_GROM;
      pulseGRC(2);
      sdBuffer[y] = PINC;
      pulseGRC(6);
      DISABLE_GROM;
      pulseGRC(16);
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// WRITE FUNCTION
//******************************************

void writeData_TI99(uint16_t addr, uint8_t data)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0xFF; // Set to Output
  PORTC = data;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /WE(PH5) to LOW
  PORTH &= ~(1 << 5); // /WE LOW
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /WE(PH5) to HIGH
  PORTH |= (1 << 5);
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0x00; // Reset to Input
}

//******************************************
// ROM FOLDER
//******************************************

void CreateROMFolder_TI99()
{
  sd.chdir();
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "TI99/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);
}

void FinishROMFolder_TI99()
{
  foldern += 1;
  EEPROM_writeAnything(0, foldern); // FOLDER #
  sd.chdir();
}

//******************************************
// READ ROM
//******************************************

// CARTRIDGE GROMS 3-7 (GROMS 0-2 ARE IN THE CONSOLE)
// GROM 3 = $6000-$77FF
// GROM 4 = $8000-$97FF
// GROM 5 = $A000-$B7FF
// GROM 6 = $C000-$D7FF
// GROM 7 = $E000-$F7FF

// GROM ACCESS DETAILS
// INTERNAL POINTER AUTO INCREMENTED EACH TIME CHIP ACCESSED
// SET VALUE OF POINTER BY PASSING TWO BYTES TO THE CHIP

void readGROM_TI99()
{
  display_Clear();
  if (gromsize == 0) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    println_Msg(F("GROM SIZE 0K"));
    display_Update();
#else
    Serial.println(F("GROM SIZE 0K"));
#endif
  }
  else {
//    println_Msg(F("USE GOOD POWER SUPPLY"));
//    println_Msg(FS(FSTRING_EMPTY));
    print_Msg(F("Saving to "));
    print_Msg(folder);
    println_Msg(F("/..."));
    display_Update();

    // Split into Individual GROM Files
    for (int x = 3; x < 8; x++) {
      if (((grommap >> x) & 0x1) == 1) {
        snprintf(fileName, sizeof(fileName), "%s.g%d.bin", romName, x);
        // open file on sdcard
        if (!myFile.open(fileName, O_RDWR | O_CREAT))
          print_Error(F("Can't create file on SD"));

        display_Clear();
        print_Msg(F("Reading GROM "));
        println_Msg(x);
        println_Msg(FS(FSTRING_EMPTY));
        display_Update();

        // Normal GROM 6KB/12KB/18KB/24KB/30K
        // MBX GROM 6KB/12KB/18KB
        DISABLE_ROM;
        ENABLE_GROM;
        setupGROM();
        if (x == 3)
          readSegmentGROM_TI99(0x6000,0x7800); // 6K
        else if (x == 4)
          readSegmentGROM_TI99(0x8000,0x9800); // +6K = 12K
        else if (x == 5)
          readSegmentGROM_TI99(0xA000,0xB800); // +6K = 18K
        else if (x == 6)
          readSegmentGROM_TI99(0xC000,0xD800); // +6K = 24K
        else if (x == 7)
          readSegmentGROM_TI99(0xE000,0xF800); // +6K = 30K
        DISABLE_GROM;
        myFile.close();

        printCRC(fileName, NULL, 0);
      }
    }
  }
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait(); 
}

// CARTRIDGE ROM
// EACH CART UP TO 8K MAPPED AT $6000-$7FFF

void readCROM_TI99()
{
  display_Clear();
  if (cromsize == 0) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    println_Msg(F("ROM SIZE 0K"));
    display_Update();
#else
    Serial.println(F("ROM SIZE 0K"));
#endif
  }
  else {
    print_Msg(F("Saving to "));
    print_Msg(folder);
    println_Msg(F("/..."));
    display_Update();

    snprintf(fileName, sizeof(fileName), "%s.c.bin", romName);
    // open file on sdcard
    if (!myFile.open(fileName, O_RDWR | O_CREAT))
      print_Error(F("Can't create file on SD"));

    if (ti99mapper == 1) { // MBX
      // MBX Cart ROM 16K
      // Fixed Bank 0 at 0x6000 (RAM at 0x6C00)
      // Bankswitch 0x7000-0x7FFF using write 1/2/3 to 0x6FFE
      DISABLE_GROM;
      ENABLE_ROM;
      readSegment_TI99(0x6000, 0x6C00); // 3K
      for (int w = 0; w < 1024; w += 512) { // 1K RAM at 0x6C00
        for (int x = 0; x < 512; x++) {
          sdBuffer[x] = 0xFF; // Replace Random RAM Contents with 0xFF
        }
        myFile.write(sdBuffer, 512);
      }
      for (int y = 1; y < 4; y++) {
        writeData_TI99(0x6FFE, y); // Set Bank 1/2/3
        readSegment_TI99(0x7000, 0x8000); // 4K x 3 = +12K = 16K
      }
    }
    else if (ti99mapper == 2) { // TI-CALC [UNTESTED]
      // TI-CALC ROM 16K
      // Fixed Bank 0 at 0x6000-0x6FFF
      // Bankswitch 0x7000-0x7FFF using write to 0x7000/0x7002/0x7004/0x7006
      DISABLE_GROM;
      ENABLE_ROM;
      writeData_TI99(0x7000, 0x00); // Set Bank 0
      readSegment_TI99(0x7000,0x8000); // 4K
      writeData_TI99(0x7002, 0x00); // Set Bank 1
      readSegment_TI99(0x7000,0x8000); // +4K = 8K
      writeData_TI99(0x7004, 0x00); // Set Bank 2
      readSegment_TI99(0x7000, 0x8000); // +4K = 12K
      writeData_TI99(0x7006, 0x00); // Set Bank 3
      readSegment_TI99(0x7000, 0x8000); // +4K = 16K
    }
    else { 
      // Normal Cart ROM 4K/8K/12K/16K
      // ROM Space 0x6000-0x7FFF
      // Bankswitch 0x7000-0x7FFF using write to 0x6000/0x6002
      DISABLE_GROM;
      ENABLE_ROM;
      writeData_TI99(0x6000, 0x00); // Set Bank 0
      readSegment_TI99(0x6000,0x7000); // 4K
      if (cromsize > 1) {
        readSegment_TI99(0x7000,0x8000); // +4K = 8K
        if (cromsize > 2) {
          writeData_TI99(0x6002, 0x00); // Set Bank 1
          if (cromsize > 3) // 16K
            readSegment_TI99(0x6000, 0x7000); // +4K = 16K
          readSegment_TI99(0x7000, 0x8000); // +4K = 12K
        }
      }
    }
    DISABLE_ROM;
    myFile.close();

    printCRC(fileName, NULL, 0);
  }
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// CHECK STATUS
//******************************************

void checkStatus_TI99()
{
  EEPROM_readAnything(7, ti99mapper);
  EEPROM_readAnything(8, gromsize);
  EEPROM_readAnything(9, cromsize);
  EEPROM_readAnything(13, grommap);
  if (ti99mapper > (ti99mapcount - 1)) {
    ti99mapper = 0;
    EEPROM_writeAnything(7, ti99mapper);
  }
  if (gromsize > gromhi) {
    gromsize = 1; // default 6K
    EEPROM_writeAnything(8, gromsize);
  }
  if (cromsize > cromhi) {
    cromsize = 2; // default 8K
    EEPROM_writeAnything(9, cromsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("TI-99 READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:     "));
//  println_Msg(ti99mapper);
  printMapper_TI99(ti99mapper);
  print_Msg(F("GROM SIZE:  "));
  print_Msg(GROM[gromsize]);
  println_Msg(F("KB"));
  print_Msg(F("ROM SIZE:   "));
  print_Msg(CROM[cromsize]);
  println_Msg(F("KB"));
  print_Msg(F("GROM MAP:   "));
  display_Update();
  readGROMMap();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:     "));
  Serial.println(ti99mapper);
  Serial.print(F("GROM SIZE: "));
  Serial.print(GROM[gromsize]);
  Serial.println(F("KB"));
  Serial.print(F("ROM SIZE:  "));
  Serial.print(CROM[cromsize]);
  Serial.println(F("KB"));
  Serial.print(F("GROM MAP:  "));
  readGROMMap();
  Serial.println(F(""));
#endif
}

void printMapper_TI99(byte ti99maplabel)
{
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  switch (ti99maplabel)
  {
    case 0:
      println_Msg(F("NORMAL"));
      break;
    case 1:
      println_Msg(F("MBX"));
      break;
    case 2:
      println_Msg(F("TI-CALC"));
      break;
  }
#else
  Serial.println(F("0 = NORMAL"));
  Serial.println(F("1 = MBX"));
  Serial.println(F("2 = TI-CALC"));
#endif
}

//******************************************
// MAPPER CODE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printMapperSelection_TI99(int index)
{
  display_Clear();
  print_Msg(FS(FSTRING_MAPPER));
  ti99index = index * 5;
  ti99mapselect = pgm_read_byte(ti99mapsize + ti99index);
  println_Msg(ti99mapselect);
}
#endif

void setMapper_TI99()
{
  byte newti99mapper;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  navigateMenu(0, ti99mapcount - 1, &printMapperSelection_TI99);
  newti99mapper = ti99mapselect;
  
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(newti99mapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  ti99mapfound = false;
  printMapper_TI99(0);
  Serial.print(F("Enter Mapper [0-2]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newti99mapper = newmap.toInt();
  for (int i = 0; i < ti99mapcount; i++) {
    ti99index = i * 5;
    ti99mapselect = pgm_read_byte(ti99mapsize + ti99index);
    if (newti99mapper == ti99mapselect)
      ti99mapfound = true;
  }
  if (ti99mapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(F(""));
    newti99mapper = 0;
    goto setmapper;
  }
#endif
  EEPROM_writeAnything(7, newti99mapper);
  ti99mapper = newti99mapper;
}

void checkMapperSize_TI99()
{
  for (int i = 0; i < ti99mapcount; i++) {
    ti99index = i * 5;
    byte mapcheck = pgm_read_byte(ti99mapsize + ti99index);
    if (mapcheck == ti99mapper) {
      gromlo = pgm_read_byte(ti99mapsize + ti99index + 1);
      gromhi = pgm_read_byte(ti99mapsize + ti99index + 2);
      cromlo = pgm_read_byte(ti99mapsize + ti99index + 3);
      cromhi = pgm_read_byte(ti99mapsize + ti99index + 4);
      break;
    }
  }
}

//******************************************
// GROM SIZE
//******************************************

void setGROMSize_TI99()
{
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (gromlo == gromhi)
    newgromsize = gromlo;
  else {
    int b = 0;
    int i = gromlo;
    while (1) {
      display_Clear();
      print_Msg(F("GROM Size: "));
      println_Msg(GROM[i]);
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("Press to Change"));
      println_Msg(F("Hold to Select"));
      display_Update();
      b = checkButton();
      if (b == 2) { // Previous
        if (i == gromlo)
          i = gromhi;
        else
          i--;
      }
      if (b == 1) { // Next
        if (i == gromhi)
          i = gromlo;
        else
          i++;
      }
      if (b == 3) { // Long Press - Execute
        newgromsize = i;
        break;
      }
    }
    display.setCursor(0, 48); // Display selection at bottom
  }
  print_Msg(F("GROM SIZE "));
  print_Msg(GROM[newgromsize]);
  println_Msg(F("KB"));
  display_Update();
  EEPROM_writeAnything(8, newgromsize);
  gromsize = newgromsize;
  // Default GROM Map to sequential GROMs
  newgrommap = 0;
  for (int x = 0; x < gromsize; x++){
    newgrommap = (newgrommap | (1 << (x + 3)));
  }
  EEPROM_writeAnything(13, newgrommap);
  grommap = newgrommap;
  print_Msg(F("GROM MAP  "));
  display_Update();
  readGROMMap();
  wait();
#else
  if (gromlo == gromhi)
    newgromsize = gromlo;
  else {
setgrom:
    String sizeGROM;
    for (int i = 0; i < (gromhi - gromlo + 1); i++) {
      Serial.print(F("Select GROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(GROM[i + gromlo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter GROM Size: "));
    while (Serial.available() == 0) {}
    sizeGROM = Serial.readStringUntil('\n');
    Serial.println(sizeGROM);
    newgromsize = sizeGROM.toInt() + gromlo;
    if (newgromsize > gromhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setgrom;
    }
  }
  Serial.print(F("GROM SIZE "));
  Serial.print(GROM[newgromsize]);
  Serial.println(F("KB"));
  EEPROM_writeAnything(8, newgromsize);
  gromsize = newgromsize;
  // Default GROM Map to sequential GROMs
  newgrommap = 0;
  for (int x = 0; x < gromsize; x++){
    newgrommap = (newgrommap | (1 << (x + 3)));
  }
  EEPROM_writeAnything(13, newgrommap);
  grommap = newgrommap;
  Serial.print(F("GROM MAP  "));
  readGROMMap();
#endif
}

void readGROMMap()
{
  newgromsize = 0;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  if (grommap == 0)
    print_Msg(FS(FSTRING_EMPTY));
  else {
    for (int x = 3; x < 8; x++) {
      if (((grommap >> x) & 0x1) == 1) {
        print_Msg(x);
        newgromsize++;
      }
    }
  }
  println_Msg(FS(FSTRING_EMPTY));
  display_Update();
#else
  if (grommap == 0)
    Serial.print(0);
  else (
    for (int x = 3; x < 8; x++) {
      if (((grommap >> x) & 0x1) == 1) {
        Serial.print(x);
        newgromsize++;
      }
    }
  }
  Serial.println(FS(FSTRING_EMPTY));
#endif
  if (gromsize != newgromsize) {
    EEPROM_writeAnything(8, newgromsize);
    gromsize = newgromsize;
  }
}

void setGROMMap_TI99()
{
  newgrommap = 0;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  int b = 0;
  for(int i = 3; i < 8; i++) {
    display_Clear();
    print_Msg(F("Enable GROM "));
    println_Msg(i);
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("Press to Skip GROM"));
    println_Msg(F("Hold to Enable GROM"));
    display_Update();
    while (1) {
      b = checkButton();
      if (b == 2) { // Previous
        break;
      }
      if (b == 1) { // Next
        break;
      }
      if (b == 3) { // Long Press - Execute
        newgrommap = (newgrommap | (1 << i));
        println_Msg(FS(FSTRING_EMPTY));
        print_Msg(F("GROM "));
        print_Msg(i);
        println_Msg(F(" ENABLED"));
        display_Update();
        break;
      }
    }
    b = 0;
    delay(1000);
  }
  EEPROM_writeAnything(13, newgrommap);
  grommap = newgrommap;
  display.setCursor(0, 56); // Display selection at bottom
  print_Msg(F("GROM MAP: "));
  display_Update();
  readGROMMap();
  delay(1000);
#else
  String mapGROM;
  for (int i = 3; i < 8; i++) {
    Serial.print(F("Enable GROM "));
    Serial.println(i);
    Serial.println(F("0 = NO"));
    Serial.println(F("1 = YES"));
    while (Serial.available() == 0) {}
    mapCROM = Serial.readStringUntil('\n');
    Serial.println(mapGROM);
    if (mapGROM.toInt() == 1)
      newgrommap = (newgrommap | (1 << i));
  }
  EEPROM_writeAnything(13, newgrommap);
  grommap = newgrommap;
  Serial.print(F("GROM MAP: "));
  readGROMMap();
#endif
}

//******************************************
// ROM SIZE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_TI99(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(CROM[index]);
}
#endif

void setCROMSize_TI99()
{
  byte newcromsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (cromlo == cromhi)
    newcromsize = cromlo;
  else {
    newcromsize = navigateMenu(cromlo, cromhi, &printRomSize_TI99);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(CROM[newcromsize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (cromlo == cromhi)
    newcromsize = cromlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (cromhi - cromlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(CROM[i + cromlo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newcromsize = sizeROM.toInt() + cromlo;
    if (newcromsize > cromhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(CROM[newcromsize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newcromsize);
  cromsize = newcromsize;
}

//******************************************
// CART SELECT CODE
//******************************************
struct database_entry_TI99 {
  byte gameMapper;
  byte gromSize;
  byte gromMap;
  byte romSize;
};

void readDataLine_TI99(FsFile& database, void* entry)
{
  struct database_entry_TI99* castEntry = (database_entry_TI99*)entry;
  
  // Read Maooer
  castEntry->gameMapper = database.read() - 48;

  // Skip over semicolon
  database.seekCur(1);

  // Read grom size
  castEntry->gromSize = database.read() - 48;

  // Skip over semicolon
  database.seekCur(1);

  // Read grom map
  // Read the next ascii character and subtract 48 to convert to decimal
  castEntry->gromMap = ((database.read() - 48) * 100) + ((database.read() - 48) * 10) + (database.read() - 48);

  // Skip over semicolon
  database.seekCur(1);

  // Read grom size
  castEntry->romSize = database.read() - 48;

  // Skip rest of line
  database.seekCur(2);
}

void printDataLine_TI99(void* entry)
{
  struct database_entry_TI99* castEntry = (database_entry_TI99*)entry;
  print_Msg(F("CODE: M"));
  print_Msg(castEntry->gameMapper);
  print_Msg(F("/G"));
  print_Msg(castEntry->gromSize);
  print_Msg(F("/C"));
  print_Msg(castEntry->romSize);
  print_Msg(F("/X"));
  println_Msg(castEntry->gromMap);
}

void setCart_TI99()
{
  sd.chdir();

  struct database_entry_TI99 entry;

    // Select starting letter
  byte myLetter = starting_letter();

  if (myFile.open("ti99cart.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLine_TI99, &entry, &printDataLine_TI99)) {
      EEPROM_writeAnything(7, entry.gameMapper);
      EEPROM_writeAnything(8, entry.gromSize);
      EEPROM_writeAnything(9, entry.romSize);
      EEPROM_writeAnything(13, entry.gromMap);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
