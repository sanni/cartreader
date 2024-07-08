//******************************************
// COMMODORE VIC-20 MODULE
//******************************************
#ifdef ENABLE_VIC20
// Commodore VIC-20
// Cartridge Pinout
// 44P 3.96mm pitch connector
//
//   FRONT             BACK
//    SIDE             SIDE
//          +-------+
//     GND -|  1  A |- GND
//      D0 -|  2  B |- A0
//      D1 -|  3  C |- A1
//      D2 -|  4  D |- A2
//      D3 -|  5  E |- A3
//      D4 -|  6  F |- A4
//      D5 -|  7  H |- A5
//      D6 -|  8  J |- A6
//      D7 -|  9  K |- A7
//   /BLK1 -| 10  L |- A8
//   /BLK2 -| 11  M |- A9
//   /BLK3 -| 12  N |- A10
//   /BLK5 -| 13  P |- A11
//   /RAM1 -| 14  R |- A12
//   /RAM2 -| 15  S |- A13
//   /RAM3 -| 16  T |- IO2
//    VR/W -| 17  U |- IO3
//    CR/W -| 18  V |- PHI2
//     IRQ -| 19  W |- /NMI
//      NC -| 20  X |- /RESET
//     +5V -| 21  Y |- NC
//     GND -| 22  Z |- GND
//          +-------+
//
//                                              BACK
//       GND NC /RST /NMI PH2 I3  I2 A13 A12 A11 A10  A9  A8 A7 A6 A5 A4 A3 A2 A1 A0 GND    
//      +-------------------------------------------------------------------------------+
//      |  Z   Y   X   W   V   U   T   S   R   P   N   M   L  K  J  H  F  E  D  C  B  A |
// LEFT |                                                                               | RIGHT
//      | 22  21  20  19  18  17  16  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1 |
//      +-------------------------------------------------------------------------------+
//       GND +5V  NC IRQ CRW VRW /R3 /R2 /R1 /B5 /B3 /B2 /B1 D7 D6 D5 D4 D3 D2 D1 D0 GND  
//                                              FRONT

// CONTROL PINS:
// /BLK1(PH3) - SNES /CS  - [$2000-$3FFF]
// /BLK2(PH4) - SNES /IRQ - [$4000-$5FFF]
// /BLK3(PH5) - SNES /WR  - [$6000-$7FFF]
// /BLK5(PH6) - SNES /RD  - [$A000-$BFFF]

//******************************************
// VARIABLES
//******************************************
byte VIC20MAP[] = {
0x20, // 0x2000
0x24, // 0x2000/0x4000
0x2A, // 0x2000/0xA000
0x46, // 0x4000/0x6000 - Adventure Games
0x60, // 0x6000
0x6A, // 0x6000/0xA000 - Standard 16K 
0x70, // 0x7000
0xA0, // 0xA000        - Standard 8K
0xB0  // 0xB000
};

byte vic20mapcount = 9;
byte vic20mapselect;
byte vic20maplo = 0; // Lowest Entry
byte vic20maphi = 8; // Highest Entry
byte vic20map = 0;
byte newvic20map;

byte VIC20SIZE[] = {
0x20, // 2K/0K 0x800 
0x40, // 4K/0K 0x1000
0x80, // 8K/0K 0x2000
0x44, // 4K/4K 0x1000/0x1000
0x48, // 4K/8K 0x1000/0x2000
0x84, // 8K/4K 0x2000/0x1000
0x88  // 8K/8K 0x2000/0x2000	
};

byte vic20lo = 0; // Lowest Entry
byte vic20hi = 6; // Highest Entry
byte vic20size;
byte newvic20size;

