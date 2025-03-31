//******************************************
// VSMILE MODULE
//******************************************
#ifdef ENABLE_VSMILE
// V.Smile
// Cartridge pinout
// 48P 2.54mm pitch connector
//
// FRONT               BACK
//  SIDE               SIDE
//        +---------+
//   VCC -| 25    1 |- WE
// RAMCS -| 26    2 |- OE
//   VCC -| 27    3 |- GND
//  CSB2 -| 28    4 |- D3
//    D2 -| 29    5 |- D4
//    D1 -| 30    6 |- D5
//    D0 -| 31    7 |- D6
//    D7 -| 32    8 |- D11
//   D10 -| 33    9 |- D12
//    D9 -| 34   10 |- D13
//    D8 -| 35   11 |- D15
//   D14 -| 36   12 |- A1
//   A17 -| 37   13 |- A2
//    A3 -| 38   14 |- A4
//        |---------|
//    A5 -| 39   15 |- A6
//    A7 -| 40   16 |- A8
//   A18 -| 41   17 |- A19
//    A9 -| 42   18 |- A10
//   A11 -| 43   19 |- A12
//   A13 -| 44   20 |- A14
//   A15 -| 45   21 |- A16
//   A20 -| 46   22 |- A21
//   A22 -| 47   23 |- GND
//  CSB1 -| 48   24 |- VCC
//        +---------+
//
// DATA D0-D7 - PORTC
// DATA D8-D15 - PORTA
// ADDR A1-A8 - PORTF
// ADDR A9-A16 - PORTK
// ADDR A17-A22 - PORTL
// CONTROL PINS - PORTH

//******************************************
// DEFINES
//******************************************
#define OE_HIGH PORTH |= (1<<3)   // SNES /CART
#define OE_LOW PORTH &= ~(1<<3)
#define CSB1_HIGH PORTH |= (1<<6) // SNES /RD
#define CSB1_LOW PORTH &= ~(1<<6)
#define CSB2_HIGH PORTH |= (1<<4) // SNES /IRQ
#define CSB2_LOW PORTH &= ~(1<<4)

