//******************************************
// SEGA MASTER SYSTEM MODULE
//******************************************
// unfinished and untested

/******************************************
   Variables
 *****************************************/

/******************************************
   Menu
 *****************************************/
// MD menu items
static const char SMSMenuItem1[] PROGMEM = "Read Rom";
static const char SMSMenuItem2[] PROGMEM = "Reset";
static const char* const menuOptionsSMS[] PROGMEM = {SMSMenuItem1, SMSMenuItem2};


void smsMenu() {
  // create menu with title and 2 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSMS, 2);
  mainMenu = question_box(F("Sega Master System"), menuOptions, 2, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_SMS();
      break;

    case 1:
      // Reset
      asm volatile ("  jmp 0");
      break;
  }
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_SMS() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) OE(PH6)
  DDRH |=  (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;

  // Setting RST(PH0) CS(PH3) WR(PH5) OE(PH6) HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  delay(200);

  // Print all the info
  getCartInfo_SMS();
}

/******************************************
  Low level functions
*****************************************/
void writeByte_SMS(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t");

  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);
  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);

  // Leave WR low for at least 200ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

byte readByte_SMS(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);
  // Setting OE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Long delay here or there will be read errors
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = (PINC & 0xFF);

  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);
  // Setting OE(PH6) HIGH
  PORTH |= (1 << 6);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

// Switch data pins to write
void dataOut_SMS() {
  DDRC = 0xFF;
}

// Switch data pins to read
void dataIn_SMS() {
  DDRC = 0x00;
}

unsigned char hex2bcd (unsigned char x) {
  unsigned char y;
  y = (x / 10) << 4;
  y = y | (x % 10);
  return (y);
}

/******************************************
  MASTER SYSTEM functions
*****************************************/
void getCartInfo_SMS() {
  // Set control
  dataIn_SMS();

  // Rom size
  switch (readByte_SMS(0x7fff) & 0xFF) {
    case 0xa:
      cartSize =  8 * 1024;
      break;
    case 0xb:
      cartSize =  16 * 1024;
      break;
    case 0xc:
      cartSize =  32 * 1024;
      break;
    case 0xd:
      cartSize =  48 * 1024;
      break;
    case 0xe:
      cartSize =  64 * 1024;
      break;
    case 0xf:
      cartSize =  128 * 1024;
      break;
    case 0x0:
      cartSize =  256 * 1024;
      break;
    case 0x1:
      cartSize =  512 * 1024;
      break;
    case 0x2:
      cartSize =  512 * 1024;
      break;
  }

  // Get product code
  romName[0] = (readByte_SMS(0x7ffe) >> 4);
  romName[1] = hex2bcd(readByte_SMS(0x7ffd));
  romName[2] = hex2bcd(readByte_SMS(0x7ffc));

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Size: "));
  print_Msg(cartSize * 8 / 1024 );
  println_Msg(F(" KBit"));
  println_Msg(F(" "));

  // Wait for user input
#ifdef enable_OLED
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
#endif
}

// Read rom and save to the SD card
void readROM_SMS() {
  // Set control
  dataIn_SMS();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".SMS");

  // create a new folder
  EEPROM_readAnything(10, foldern);
  sprintf(folder, "SMS/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(10, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }


  for (unsigned long currBuffer = 0; currBuffer < cartSize; currBuffer += 512) {
    // Blink led
    if (currBuffer % 16384 == 0)
      PORTB ^= (1 << 4);

    for (int currByte = 0; currByte < 512; currByte++) {
      sdBuffer[currByte] = readByte_SMS(currBuffer + currByte);
    }
    myFile.write(sdBuffer, 512);
  }
  // Close the file:
  myFile.close();
}

//******************************************
// End of File
//******************************************
