//******************************************
// INTELLIVISION MODULE
//******************************************
#ifdef enable_INTV

// Mattel Intellivision
// Cartridge Pinout
// 44P 2.54mm pitch connector
//
//    TOP SIDE             BOTTOM SIDE
//      (EVEN)             (ODD)
//              +-------+
//         GND -| 2   1 |- GND
//       CBLNK -| 4   3 |- /MSYNC
//     EXT AUD -| 6   5 |- DB7
//     EXT VID -| 8   7 |- DB8
//        MCLK -| 10  9 |- DB6
//       RESET -| 12 11 |- DB9
//         SR1 -| 14 13 |- DB5
//       VOICE -| 16 15 |- DB10
//       VOICE -| 18 17 |- DB4
//         GND -| 20 19 |- DB11
//         GND -| 22 21 |- DB3
//         GND -| 24 23 |- DB12
//         GND -| 26 25 |- DB13
//         GND -| 28 27 |- DB2
//      /BUSAK -| 30 29 |- DB14
//         BC1 -| 32 31 |- DB1
//         BC2 -| 34 33 |- DB0
//        BDIR -| 36 35 |- DB15
//        BDIR -| 38 37 |- BDIR
//         BC2 -| 40 39 |- BC2
//         BC1 -| 42 41 |- BC1
//         GND -| 44 43 |- VCC(+5V)
//              +-------+

// CONTROL PINS:
// /MSYNC(PH3)- PIN 3  [ROM PIN 14]
// BC1(PH4)   - PIN 41 [ROM PIN 28]
// BC2(PH5)   - PIN 39 [ROM PIN 27]
// BDIR(PH6)  - PIN 37 [ROM PIN 26]
// NOTE: BDIR/BC1/BC2 ONLY CONNECTED TO BOTTOM (ODD) PINS

// NOT CONNECTED:
// RESET(PH0) - N/C - GND IN CART
// MCLK(PH1)  - N/C - GND IN CART

// BUS PROTOCOL COMMANDS - BDIR/BC1/BC2
// NO ACTION (NACT):           0/0/0
// ADDR TO REG (ADAR):         0/1/0
// INT ADDR TO BUS (IAB):      0/0/1
// DATA TO BUS (DTB):          0/1/1
// BUS TO ADDR (BAR):          1/0/0
// DATA WRITE (DW):            1/1/0
// DATA WRITE STROBE (DWS):    1/0/1
// INTERRUPT ACK (INTAK):      1/1/1

// Cart Configurations
// Format = {mapper,romlo,romhi,ramsize}
static const byte PROGMEM intvmapsize[] = {
  0, 0, 2, 0,  // default mattel up to 32K (8K/16K/24K/32K)
  1, 1, 3, 0,  // demo cart 16K, championship tennis 32K, wsml baseball 48K
  2, 1, 3, 0,  // up to 48K (16K/32K/48K)
  3, 4, 4, 0,  // tower of doom 48K
  4, 0, 1, 1,  // uscf chess 16K + RAM 1K
  5, 2, 3, 0,  // congo bongo/defender/pac-man 24K, dig dug 32K
  6, 1, 1, 0,  // centipede 16K
  7, 1, 1, 0,  // imagic carts 16K
  8, 1, 1, 0,  // mte-201 test cart 16K
  9, 3, 3, 2,  // triple challenge 32K + RAM 2K
};

byte intvmapcount = 10;  // (sizeof(mapsize)/sizeof(mapsize[0])) / 4;
boolean intvmapfound = false;
byte intvmapselect;
int intvindex;

const byte INTV[] PROGMEM = { 8, 16, 24, 32, 48 };
byte intvlo = 0;  // Lowest Entry
byte intvhi = 4;  // Highest Entry