//******************************************
// VARIABLES
//******************************************
byte VSMILE[] = {4,6,8,16};
byte vsmilelo = 0; // Lowest Entry
byte vsmilehi = 3; // Highest Entry
byte vsmilesize;
byte newvsmilesize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsVSMILE[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void vsmileMenu()
{
  convertPgm(menuOptionsVSMILE, 4);
  uint8_t mainMenu = question_box(F("V.SMILE MENU"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    // Select Cart
    case 0:
      setCart_VSMILE();
      setup_VSMILE();
      break;

    // Read ROM
    case 1:
      sd.chdir("/");
      readROM_VSMILE();
      sd.chdir("/");
      break;

    // Set Size
    case 2:
      setROMSize_VSMILE();
      break;

    // Reset
    case 3:
      resetArduino();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_VSMILE()
{
  // Request 3.3V
  setVoltage(VOLTS_SET_3V3);

  // Control Pins OE(PH3), CSB2(PH4), CSB1(PH6)
  DDRH = 0x58;  // 0b01011000 - CSB1, CSB2, OE [OUTPUT] - Unused Pins [INPUT]
  PORTH = 0xFF; // 0b11111111 - CSB1, CSB2, OE [HIGH] - Unused Pins [HIGH]

  // Address Pins
  DDRF = 0xFF;  // Address A1-A8 [OUTPUT]
  DDRK = 0xFF;  // Address A9-A16 [OUTPUT]
  DDRL = 0x3F;  // Address A17-A22 [OUTPUT] 0b00111111
  // Data Pins
  DDRC = 0x00;  // D0-D7 [INPUT]
  DDRA = 0x00;  // D8-D15 [INPUT]

  checkStatus_VSMILE();
  strcpy(romName, "VSMILE");

  mode = CORE_VSMILE;
}

//******************************************
// READ FUNCTIONS
//******************************************
// Max Single ROM Size 0x800000 (Highest WORD Address = 0x3FFFFF)
word read_rom_word_VSMILE(unsigned long address)
{
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  PORTF = address & 0xFF;
  _delay_us(1); // Need longer delay
  CSB2_HIGH;
  CSB1_LOW;
  OE_LOW;
  unsigned char data1 = PINC;
  unsigned char data2 = PINA;
  word data = (data1 << 8) | (data2);
  OE_HIGH;
  CSB1_HIGH;

  return data;
}

// VSMILE 2ND EPOXY CHIP [+2MB]
// CSB2 LOW ONLY
word read_rom2_word_VSMILE(unsigned long address)
{
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  PORTF = address & 0xFF;
  _delay_us(1); // Need longer delay
  CSB1_HIGH;
  CSB2_LOW;
  OE_LOW;
  _delay_us(1); // Need longer delay
  unsigned char data1 = PINC;
  unsigned char data2 = PINA;
  word data = (data1 << 8) | (data2);
  OE_HIGH;
  CSB2_HIGH;

  return data;
}

// VSMILE MOTION 16MB 2ND CHIP [+8MB]
// CSB1 + CSB2 LOW
word read_rom3_word_VSMILE(unsigned long address)
{
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  PORTF = address & 0xFF;
  CSB1_LOW;
  CSB2_LOW;
  OE_LOW;
  unsigned char data1 = PINC;
  unsigned char data2 = PINA;
  word data = (data1 << 8) | (data2);
  OE_HIGH;
  CSB1_HIGH;
  CSB2_HIGH;

  return data;
}

//******************************************
// READ ROM
//******************************************

void readROM_VSMILE()
{
  createFolderAndOpenFile("VSMILE", "ROM", romName, "bin");

  for (unsigned long address = 0; address < 0x200000; address += 256) { // 4MB
    for (unsigned int x = 0; x < 256; x++) {
      word tempword = read_rom_word_VSMILE(address + x); // CSB1 LOW [CSB2 HIGH]
      sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
      sdBuffer[(x * 2) + 1] = tempword & 0xFF;
    }
    myFile.write(sdBuffer, 512);
  }
  if (vsmilesize == 1) { // 6MB - 2 EPOXY CHIPS [4MB + 2MB] Alphabet Park/Care Bears
    for (unsigned long address = 0; address < 0x100000; address += 256) { // +2MB HIGH = 6MB
      for (unsigned int x = 0; x < 256; x++) {
        word tempword = read_rom2_word_VSMILE(address + x); // CSB2 LOW [CSB1 HIGH]
        sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
        sdBuffer[(x * 2) + 1] = tempword & 0xFF;
      }
      myFile.write(sdBuffer, 512);
    }
  }
  else if (vsmilesize > 1) { // Normal 8MB
    for (unsigned long address = 0x200000; address < 0x400000; address += 256) { // +4MB = 8MB
      for (unsigned int x = 0; x < 256; x++) {
        word tempword = read_rom_word_VSMILE(address + x); // CSB1 LOW [CSB2 HIGH]
        sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
        sdBuffer[(x * 2) + 1] = tempword & 0xFF;
      }
      myFile.write(sdBuffer, 512);
    }
    if (vsmilesize > 2) { // Motion 16MB [8MB + 8MB] - Cars 2/Shrek Forever After/Super WHY!/Toy Story 3
      for (unsigned long address = 0; address < 0x400000; address += 256) { // +8MB HIGH = 16MB
        for (unsigned int x = 0; x < 256; x++) {
          word tempword = read_rom3_word_VSMILE(address + x); // CSB1 + CSB2 LOW
          sdBuffer[x * 2] = (tempword >> 0x8) & 0xFF;
          sdBuffer[(x * 2) + 1] = tempword & 0xFF;
        }
        myFile.write(sdBuffer, 512);
      }
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

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_VSMILE(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(VSMILE[index]);
}
#endif

void setROMSize_VSMILE()
{
  byte newvsmilesize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (vsmilelo == vsmilehi)
    newvsmilesize = vsmilelo;
  else {
    newvsmilesize = navigateMenu(vsmilelo, vsmilehi, &printRomSize_VSMILE);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(VSMILE[newvsmilesize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (vsmilelo == vsmilehi)
    newvsmilesize = vsmilelo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (vsmilehi - vsmilelo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(VSMILE[i + vsmilelo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newvsmilesize = sizeROM.toInt() + vsmilelo;
    if (newvsmilesize > vsmilehi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(VSMILE[newvsmilesize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newvsmilesize);
  vsmilesize = newvsmilesize;
}

void checkStatus_VSMILE()
{
  EEPROM_readAnything(8, vsmilesize);
  if (vsmilesize > vsmilehi) {
    vsmilesize = 2; // default 8M
    EEPROM_writeAnything(8, vsmilesize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("V.SMILE READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(VSMILE[vsmilesize]);
  println_Msg(F("MB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(VSMILE[vsmilesize]);
  Serial.println(F("MB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_VSMILE()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("vsmilecart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