//byte VIC20[] = {2,4,8};
//byte vic20lo = 0; // Lowest Entry
//byte vic20hi = 2; // Highest Entry
//byte vic20size;
//byte newvic20size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsVIC20[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void vic20Menu()
{
  convertPgm(menuOptionsVIC20, 4);
  uint8_t mainMenu = question_box(F("VIC-20 MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_VIC20();
      setup_VIC20();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      CreateROMFolder_VIC20();
      readROM_VIC20();
      FinishROMFolder_VIC20();
      sd.chdir("/");
      break;

    case 2:
      // Set ROM Map + Size
      setROMMap_VIC20();
      setROMSize_VIC20();
      break;
    
    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_VIC20()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // VIC-20 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //      /RST(PH0)   ---(PH1)  /BLK1(PH3) /BLK2(PH4) /BLK3(PH5) /BLK5(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //      /RST(PH0)   ---(PH1)  /BLK1(PH3) /BLK2(PH4) /BLK3(PH5) /BLK5(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_VIC20();
  strcpy(romName, "VIC20");

  mode = CORE_VIC20;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_VIC20(uint16_t addr)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  return ret;
}

void readSegment_VIC20(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_VIC20(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// ROM FOLDER
//******************************************

void CreateROMFolder_VIC20()
{
  sd.chdir();
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "VIC20/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);
}

void FinishROMFolder_VIC20()
{
  foldern += 1;
  EEPROM_writeAnything(0, foldern); // FOLDER #
  sd.chdir();
}

//******************************************
// READ ROM
//******************************************

void readROM_VIC20() 
{
  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  byte rommap = 0;
  byte romsize = 0;

  // Split into Individual ROM Files
  for (int x = 0; x < 2; x++)  { // ROM0/ROM1
    if (x == 1) {
      if ((VIC20MAP[vic20map] & 0x0F) == 0)
        break;
      rommap = ((VIC20MAP[vic20map] & 0x0F) << 4);
      romsize = VIC20SIZE[vic20size] & 0x0F;
    }
    else {
      rommap = VIC20MAP[vic20map] & 0xF0;
      romsize = ((VIC20SIZE[vic20size] & 0xF0) >> 4);
    }
    snprintf(fileName, sizeof(fileName), "%s.%x", romName, rommap);
    // open file on sdcard
    if (!myFile.open(fileName, O_RDWR | O_CREAT))
      print_FatalError(F("Can't create file on SD"));

    if (rommap == 0x20) { // BLK1
      PORTH &= ~(1 << 3); // BLK1(PH3) LOW
      readSegment_VIC20(0x2000,0x3000); // 4K
      if (romsize == 8)
        readSegment_VIC20(0x3000,0x4000); // +4K = 8K
      PORTH |= (1 << 3); // BLK1(PH3) HIGH
    }
    else if (rommap == 0x40) { // BLK2
      PORTH &= ~(1 << 4); // BLK2(PH4) LOW
      readSegment_VIC20(0x4000,0x5000); // 4K
      if (romsize == 8)
        readSegment_VIC20(0x5000,0x6000); // +4K = 8K
      PORTH |= (1 << 4); // BLK2(PH4) HIGH
    }
    else if (rommap == 0x60) { // BLK3
      PORTH &= ~(1 << 5); // BLK3(PH5) LOW
      readSegment_VIC20(0x6000,0x7000); // 4K
        if (romsize == 8)
          readSegment_VIC20(0x7000,0x8000); // +4K = 8K
      PORTH |= (1 << 5); // BLK3(PH5) HIGH
    }
    else if (rommap == 0x70) { // BLK3 UPPER HALF
      PORTH &= ~(1 << 5); // BLK3(PH5) LOW
      readSegment_VIC20(0x7000,0x8000);
      PORTH |= (1 << 5); // BLK3(PH5) HIGH
    }
    else if (rommap == 0xA0) { // BLK5
      PORTH &= ~(1 << 6); // BLK5(PH6) LOW
      readSegment_VIC20(0xA000,0xA800); // 2K
      if (romsize > 2) {
        readSegment_VIC20(0xA800,0xB000); // +2K = 4K
        if (romsize > 4)
          readSegment_VIC20(0xB000,0xC000); // +4K = 8K
      }
      PORTH |= (1 << 6); // BLK5(PH6) HIGH
    }
    else if (rommap == 0xB0) { // BLK5 UPPER HALF
      PORTH &= ~(1 << 6); // BLK5(PH6) LOW
      readSegment_VIC20(0xB000,0xB800); // 2K
      if (romsize > 2)
        readSegment_VIC20(0xB800,0xC000); // +2K = 4K
      PORTH |= (1 << 6); // BLK5(PH6) HIGH
    }
    myFile.close();

    print_Msg(F("ROM"));
    print_Msg(x);
    print_Msg(FS(FSTRING_SPACE));
    printCRC(fileName, NULL, 0);
  }
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait(); 
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_VIC20()
{
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (vic20lo == vic20hi)
    newvic20size = vic20lo;
  else {
    int b = 0;
    int i = vic20lo;
    while (1) {
      display_Clear();
      print_Msg(F("ROM0 Size: "));
      println_Msg((VIC20SIZE[i] & 0xF0) >> 4);
      print_Msg(F("ROM1 Size: "));
      println_Msg(VIC20SIZE[i] & 0x0F);
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("Press to Change"));
      println_Msg(F("Hold to Select"));
      display_Update();
      b = checkButton();
      if (b == 2) { // Previous (doubleclick)
        if (i == vic20lo)
          i = vic20hi;
        else
          i--;
      }
      if (b == 1) { // Next (press)
        if (i == vic20hi)
          i = vic20lo;
        else
          i++;
      }
      if (b == 3) { // Long Press - Execute (hold)
        newvic20size = i;
        break;
      }
    }
    display.setCursor(0, 48); // Display selection at bottom
  }
  print_Msg(F("ROM0 SIZE "));
  print_Msg((VIC20SIZE[newvic20size] & 0xF0) >> 4);
  println_Msg(F("KB"));
  print_Msg(F("ROM1 SIZE "));
  print_Msg(VIC20SIZE[newvic20size] & 0x0F);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (vic20lo == vic20hi)
    newvic20size = vic20lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (vic20hi - vic20lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print((VIC20SIZE[i + vic20lo] & 0xF0) >> 4);
      Serial.print(F("KB/"));
      Serial.print(VIC20SIZE[i + vic20lo] & 0x0F);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newvic20size = sizeROM.toInt() + vic20lo;
    if (newvic20size > vic20hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM0 Size = "));
  Serial.print((VIC20SIZE[newvic20size] & 0xF0) >> 4);
  Serial.println(F("KB"));
  Serial.print(F("ROM1 Size = "));
  Serial.print(VIC20SIZE[newvic20size] & 0x0F);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newvic20size);
  vic20size = newvic20size;
}

void checkStatus_VIC20()
{
  EEPROM_readAnything(7, vic20map);
  EEPROM_readAnything(8, vic20size);
  if (vic20map > 8) {
    vic20map = 7; // default 0xA000
    EEPROM_writeAnything(7, vic20map);
  }
  if (vic20size > vic20hi) {
    vic20size = 2; // default 8K
    EEPROM_writeAnything(8, vic20size);
  }
 
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("COMMODORE VIC-20"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("ROM MAP:   "));
  println_Msg(VIC20MAP[vic20map], HEX);
  print_Msg(F("ROM0 SIZE: "));
  print_Msg((VIC20SIZE[vic20size] & 0xF0) >> 4);
  println_Msg(F("KB"));
  print_Msg(F("ROM1 SIZE: "));
  print_Msg(VIC20SIZE[vic20size] & 0x0F);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM MAP:   "));
  Serial.println(VIC20MAP[vic20map]);
  Serial.print(F("CURRENT ROM0 SIZE: "));
  Serial.print((VIC20SIZE[vic20size] & 0xF0) >> 4);
  Serial.println(F("KB"));
  Serial.print(F("CURRENT ROM1 SIZE: "));
  Serial.print(VIC20SIZE[vic20size] & 0x0F);
  Serial.println(F("KB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// SET ROM MAP
//******************************************

void setROMMap_VIC20()
{
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  int b = 0;
  int i = 0;
  // Check Button Status
#     if defined(ENABLE_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#     elif defined(ENABLE_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#     endif /* ENABLE_OLED | ENABLE_LCD */

  if (buttonVal1 == LOW) {  // Button Pressed
    while (1) {             // Scroll Mapper List
#     if defined(ENABLE_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#     elif defined(ENABLE_LCD)
      buttonVal1 = (PING & (1 << 2));  // PG2
#     endif /* ENABLE_OLED | ENABLE_LCD */
      if (buttonVal1 == HIGH) { // Button Released
        // Correct Overshoot
        if (i == 0)
          i = vic20mapcount - 1;
        else 
          i--;
        break;
      }
      display_Clear();
      print_Msg(F("ROM Map: "));
      vic20mapselect = VIC20MAP[i];
      println_Msg(vic20mapselect, HEX);
      if (i == (vic20mapcount - 1))
        i = 0;
      else
        i++;
      delay(250);
    }
  }
  while (1) {
    display_Clear();
    print_Msg(F("ROM Map: "));
    vic20mapselect = VIC20MAP[i];
    println_Msg(vic20mapselect, HEX);
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("Press to Change"));
    println_Msg(F("Hold to Select"));
    display_Update();
    b = checkButton();
    if (b == 2) { // Previous ROM Map (doubleclick)
      if (i == 0)
        i = vic20mapcount - 1;
      else
        i--;
    }
    if (b == 1) { // Next ROM Map (press)
      if (i == (vic20mapcount - 1))
        i = 0;
      else
        i++;
    }
    if (b == 3) { // Long Press - Execute (hold)
      newvic20map = i;
      break;
    }
  }
  display.setCursor(0, 56);
  print_Msg(F("ROM MAP "));
  print_Msg(vic20mapselect, HEX);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
  String newmap;
  Serial.println(F("ROM MAP:"));
  Serial.println(F("0 = 0x2000"));
  Serial.println(F("1 = 0x2000/0x4000"));
  Serial.println(F("2 = 0x2000/0xA000"));
  Serial.println(F("3 = 0x4000/0x6000"));
  Serial.println(F("4 = 0x6000"));
  Serial.println(F("5 = 0x6000/0xA000"));
  Serial.println(F("6 = 0x7000"));
  Serial.println(F("7 = 0xA000"));
  Serial.println(F("8 = 0xB000"));
  Serial.print(F("Enter Mapper [0-8]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newvic20map = newmap.toInt();
#endif
  EEPROM_writeAnything(7, newvic20map);
  vic20map = newvic20map;
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_VIC20()
{
  //go to root
  sd.chdir();

  struct database_entry_mapper_size entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("vic20cart.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineMapperSize, &entry)) {
      EEPROM_writeAnything(7, entry.gameMapper);
      EEPROM_writeAnything(8, entry.gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif