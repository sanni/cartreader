//******************************************
// LITTLE JAMMER PRO MODULE
//******************************************
#ifdef ENABLE_LJPRO
// Little Jammer Pro
// Cartridge Pinout
// 48P 1.25mm pitch connector
// 
// FORM FACTOR IS SAME AS BANDAI WONDERSWAN/BENESSE POCKET CHALLENGE V2/LITTLE JAMMER
// WIRING IS COMPLETELY DIFFERENT!
//
// LEFT SIDE
// 1 GND
// 2 GND
// 3 S1 (GND)
// 4 S2 (GND)
// 5 U1_WP#/ACC
// 6 U1_SCLK
// 7 U1_SCLK
// 8 U1_SI
// 9 U1_SI
// 10 U1_SO/PO7
// 11 U1_SO/PO7
// 12 U1_PO6
// 13 U1_PO5
// 14 U1_PO4
// 15 U1_PO3
// 16 U1_PO2
// 17 U1_PO1
// 18 U1_PO0
// 19 U1_CS#
// 20 U1_CS#
// 21 U1_HOLD#
// 22 U1_HOLD#
// 23 VCC (+3V)
// 24 VCC (+3V)
// 25 VCC (+3V)
// 26 VCC (+3V)
// 27 U2_SCLK
// 28 U2_SCLK
// 29 U2_SI
// 30 U2_SI
// 31 U2_SO/PO7
// 32 U2_SO/PO7
// 33 U2_PO6
// 34 U2_PO5
// 35 U2_PO4
// 36 U2_PO3
// 37 U2_PO2
// 38 U2_PO1
// 39 U2_PO0
// 40 U2_CS#
// 41 U2_CS#
// 42 U2_HOLD#
// 43 U2_HOLD#
// 44 U2_WP#/ACC
// 45 S3 (GND)
// 46 S4 (GND)
// 47 GND
// 48 GND
// RIGHT SIDE

// CONTROL PINS:
// U1_HOLD#    (PH4) - SNES /IRQ
// U1_CS#      (PK0) - SNES A8
// U1_SI       (PK1) - SNES A9
// U1_WP#/ACC  (PK2) - SNES A10
//
// U2_HOLD#    (PH0) - SNES RESET
// U2_SI       (PH3) - SNES /CS
// U2_WP#/ACC  (PH5) - SNES /WR
// U2_CS#      (PH6) - SNES /RD
//
// S1          (PK4) - SNES A12
// S2          (PK5) - SNES A13
// S3          (PK6) - SNES A14
// S4          (PK7) - SNES A15

// COMBINE U1_SCLK + U2_SCLK INTO SINGLE SCLK
// SCLK(PH1)   - SNES CPUCLK

// DATA PINS:
// U1 D0-D7 (PORTF)
// U2 D0-D7 (PORTC)

// NOTES:
// HOLD# NOT USED FOR PARALLEL MODE - PULLED UP TO VCC ON CARTS
// WP#/ACC PULLED DOWN TO GND ON CARTS

//******************************************
// DEFINES
//******************************************
#define CS1_LOW   PORTK &= ~(1 << 0)
#define CS1_HIGH  PORTK |= (1 << 0)
#define CS2_LOW   PORTH &= ~(1 << 6)
#define CS2_HIGH  PORTH |= (1 << 6)

//******************************************
// VARIABLES
//******************************************
byte LJPRO[] = {2,4,6,8};
byte ljprolo = 0; // Lowest Entry
byte ljprohi = 3; // Highest Entry
byte ljprosize;
byte newljprosize;

