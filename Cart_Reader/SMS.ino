//******************************************
// SEGA MASTER SYSTEM MODULE
//******************************************

#include "options.h"
#ifdef enable_MD

/******************************************
   Variables
 *****************************************/

/******************************************
   Menu
 *****************************************/
// MD menu items
static const char SMSMenuItem1[] PROGMEM = "Read Rom";
static const char SMSMenuItem2[] PROGMEM = "Read from SRAM";
static const char SMSMenuItem3[] PROGMEM = "Write to SRAM";
static const char SMSMenuItem4[] PROGMEM = "Change Retrode Mode";
static const char SMSMenuItem5[] PROGMEM = "Reset";
static const char* const menuOptionsSMS[] PROGMEM = {SMSMenuItem1, SMSMenuItem2, SMSMenuItem3, SMSMenuItem4, SMSMenuItem5};

// Set retrode_mode to true when using a retrode SMS/GG adapter
static bool retrode_mode = false;
static bool retrode_mode_sms = false; // true: SMS/Mark3 false: GG

void _smsMenu() {
  // create menu with title and 2 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  int noptions = sizeof(menuOptionsSMS) / sizeof(menuOptionsSMS[0]);
  convertPgm(menuOptionsSMS, noptions);
  mainMenu = question_box(retrode_mode ? (retrode_mode_sms ? F("Retrode:SMS") : F("Retrode:GG")) : F("SMS/GG Retrode:NO"), menuOptions, noptions, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      mode = mode_SMS;
      setup_SMS();
      // Change working dir to root
      sd.chdir("/");
      readROM_SMS();
      break;

    case 1:
      display_Clear();
      mode = mode_SMS;
      setup_SMS();
      // Change working dir to root
      sd.chdir("/");
      readSRAM_SMS();
      break;

    case 2:
      display_Clear();
      mode = mode_SMS;
      setup_SMS();
      // Change working dir to root
      sd.chdir("/");
      writeSRAM_SMS();
      break;

    case 3:
      if (!retrode_mode && !retrode_mode_sms) {
        // first state (default)
        retrode_mode = true; // Change to GG
      } else if (retrode_mode && !retrode_mode_sms) {
        // second state
        retrode_mode_sms = true; // Change to SMS
      } else {
        // third state, reset to the first state
        retrode_mode = false;
        retrode_mode_sms = false;
      }
      break;

    case 4:
      // Reset
      resetArduino();
      break;
  }
  println_Msg(retrode_mode ? (retrode_mode_sms ? F("Retrode Mode SMS") : F("Retrode Mode GG")) : F("Retrode Mode Off"));
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

void smsMenu() {
  for (;;) _smsMenu();
}

/******************************************
   Setup
 *****************************************/
void setup_SMS() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A14
  DDRK = 0xFF;
  //A15
  DDRH |= (1 << 3);

  if (retrode_mode) {
    // Revert changes from the other mode
    PORTH &= ~((1 << 0) | (1 << 3) | (1 << 5));
    PORTL &= ~(1 << 1);
    DDRH &= ~((1 << 0) | (1 << 5));
    DDRL &= ~((1 << 1));
    // Set Control Pins to Output OE(PH6)
    DDRH |= (1 << 6);
    // WR(PL5) and RD(PL6)
    DDRL |= (1 << 5) | (1 << 6);

    // Setting OE(PH6) HIGH
    PORTH |= (1 << 6);
    // Setting WR(PL5) and RD(PL6) HIGH
    PORTL |= (1 << 5) | (1 << 6);
  } else {
    // Revert changes from the other mode
    PORTL &= ~((1 << 5) | (1 << 6));
    DDRL &= ~((1 << 5) | (1 << 6));
    // Set Control Pins to Output RST(PH0) WR(PH5) OE(PH6)
    DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
    // CE(PL1)
    DDRL |= (1 << 1);

    // Setting RST(PH0) WR(PH5) OE(PH6) HIGH
    PORTH |= (1 << 0) | (1 << 5) | (1 << 6);
    // CE(PL1)
    PORTL |= (1 << 1);
  }

  // ROM has 16KB banks which can be mapped to one of three slots via register writes
  // Register Slot Address space
  // $fffd    0    $0000-$3fff
  // $fffe    1    $4000-$7fff
  // $ffff    2    $8000-$bfff
  // Disable sram
  writeByte_SMS(0xFFFC, 0);
  // Map first 3 banks so we can read-out the header info
  writeByte_SMS(0xFFFD, 0);
  writeByte_SMS(0xFFFE, 1);
  writeByte_SMS(0xFFFF, 2);

  delay(400);

  // Print all the info
  getCartInfo_SMS();
}