byte intvmapper;
byte newintvmapper;
byte intvsize;
byte newintvsize;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char intvMenuItem1[] PROGMEM = "Select Cart";
static const char intvMenuItem2[] PROGMEM = "Read ROM";
static const char intvMenuItem3[] PROGMEM = "Set Mapper + Size";
//static const char intvMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsINTV[] PROGMEM = { intvMenuItem1, intvMenuItem2, intvMenuItem3, string_reset2 };

void setup_INTV() {
  // Set Address Pins to Output (UNUSED)
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //      ---(PH0)   ---(PH1)  /MSYNC(PH3) BC1(PH4)   BC2(PH5)  BDIR(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (DB0-DB15) to Input
  DDRC = 0x00;
  DDRA = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)  /MSYNC(PH3) BC1(PH4)   BC2(PH5)  BDIR(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Pins HIGH
  PORTF = 0xFF;       // A0-A7
  PORTK = 0xFF;       // A8-A15
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_INTV();
  strcpy(romName, "INTV");

  mode = mode_INTV;
}

void intvMenu() {
  vselect(false);
  convertPgm(menuOptionsINTV, 4);
  uint8_t mainMenu = question_box(F("INTELLIVISION MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_INTV();
      setup_INTV();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_INTV();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper
      setMapper_INTV();
      checkMapperSize_INTV();
      setROMSize_INTV();
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// INTELLIVISION BUS PROTOCOL COMMANDS
//******************************************
// CONTROL PINS -         BDIR/BC1/BC2
// NO ACTION (NACT):         0/0/0
// ADDR TO REG (ADAR):       0/1/0 [NOTE: ORDER BDIR/BC1/BC2]
// DATA TO BUS (DTB):        0/1/1
// BUS TO ADDR (BAR):        1/0/0
// DATA WRITE (DW):          1/1/0 [NOTE: ORDER BDIR/BC1/BC2]
// DATA WRITE STROBE (DWS):  1/0/1 [NOTE: ORDER BDIR/BC1/BC2]

// IMMEDIATE MODE DATA READ SEQUENCE: BAR-NACT-DTB-NACT
// IMMEDIATE MODE DATA WRITE SEQUENCE: BAR-NACT-DW-DWS-NACT
// DIRECT ADDRESSING MODE READ SEQUENCE: BAR-NACT-ADAR-NACT-DTB-NACT

// NO ACTION (NACT) - 0/0/0
void NACT_INT() {
  // Switch BC1(PH4) + BC2(PH5) + BDIR(PH6) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 5) & ~(1 << 6);
  // DB0..DB15 INPUT
  DDRC = 0x00;
  DDRA = 0x00;
}

// SET ADDRESS - BUS TO ADDR (BAR) - 1/0/0
void BAR_INT() {
  // Switch BDIR(PH6) to HIGH
  PORTH |= (1 << 6);
  // Switch BC1(PH4) + BC2(PH5) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 5);
  // Address OUT
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// READ DATA - DATA TO BUS (DTB) - 0/1/1
void DTB_INT() {
  // Switch BDIR(PH6) to LOW
  PORTH &= ~(1 << 6);
  // Switch BC1(PH4) + BC2(PH5) to HIGH
  PORTH |= (1 << 4) | (1 << 5);
  // Data IN
  DDRC = 0x00;
  DDRA = 0x00;
}

// ADDRESS DATA TO ADDRESS REGISTER (ADAR) - 0/1/0
void ADAR_INT() {
  // Switch BC2(PH5) + BDIR(PH6) to LOW
  PORTH &= ~(1 << 5) & ~(1 << 6);
  // Switch BC1(PH4) to HIGH
  PORTH |= (1 << 4);
}

// DATA WRITE PAIRED COMMAND - DW + DWS
// DATA SHOULD BE STABLE ACROSS BOTH

// DATA WRITE (DW) - 1/1/0
void DW_INT() {
  // Switch BC1(PH4) + BDIR(PH6) to HIGH
  PORTH |= (1 << 4) | (1 << 6);
  // Switch BC2(PH5) to LOW
  PORTH &= ~(1 << 5);
}

// DATA WRITE STROBE (DWS) - 1/0/1
void DWS_INT() {
  // Switch BC2(PH5) + BDIR(PH6) to HIGH
  PORTH |= (1 << 5) | (1 << 6);
  // Switch BC1(PH4) to LOW
  PORTH &= ~(1 << 4);
}

//******************************************
// READ CODE
//******************************************

uint16_t readData_INTV(uint32_t addr) {
  PORTC = addr & 0xFF;
  PORTA = (addr >> 8) & 0xFF;

  BAR_INT();
  // Wait for bus
  // 5 x 62.5ns = 312.5ns
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  NACT_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DTB_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  uint16_t ret = (((PINA & 0xFF) << 8) | (PINC & 0xFF));

  NACT_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  return ret;
}

// MAPPER ROM ADDRESSES
// 0: 0x50-0x70,0xD0-0xE0,0xF0-0x100,           // default mattel up to 32K (8K/16K/24K/32K)
// 1: 0x50-0x70,0xD0-0x100,                     // demo cart 16K, championship tennis 32K, wsml baseball 48K
// 2: 0x50-0x70,0x90-0xC0,0xD0-0xE0,            // up to 48K (16K/32K/48K)
// 3: 0x50-0x70,0x90-0xB0,0xD0-0xE0,0xF0-0x100, // tower of doom 48K
// 4: 0x50-0x70,                                // uscf chess 16K + RAM 1K
// 5: 0x50-0x80,0x90-0xC0,                      // congo bongo/defender/pac-man 24K, dig dug 32K
// 6: 0x60-0x80,                                // centipede 16K
// 7: 0x48-0x68,                                // imagic carts 16K
// 8: 0x50-0x60,0x70-0x80,                      // mte-201 test cart 16K
// 9: 0x50-0x70,0x90-0xB0,[0xC0-0xC8,0xD0-0xD8] // triple challenge 32K + RAM 2K [0xC0 + 0xD0 segments are not needed]

void readSegment_INTV(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 256) {
    for (uint16_t w = 0; w < 256; w++) {
      uint16_t temp = readData_INTV(addr + w);
      sdBuffer[w * 2] = (temp >> 8) & 0xFF;
      sdBuffer[(w * 2) + 1] = temp & 0xFF;
    }
    myFile.write(sdBuffer, 512);
  }
}

// MODIFIED READ ROUTINE FOR ALL 10 MAPPERS
void readROM_INTV() {
  strcpy(fileName, romName);
  strcat(fileName, ".int");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "INTV/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(create_file_STR);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  switch (intvmapper) {
    case 0:                              //default mattel up to 32K (8K/16K/24K/32K)
      readSegment_INTV(0x5000, 0x6000);  // 8K
      if (intvsize > 0) {
        readSegment_INTV(0x6000, 0x7000);  // +8K = 16K
        if (intvsize > 1) {
          readSegment_INTV(0xD000, 0xE000);  // +8K = 24K
          if (intvsize > 2)
            readSegment_INTV(0xF000, 0x10000);  // +8K = 32K
        }
      }
      break;

    case 1:                              // demo cart/championship tennis/wsml baseball
      readSegment_INTV(0x5000, 0x7000);  // 16K Demo Cart
      if (intvsize > 1) {
        readSegment_INTV(0xD000, 0xE000);  // +8K = 24K [NONE]
        if (intvsize > 2) {
          readSegment_INTV(0xE000, 0xF000);  // +8K = 32K Championship Tennis
          if (intvsize > 3) {
            readSegment_INTV(0xF000, 0x10000);  // +8K = 40K WSML Baseball [MISSING 8K ECS BANK]
            // ecs bank switch
            ecsBank(0xFFFF, 0x1);               // switch ecs page 1 to 0xF000
            readSegment_INTV(0xF000, 0x10000);  // + 8K = 48K WSML Baseball
            ecsBank(0xFFFF, 0x0);               // reset ecs page 0 to 0xF000
          }
        }
      }
      break;

    case 2:                              // up to 48K (16K/32K/48K)
      readSegment_INTV(0x5000, 0x7000);  // 16K
      if (intvsize > 1) {
        readSegment_INTV(0x9000, 0xA000);  // +8K = 24K [NONE]
        if (intvsize > 2) {
          readSegment_INTV(0xA000, 0xB000);  // +8K = 32K
          if (intvsize > 3) {
            readSegment_INTV(0xB000, 0xC000);  // +8K = 40K
            readSegment_INTV(0xD000, 0xE000);  // +8K = 48K
          }
        }
      }
      break;

    case 3:                               // tower of doom 48K
      readSegment_INTV(0x5000, 0x7000);   // 16K
      readSegment_INTV(0x9000, 0xB000);   // +16K = 32K
      readSegment_INTV(0xD000, 0xE000);   // +8K = 40K
      readSegment_INTV(0xF000, 0x10000);  // +8K = 48K
      break;

    case 4:                              // chess 16K
      PORTH &= ~(1 << 3);                // /MSYNC to LOW
      readSegment_INTV(0x5000, 0x6000);  // 8K
      PORTH |= (1 << 3);                 // /MSYNC to HIGH
      readSegment_INTV(0x6000, 0x7000);  // 8K
      break;

    case 5:                              // congo bongo/defender/pac-man/dig dug
      readSegment_INTV(0x5000, 0x7000);  // 16K
      readSegment_INTV(0x7000, 0x8000);  // +8K = 24K Congo Bongo/Defender/Pac-Man
      if (intvsize > 2) {
        readSegment_INTV(0x9000, 0xA000);  // +8K = 32K Dig Dug
        //readSegment_INTV(0xA000,0xC000); // +16K = 48K [UNUSED]
      }
      break;

    case 6:                              // centipede 16K
      readSegment_INTV(0x6000, 0x8000);  // 16K
      break;

    case 7:                              // imagic carts 16K
      readSegment_INTV(0x4800, 0x6800);  // 16K
      break;

    case 8:                              //mte-201 test cart 16K
      readSegment_INTV(0x5000, 0x6000);  // 8K
      readSegment_INTV(0x7000, 0x8000);  // +8K = 16K
      break;

    case 9:                              // triple challenge 32K [KNOWN ROM 44K BAD!]
      readSegment_INTV(0x5000, 0x7000);  // 16K
      readSegment_INTV(0x9000, 0xB000);  // +16K = 32K
      // 0xC000 + 0xD000 SEGMENTS ARE NOT NEEDED (PER INTVNUT POST)
      //      readSegment_INTV(0xC000,0xC800); // +4K = 36K
      //      readSegment_INTV(0xD000,0xE000); // +8K = 44K
      break;
  }
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  compareCRC("intv.txt", 0, 1, 0);

  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

// ECS BANKSWITCH FOR WSML BASEBALL
// write $xA5y to $xFFF
// x = rom location ($x000 - $xFFF)
// y = page (up to 16 - WSML Baseball only uses 0/1)
void ecsBank(uint32_t addr, uint8_t bank) {
  uint16_t ecsdata = (addr & 0xF000) + 0x0A50 + bank;  // $xA5y

  // Data OUT
  DDRA = 0xFF;
  DDRC = 0xFF;

  // Set Address
  PORTA = (addr >> 8) & 0xFF;
  PORTC = addr & 0xFF;

  BAR_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NACT_INT();
  NOP;

  // Data OUT
  DDRA = 0xFF;
  DDRC = 0xFF;
  PORTA = (ecsdata >> 8) & 0xFF;  // $xA
  PORTC = ecsdata & 0xFF;         // $5y

  DW_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  DWS_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NACT_INT();
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
}

//******************************************
// MAPPER CODE
//******************************************

void setMapper_INTV() {
#if (defined(enable_OLED) || defined(enable_LCD))
  int b = 0;
  int i = 0;
  // Check Button Status
#if defined(enable_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
  boolean buttonVal1 = (PING & (1 << 2));      // PG2
#endif
  if (buttonVal1 == LOW) {  // Button Pressed
    while (1) {             // Scroll Mapper List
#if defined(enable_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
      boolean buttonVal1 = (PING & (1 << 2));  // PG2
#endif
      if (buttonVal1 == HIGH) {  // Button Released
        // Correct Overshoot
        if (i == 0)
          i = intvmapcount - 1;
        else
          i--;
        break;
      }
      display_Clear();
      print_Msg(F("Mapper: "));
      intvindex = i * 4;
      intvmapselect = pgm_read_byte(intvmapsize + intvindex);
      println_Msg(intvmapselect);
      display_Update();
      if (i == (intvmapcount - 1))
        i = 0;
      else
        i++;
      delay(250);
    }
  }

  display_Clear();
  print_Msg(F("Mapper: "));
  intvindex = i * 4;
  intvmapselect = pgm_read_byte(intvmapsize + intvindex);
  println_Msg(intvmapselect);
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
        i = intvmapcount - 1;
      else
        i--;

      // Only update display after input because of slow LCD library
      display_Clear();
      print_Msg(F("Mapper: "));
      intvindex = i * 4;
      intvmapselect = pgm_read_byte(intvmapsize + intvindex);
      println_Msg(intvmapselect);
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
      if (i == (intvmapcount - 1))
        i = 0;
      else
        i++;

      // Only update display after input because of slow LCD library
      display_Clear();
      print_Msg(F("Mapper: "));
      intvindex = i * 4;
      intvmapselect = pgm_read_byte(intvmapsize + intvindex);
      println_Msg(intvmapselect);
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
      newintvmapper = intvmapselect;
      break;
    }
  }
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(newintvmapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  intvmapfound = false;
  Serial.print(F("Enter Mapper [0-9]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newintvmapper = newmap.toInt();
  for (int i = 0; i < intvmapcount; i++) {
    intvindex = i * 4;
    intvmapselect = pgm_read_byte(intvmapsize + intvindex);
    if (newintvmapper == intvmapselect)
      intvmapfound = true;
  }
  if (intvmapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(F(""));
    newintvmapper = 0;
    goto setmapper;
  }
#endif
  EEPROM_writeAnything(7, newintvmapper);
  intvmapper = newintvmapper;
}

void checkMapperSize_INTV() {
  for (int i = 0; i < intvmapcount; i++) {
    intvindex = i * 4;
    byte mapcheck = pgm_read_byte(intvmapsize + intvindex);
    if (mapcheck == intvmapper) {
      intvlo = pgm_read_byte(intvmapsize + intvindex + 1);
      intvhi = pgm_read_byte(intvmapsize + intvindex + 2);
      break;
    }
  }
}

void setROMSize_INTV() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (intvlo == intvhi)
    newintvsize = intvlo;
  else {
    int b = 0;
    int i = intvlo;

    // Only update display after input because of slow LCD library
    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(pgm_read_byte(&(INTV[i])));
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
      if (b == 2) {  // Previous (doubleclick)
        if (i == intvlo)
          i = intvhi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(pgm_read_byte(&(INTV[i])));
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
      if (b == 1) {  // Next (press)
        if (i == intvhi)
          i = intvlo;
        else
          i++;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(pgm_read_byte(&(INTV[i])));
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
        newintvsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(pgm_read_byte(&(INTV[newintvsize])));
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (intvlo == intvhi)
    newintvsize = intvlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (intvhi - intvlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(pgm_read_byte(&(INTV[i + intvlo])));
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newintvsize = sizeROM.toInt() + intvlo;
    if (newintvsize > intvhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(pgm_read_byte(&(INTV[newintvsize])));
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newintvsize);
  intvsize = newintvsize;
}

void checkStatus_INTV() {
  EEPROM_readAnything(7, intvmapper);
  EEPROM_readAnything(8, intvsize);
  if (intvmapper > 9) {
    intvmapper = 0;
    EEPROM_writeAnything(7, intvmapper);
  }
  if (intvsize > 4) {
    intvsize = 0;
    EEPROM_writeAnything(8, intvsize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("INTELLIVISION READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("MAPPER:   "));
  println_Msg(intvmapper);
  print_Msg(F("ROM SIZE: "));
  print_Msg(pgm_read_byte(&(INTV[intvsize])));
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:   "));
  Serial.println(intvmapper);
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(pgm_read_byte(&(INTV[intvsize])));
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_INTV() {
  char gamename[100];
  char tempStr2[2];
  char crc_search[9];

  //go to root
  sd.chdir();

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("intv.txt", O_READ)) {
    // Skip ahead to selected starting letter
    if ((myLetter > 0) && (myLetter <= 26)) {
      while (myFile.available()) {
        // Read current name
        get_line(gamename, &myFile, 96);

        // Compare selected letter with first letter of current name until match
        while (gamename[0] != 64 + myLetter) {
          skip_line(&myFile);
          skip_line(&myFile);
          get_line(gamename, &myFile, 96);
        }
        break;
      }

      // Rewind one line
      rewind_line(myFile);
    }

    // Display database
    while (myFile.available()) {
      display_Clear();

      // Read game name
      get_line(gamename, &myFile, 96);

      // Read CRC32 checksum
      sprintf(checksumStr, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(checksumStr, tempStr2);
      }

      // Skip over semicolon
      myFile.seekCur(1);

      // Read CRC32 of first 512 bytes
      sprintf(crc_search, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(crc_search, tempStr2);
      }

      // Skip over semicolon
      myFile.seekCur(1);

      // Read mapper
      intvmapper = myFile.read() - 48;

      // Skip over semicolon
      myFile.seekCur(1);

      // Read rom size
      // Read the next ascii character and subtract 48 to convert to decimal
      cartSize = myFile.read() - 48;

      // Remove leading 0 for single digit cart sizes
      if (cartSize != 0) {
        cartSize = cartSize * 10 + myFile.read() - 48;
      } else {
        cartSize = myFile.read() - 48;
      }

      // Skip over semicolon
      myFile.seekCur(1);

      // Read SRAM size
      byte sramSize __attribute__((unused)) = myFile.read() - 48;

      // Skip rest of line
      myFile.seekCur(2);

      // Skip every 3rd line
      skip_line(&myFile);

      println_Msg(F("Select your cartridge"));
      println_Msg(F(""));
      println_Msg(gamename);
      print_Msg(F("Size: "));
      print_Msg(cartSize);
      println_Msg(F("KB"));
      print_Msg(F("Mapper: "));
      println_Msg(intvmapper);
#if defined(enable_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#elif defined(SERIAL_MONITOR)
      println_Msg(F("U/D to Change"));
      println_Msg(F("Space to Select"));
#endif
      display_Update();

      int b = 0;
      while (1) {
        // Check button input
        b = checkButton();

        // Next
        if (b == 1) {
          break;
        }

        // Previous
        else if (b == 2) {
          rewind_line(myFile, 6);
          break;
        }

        // Selection
        else if (b == 3) {
          //byte INTV[] = {8, 16, 24, 32, 48};
          switch (cartSize) {
            case 8:
              intvsize = 0;
              break;

            case 16:
              intvsize = 1;
              break;

            case 24:
              intvsize = 2;
              break;

            case 32:
              intvsize = 3;
              break;

            case 48:
              intvsize = 4;
              break;

            default:
              intvsize = 0;
              break;
          }
          EEPROM_writeAnything(7, intvmapper);
          EEPROM_writeAnything(8, intvsize);
          myFile.close();
          break;
        }
      }
    }
  } else {
    print_FatalError(F("Database file not found"));
  }
}
#endif
//******************************************
// End of File
//******************************************