char mnfID[3];
char deviceID_str[5];
boolean ljproflash1found = false;
boolean ljproflash2found = false;
byte ljproflash1size;
byte ljproflash2size;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsLJPRO[] PROGMEM = { FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

// U1_HOLD#(PH4)    - SNES /IRQ
// U1_CS#           - SNES A8
// U1_WP#/ACC       - SNES A9
// U1_SI            - SNES A10
//
// U2_HOLD#(PH0)    - SNES RESET
// U2_SI(PH3)       - SNES /CS
// U2_WP#/ACC(PH5)  - SNES /WR
// U2_CS#(PH6)      - SNES /RD

void ljproMenu()
{
  convertPgm(menuOptionsLJPRO, 3);
  uint8_t mainMenu = question_box(F("LITTLE JAMMER PRO"), menuOptions, 3, 0);

  switch (mainMenu)
  {
    case 0:
      // Read ROM
      sd.chdir("/");
      readROM_LJPRO();
      sd.chdir("/");
      break;

    case 1:
      // Set Size
      setROMSize_LJPRO();
      break;

    case 2:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_LJPRO()
{
  // Request 3.3V
  setVoltage(VOLTS_SET_3V3);

  // LITTLE JAMMER PRO uses Serial Flash
  // Set Data Pins to Input
  DDRF = 0x00; // U1 Data
  DDRC = 0x00; // U2 Data
  // Set Unused Address Pins to Output
  DDRL = 0xFF;

  // Set Control Pins to Output
  //     U2_HLD(PH0) SCLK(PH1)  U2_SI(PH3) U1_HLD(PH4) U2_WP(PH5) U2_CS(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  //      U1_CS(PK0) U1_SI(PK1) U1_WP(PK2) --------
  DDRK |=  (1 << 0) | (1 << 1) | (1 <<2) | (1 << 3);

  // FLASH Configuration Pins to Input
  //         S1(PK4)    S2(PK5)    S3(PK6)    S4(PK7)
  DDRK &= ~((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Setting Control Pins to HIGH
  //     U2_HLD(PH0) SCLK(PH1)  U2_SI(PH3) U1_HLD(PH4) U2_WP(PH5) U2_CS(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  //      U1_CS(PK0)  U1_SI(PK1) U1_WP(PK2)  --------    S1(PK4)    S2(PK5)    S3(PK6)    S4(PK7)
  PORTK |=  (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_LJPRO();
  strcpy(romName, "LJPRO");

  mode = CORE_LJPRO;
}

//******************************************
// SERIAL MODE
//******************************************
// 25L1605/25L3205
// Default Serial Mode

void sendSerial_U1(uint8_t data)
{
  for (int i = 0; i < 8; i++) {
    PORTH &= ~(1 << 1); // SCLK LOW
    if ((data >> 7) & 0x1) { // Bit is HIGH
      PORTK |= (1 << 1); // U1_SI HIGH;
    }
    else {
      PORTK &= ~(1 << 1); // U1_SI LOW;
    }
    PORTH |= (1 << 1); // SCLK HIGH
    // rotate to the next bit
    data <<= 1;
  }
}

void sendSerial_U2(uint8_t data)
{
  for (int i = 0; i < 8; i++) {
    PORTH &= ~(1 << 1); // SCLK LOW
    if ((data >> 7) & 0x1) { // Bit is HIGH
      PORTH |= (1 << 3); // U2_SI HIGH;
    }
    else {
      PORTH &= ~(1 << 3); // U2_SI LOW;
    }
    PORTH |= (1 << 1); // SCLK HIGH
    // rotate to the next bit
    data <<= 1;
  }
}

uint8_t readSerial_U1()
{
  bool serBits[9];
  for (byte i = 0; i < 8; i++) {
    pulseClock_LJPRO(1);
    serBits[i] = (PINF >> 7) & 0x1;
  }
  byte tempdata = serBits[0] << 7 | serBits[1] << 6 | serBits[2] << 5 | serBits[3] << 4 | serBits[4] << 3 | serBits[5] << 2 | serBits[6] << 1 | serBits[7];

  return tempdata;
}

uint8_t readSerial_U2()
{
  bool serBits[9];
  for (byte i = 0; i < 8; i++) {
    pulseClock_LJPRO(1);
    serBits[i] = (PINC >> 7) & 0x1;
  }
  byte tempdata = serBits[0] << 7 | serBits[1] << 6 | serBits[2] << 5 | serBits[3] << 4 | serBits[4] << 3 | serBits[5] << 2 | serBits[6] << 1 | serBits[7];

  return tempdata;
}

//******************************************
// PARALLEL MODE
//******************************************
// 25L1605/25L3205
// Parallel Mode - Command 0x55
// SCLK Frequency 1.2MHz (Cycle 833.33ns)
// READ 0x03
// WRITE 0x02

void pulseClock_LJPRO(unsigned int times)
{
  for (unsigned int i = 0; i < (times * 2); i++) {
    // Switch the clock pin to 0 if it's 1 and 0 if it's 1
    PORTH ^= (1 << 1);
    // without the delay the clock pulse would be 1.5us and 666kHz
    //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"));
  }
}

// Send one byte of data to Serial FLASH [Parallel Mode]
void sendData_U1(byte data)
{
  DDRF = 0xFF; // U1 Data Output
  PORTF = data;
  pulseClock_LJPRO(8);
  DDRF = 0x00; // U1 Data Input
}

void sendData_U2(byte data)
{
  DDRC = 0xFF; // U2 Data Output
  PORTC = data;
  pulseClock_LJPRO(8);
  DDRC = 0x00; // U2 Data Input
}

void readData_U1(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int x = 0; x < 512; x++) {
      pulseClock_LJPRO(1);
      sdBuffer[x] = PINF;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readData_U2(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int x = 0; x < 512; x++) {
      pulseClock_LJPRO(1);
      sdBuffer[x] = PINC;
    }
    myFile.write(sdBuffer, 512);
  }
}

// RDID
// Manufacturer 0xC2
// Memory Density 0x20
// Device ID 0x15 [25L1605]/0x16 [25L3205]

// REMS
// Manufacturer 0xC2
// Device ID 0x14 [25L1605]/0x15 [25L3205]

void readID_U1() // Parallel Mode
{
  CS1_LOW; // U1 LOW
  sendSerial_U1(0x9F); // RDID Command
  pulseClock_LJPRO(1);
  byte id0 = PINF; // 0xC2
  pulseClock_LJPRO(1);
  byte id1 = PINF; // 0x20
  pulseClock_LJPRO(1);
  byte id2 = PINF; // 0x15 [MX25L1605]/0x16 [MX25L3205]
  CS1_HIGH; // U1 HIGH
  // Flash ID
  sprintf(mnfID, "%02X", id0);
  sprintf(deviceID_str, "%02X%02X", id1, id2);
//  println_Msg(mnfID);
//  println_Msg(deviceID_str);
//  display_Update();
  if(strcmp(deviceID_str, "2015") == 0) { // MX25L1605
    ljproflash1found = 1;
    ljproflash1size = 2;
    display_Clear();
    println_Msg(F("U1 MX25L1605 FOUND"));
    display_Update();
  }
  else if (strcmp(deviceID_str, "2016") == 0) { // MX25L3205
    ljproflash1found = 1;
    ljproflash1size = 4;
    display_Clear();
    println_Msg(F("U1 MX25L3205 FOUND"));
    display_Update();
  }
}

void readID_U2() // Parallel Mode
{
  CS2_LOW; // U2 LOW
  sendSerial_U2(0x9F); // RDID Command
  pulseClock_LJPRO(1);
  byte id0 = PINC; // 0xC2
  pulseClock_LJPRO(1);
  byte id1 = PINC; // 0x20
  pulseClock_LJPRO(1);
  byte id2 = PINC; // 0x15 [MX25L1605]/0x16 [MX25L3205]
  pulseClock_LJPRO(1);
  CS2_HIGH; // U2 HIGH
  // Flash ID
  sprintf(mnfID, "%02X", id0);
  sprintf(deviceID_str, "%02X%02X", id1, id2);
//  println_Msg(mnfID);
//  println_Msg(deviceID_str);
//  display_Update();
  if(strcmp(deviceID_str, "2015") == 0) { // MX25L1605
    ljproflash2found = 1;
    ljproflash2size = 2;
    println_Msg(F("U2 MX25L1605 FOUND"));
    display_Update();
  }
  else if (strcmp(deviceID_str, "2016") == 0) { // MX25L3205
    ljproflash2found = 1;
    ljproflash2size = 4;
    println_Msg(F("U2 MX25L3205 FOUND"));
    display_Update();
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_LJPRO() 
{
  createFolderAndOpenFile("LJPRO", "ROM", romName, "bin");

  // Little Jammer Pro PCB B1043-02A
  // Footprints for two 25L1605/25L3205 chips
  // Test carts only have one 25L1605 (2MB) installed
  // PCB could possibly install two 25L3205 chips (2x4MB = 8MB)

  // Set U1 FLASH to Parallel Mode
  CS1_LOW; // U1 LOW
  sendSerial_U1(0x55); // Parallel Mode
  CS1_HIGH; // U1 HIGH
  // Read ID
  readID_U1();
  // Set U2 FLASH to Parallel Mode
  CS2_LOW; // U2 LOW
  sendSerial_U2(0x55); // Parallel Mode
  CS2_HIGH; // U2 HIGH
  // Read ID
  readID_U2();

  // Read U1
  println_Msg(F("Reading U1..."));
  display_Update();
  CS1_LOW; // U1 LOW
  DDRF = 0x00; // U1 Data Input
  sendSerial_U1(0x03); // Read Array (Parallel)
  sendSerial_U1(0x00); // Address A23-A16
  sendSerial_U1(0x00); // Address A15-A8
  sendSerial_U1(0x00); // Address A7-A0
  readData_U1(0x000000, 0x200000);
  if (ljproflash1size == 4) { // 4MB
    readData_U1(0x200000, 0x400000);
  }
  CS1_HIGH; // U1 HIGH
  if (ljproflash2found) {
    // Read U2
    println_Msg(F("Reading U2..."));
    display_Update();
    CS2_LOW; // U2 LOW
    DDRC = 0x00; // U2 Data Input
    sendSerial_U2(0x03); // Read Array (Parallel)
    sendSerial_U2(0x00); // Address A23-A16
    sendSerial_U2(0x00); // Address A15-A8
    sendSerial_U2(0x00); // Address A7-A0
    readData_U2(0x000000, 0x200000);
    if (ljproflash2size == 4) { // 4MB
      readData_U2(0x200000, 0x400000);
    }
    CS2_HIGH; // U2 HIGH
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
void printRomSize_LJPRO(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(LJPRO[index]);
}
#endif

void setROMSize_LJPRO()
{
  byte newljprosize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (ljprolo == ljprohi)
    newljprosize = ljprolo;
  else {
    newljprosize = navigateMenu(ljprolo, ljprohi, &printRomSize_LJPRO);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(LJPRO[newljprosize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (ljprolo == ljprohi)
    newljprosize = ljprolo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (ljprohi - ljprolo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(LJPRO[i + ljprolo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newljprosize = sizeROM.toInt() + ljprolo;
    if (newljprosize > ljprohi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(LJPRO[newljprosize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newljprosize);
  ljprosize = newljprosize;
}

void checkStatus_LJPRO()
{
  EEPROM_readAnything(8, ljprosize);
  if (ljprosize > ljprohi) {
    ljprosize = 0; // default 2M
    EEPROM_writeAnything(8, ljprosize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("LITTLE JAMMER PRO"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(LJPRO[ljprosize]);
  println_Msg(F("MB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(LJPRO[ljprosize]);
  Serial.println(F("MB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}
#endif