/******************************************
  Low level functions
*****************************************/
void writeByte_SMS(word myAddress, byte myData) {
  if (retrode_mode && !retrode_mode_sms) {
    // Set Data Pins (D8-D15) to Output
    DDRA = 0xFF;
  } else {
    // Set Data Pins (D0-D7) to Output
    DDRC = 0xFF;
  }

  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  if (!retrode_mode) {
    // CE(PH3) and OE(PH6) are connected
    PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
  }

  // Output data
  if (retrode_mode && !retrode_mode_sms) {
    PORTA = myData;
  } else {
    PORTC = myData;
  }

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  if (retrode_mode) {
    // Switch WR(PL5) and OE/CE(PH6) to LOW
    PORTL &= ~(1 << 5);
    PORTH &= ~(1 << 6);
  } else {
    // Switch CE(PL1) and WR(PH5) to LOW
    PORTL &= ~(1 << 1);
    PORTH &= ~(1 << 5);
  }

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  if (retrode_mode) {
    // Switch WR(PL5) and OE/CE(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTL |= (1 << 5);
  } else {
    // Switch CE(PL1) and WR(PH5) to HIGH
    PORTH |= (1 << 5);
    PORTL |= (1 << 1);
  }

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  if (retrode_mode && !retrode_mode_sms) {
    // Set Data Pins (D8-D15) to Input
    DDRA = 0x00;
  } else {
    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
  }
}

byte readByte_SMS(word myAddress) {
  if (retrode_mode && !retrode_mode_sms) {
    // Set Data Pins (D8-D15) to Input
    DDRA = 0x00;
  } else {
    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
  }

  // Set Address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  if (!retrode_mode) {
    // CE(PH3) and OE(PH6) are connected
    PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
  }

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  if (retrode_mode) {
    // Switch RD(PL6) and OE(PH6) to LOW
    PORTL &= ~(1 << 6);
    PORTH &= ~(1 << 6);
  } else {
    // Switch CE(PL1) and OE(PH6) to LOW
    PORTL &= ~(1 << 1);
    PORTH &= ~(1 << 6);
  }

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = (retrode_mode && !retrode_mode_sms) ? PINA : PINC;

  if (retrode_mode) {
    // Switch RD(PL6) and OE(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTL |= (1 << 6);
  } else {
    // Switch CE(PL1) and OE(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTL |= (1 << 1);
  }

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

//unsigned char hex2bcd (unsigned char x) {
//  unsigned char y;
//  y = (x / 10) << 4;
//  y = y | (x % 10);
//return (y);
//}

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
    case 0x3:
      // 0x3 is (only?) used in The Pro Yakyuu '91 (Game Gear)
      cartSize =  128 * 1024UL;
      break;
    default:
      cartSize =  48 * 1024UL;
      // LED Error
      setColor_RGB(0, 0, 255);
      break;

  }

  // Read TMR SEGA string
  for (byte i = 0; i < 8; i++) {
    romName[i] = char(readByte_SMS(0x7ff0 + i));
  }
  romName[8] = '\0';

  // Attempt to detect cart size by checking if TMR SEGA is mirrored
  unsigned long mirror_offset = cartSize;
  char romName2[9];
  while (mirror_offset < 1024 * 1024UL) {
    byte bank = 1 + (mirror_offset / (16 * 1024UL));
    writeByte_SMS(0xFFFE, bank);
    for (byte i = 0; i < 8; i++) {
      romName2[i] = char(readByte_SMS(0x7ff0 + i));
    }
    romName2[8] = '\0';
    // print_Msg(F("Name2: "));
    // println_Msg(romName2);
    // print_Msg(F("from bank "));
    // print_Msg(bank);
    // print_Msg(F(" offset "));
    // print_Msg_PaddedHex32(mirror_offset + 0x7ff0);
    // println_Msg(F(""));
    if (strcmp(romName2, romName) == 0) {
      break;
    }
    if (cartSize == 48 * 1024UL) {
      cartSize = 64 * 1024UL;
    } else {
      cartSize *= 2;
    }
    mirror_offset = cartSize;
  }
  writeByte_SMS(0xFFFE, 1);

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Size: "));
  print_Msg(cartSize / 1024);
  println_Msg(F("KB"));
  println_Msg(F(" "));

  if (strcmp(romName, "TMR SEGA") != 0) {
    print_Error(F("Not working yet"), false);
    sprintf(romName, "ERROR");
    cartSize =  48 * 1024UL;
  }

  // Wait for user input
