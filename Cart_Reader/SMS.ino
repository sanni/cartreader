//******************************************
// SEGA MASTER SYSTEM MODULE
//******************************************
// unfinished and not working

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
      setup_SMS();
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

  // Setting RST(PH0) CS(PH3) WR(PH5) OE(PH6) HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  // ROM has 16KB banks which can be mapped to one of three slots via register writes
  // Register Slot Address space
  // $fffd    0    $0000-$3fff
  // $fffe    1    $4000-$7fff
  // $ffff    2    $8000-$bfff
  // Map first 3 banks so we can read-out the header info
  writeByte_SMS(0xFFFD, 0);
  writeByte_SMS(0xFFFE, 1);
  writeByte_SMS(0xFFFF, 2);

  delay(200);

  // Print all the info
  getCartInfo_SMS();
}

/******************************************
  Low level functions
*****************************************/
void writeByte_SMS(unsigned long myAddress, byte myData) {
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  // Set Data Pins (D0-D15) to Output
  DDRC = 0xFF;
  // Output data
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t");

  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);
  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 200ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
}

byte readByte_SMS(unsigned long myAddress) {
  byte tempByte;

  // Set Address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);
  // Setting OE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Long delay here or there will be read errors
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  tempByte = PINC;

  // Setting OE(PH6) HIGH
  PORTH |= (1 << 6);
  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

unsigned char hex2bcd (unsigned char x) {
  unsigned char y;
  y = (x / 10) << 4;
  y = y | (x % 10);
  return (y);
}

byte readNibble(byte data, byte number) {
  return ((data >> (number * 4)) & 0xf);
}

/******************************************
  MASTER SYSTEM functions
*****************************************/
void getCartInfo_SMS() {
  // Rom size
  switch (readNibble(readByte_SMS(0x7fff), 0)) {
    case 0xa:
      // Adding UL gets rid of integer overflow compiler warning
      cartSize =  8 * 1024UL;
      break;
    case 0xb:
      cartSize =  16 * 1024UL;
      break;
    case 0xc:
      cartSize =  32 * 1024UL;
      break;
    case 0xd:
      cartSize =  48 * 1024UL;
      break;
    case 0xe:
      cartSize =  64 * 1024UL;
      break;
    case 0xf:
      cartSize =  128 * 1024UL;
      break;
    case 0x0:
      cartSize =  256 * 1024UL;
      break;
    case 0x1:
      cartSize =  512 * 1024UL;
      break;
    case 0x2:
      cartSize =  512 * 1024UL;
      break;
  }

  // Read TMR Sega string
  for (byte i = 0; i < 8; i++) {
    romName[i] = char(readByte_SMS(0x7ff0 + i));
  }
  romName[8] = '\0';

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Size: "));
  print_Msg(cartSize * 8 / 1024 );
  println_Msg(F(" KBit"));
  println_Msg(F(" "));

  if (strcmp(romName, "TMR SEGA") != 0)
    print_Error(F("Not working yet"), true);

  // Wait for user input
#ifdef enable_OLED
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
#endif
}

// Read rom and save to the SD card
void readROM_SMS() {
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
  unsigned long bankSize = 16 * 1024UL;
  for (unsigned long currBank = 0x00; currBank < (cartSize / bankSize); currBank++) {
    // Write current 16KB bank to slot 2 register 0xFFFF
    writeByte_SMS(0xFFFF, currBank);

    // Blink led
    PORTB ^= (1 << 4);
    // Read 16KB from slot 2 which starts at 0x8000
    for (unsigned long currBuffer = 0; currBuffer < bankSize; currBuffer += 512) {
      // Fill SD buffer
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_SMS(0x8000 + currBuffer + currByte);
      }
      myFile.write(sdBuffer, 512);
      //printSdBuffer();
    }
  }
  // Close the file:
  myFile.close();
}

//******************************************
// End of File
//******************************************