#if defined(enable_LCD) || defined(enable_OLED)
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
#endif
  // Turn off LED
  setColor_RGB(0, 0, 0);
}

// Read rom and save to the SD card
void readROM_SMS() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".SMS");

  // create a new folder
  EEPROM_readAnything(0, foldern);
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
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }
  word bankSize = 16 * 1024UL;
  for (byte currBank = 0x0; currBank < (cartSize / bankSize); currBank++) {
    // Write current 16KB bank to slot 2 register 0xFFFF
    writeByte_SMS(0xFFFF, currBank);

    // Blink led
    blinkLED();
    // Read 16KB from slot 2 which starts at 0x8000
    for (word currBuffer = 0; currBuffer < bankSize; currBuffer += 512) {
      // Fill SD buffer
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_SMS(0x8000 + currBuffer + currByte);
      }
      // hexdump for debugging:
      // if (currBank == 0 && currBuffer == 0) {
      //   for (word xi = 0; xi < 0x100; xi++) {
      //     if (xi%16==0) {
      //       print_Msg_PaddedHex16(xi);
      //       print_Msg(F(" "));
      //     }
      //     print_Msg_PaddedHexByte(sdBuffer[xi]);
      //     if (xi>0&&((xi+1)%16)==0) {
      //       println_Msg(F(""));
      //     } else {
      //       print_Msg(F(" "));
      //     }
      //   }
      // }
      myFile.write(sdBuffer, 512);
    }
  }
  // Close the file:
  myFile.close();
}

// Read SRAM and save to the SD card
void readSRAM_SMS() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".SAV");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "SMS/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }
  // Write the whole 32KB
  // When there is only 8KB of SRAM, the contents should be duplicated
  word bankSize = 16 * 1024UL;
  for (byte currBank = 0x0; currBank < 2; currBank++) {
    writeByte_SMS(0xFFFC, 0x08 | (currBank << 2));

    // Blink led
    blinkLED();
    // Read 16KB from slot 2 which starts at 0x8000
    for (word currBuffer = 0; currBuffer < bankSize; currBuffer += 512) {
      // Fill SD buffer
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_SMS(0x8000 + currBuffer + currByte);
      }
      myFile.write(sdBuffer, 512);
    }
  }
  // Close the file:
  myFile.close();
}

void writeSRAM_SMS() {
  display_Clear();

  if (false) {
    print_Error(F("DISABLED"), false);
  }
  else {
    fileBrowser(F("Select file"));

    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);

    display_Clear();
    println_Msg(F("Restoring from "));
    println_Msg(filePath);
    println_Msg(fileName);
    display_Update();

    if (myFile.open(filePath, O_READ)) {
      // Get SRAM size from file, with a maximum of 32KB
      uint32_t sramSize = myFile.fileSize();
      if (sramSize > (32 * 1024)) {
        sramSize = 32 * 1024;
      }
      print_Msg(F("sramSize: "));
      print_Msg(sramSize);
      println_Msg(F(""));
      word bankSize = 16 * 1024;
      for (word address = 0x0; address < sramSize; address += 512) {
        byte currBank = address >= bankSize ? 1 : 0;
        word page_address = address - (currBank * bankSize);
        writeByte_SMS(0xFFFC, 0x08 | (currBank << 2));
        // Blink led
        blinkLED();
        myFile.read(sdBuffer, 512);
        for (int x = 0; x < 512; x++) {
          writeByte_SMS(0x8000 + page_address + x, sdBuffer[x]);
        }
      }
      myFile.close();

      // Blink led
      blinkLED();

      println_Msg(F(""));
      println_Msg(F("DONE"));
      display_Update();
    }
    else {
      print_Error(F("SD ERROR"), true);
    }
  }

  display_Clear();

  sd.chdir(); // root
  filePath[0] = '\0'; // Reset filePath
}
#endif

//******************************************
// End of File
//******************************************
