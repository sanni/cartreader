//******************************************
// GAME BOY ADVANCE
//******************************************

/******************************************
   Variables
 *****************************************/
char calcChecksumStr[5];
byte cartBuffer[513];

const int nintendoLogo[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A,
  0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19,
  0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33,
  0x82, 0xE3, 0xCE, 0xBF, 0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0,
  0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27,
  0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00,
  0x40, 0xA7, 0x0E, 0xFD, 0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85,
  0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF,
  0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63,
  0x87, 0xF0, 0x3C, 0xAF, 0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07
};

/******************************************
   Menu
 *****************************************/
// GBA menu items
const char GBAMenuItem1[] PROGMEM = "Read Rom";
const char GBAMenuItem2[] PROGMEM = "Read Save";
const char GBAMenuItem3[] PROGMEM = "Write Save";
const char GBAMenuItem4[] PROGMEM = "Reset";
const char* const menuOptionsGBA[] PROGMEM = {GBAMenuItem1, GBAMenuItem2, GBAMenuItem3, GBAMenuItem4};

const char GBARomMenuItem1[] PROGMEM = "4MB";
const char GBARomMenuItem2[] PROGMEM = "8MB";
const char GBARomMenuItem3[] PROGMEM = "16MB";
const char GBARomMenuItem4[] PROGMEM = "32MB";
const char* const menuOptionsGBARom[] PROGMEM = {GBARomMenuItem1, GBARomMenuItem2, GBARomMenuItem3, GBARomMenuItem4};

void gbaMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGBA, 4);
  mainMenu = question_box("GBA Cart Reader", menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      // Read rom
      // create submenu with title and 4 options to choose from
      unsigned char GBARomMenu;
      // Copy menuOptions out of progmem
      convertPgm(menuOptionsGBARom, 4);
      GBARomMenu = question_box("Select ROM size", menuOptions, 4, 0);

      // wait for user choice to come back from the question box menu
      switch (GBARomMenu)
      {
        case 0:
          // 4MB
          cartSize = 0x400000;
          break;

        case 1:
          // 8MB
          cartSize = 0x800000;
          break;

        case 2:
          // 16MB
          cartSize = 0x1000000;
          break;

        case 3:
          // 32MB
          cartSize = 0x2000000;
          break;
      }

      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_GBA();
      sd.chdir("/");
      compare_checksum_GBA();
      break;

    case 1:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readSAVE_GBA();
      break;

    case 2:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      writeSAVE_GBA();
      unsigned long wrErrors;
      wrErrors = verifySAVE_GBA();
      if (wrErrors == 0) {
        println_Msg(F("Verified OK"));
        display_Update();
      }
      else {
        print_Msg(F("Error: "));
        print_Msg(wrErrors);
        println_Msg(F(" bytes "));
        print_Error(F("did not verify."), false);
      }
      break;

    case 3:
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
void setup_GBA() {
  // Set address/data pins to OUTPUT
  // AD0-AD7
  DDRF = 0xFF;
  // AD8-AD15
  DDRK = 0xFF;
  // AD16-AD23
  DDRC = 0xFF;

  // Output a HIGH signal
  // AD0-AD7
  PORTF = 0xFF;
  // AD8-AD15
  PORTK = 0xFF;
  // AD16-AD23
  PORTC = 0xFF;

  // Set Control Pins to Output CS_SRAM(PH0) CS_ROM(PH3) WR(PH5) RD(PH6)
  // CLK is N/C and IRQ is conected to GND inside the cartridge
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on CS_SRAM(PH0) CS_ROM(PH3) WR(PH5) RD(PH6)
  // At power-on all the control lines are high/disabled
  PORTH |= (1 << 0)  | (1 << 3) | (1 << 5) | (1 << 6);

  // Delay until all is stable
  delay(500);

  // Print start page
  getCartInfo_GBA();
  display_Clear();

  println_Msg(F("GBA Cart Info"));
  println_Msg("");
  print_Msg(F("Rom Name: "));
  println_Msg(romName);
  print_Msg(F("Cart ID: "));
  println_Msg(cartID);
  print_Msg(F("Checksum: "));
  println_Msg(checksumStr);
  print_Msg(F("Version: 1."));
  println_Msg(romVersion);
  println_Msg("");

  // Wait for user input
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
  Low level functions
*****************************************/
// Read one word and toggle both CS and RD
word readWord_GBA(unsigned long myAddress) {
  //divide address by two since we read two bytes per offset
  unsigned long currAddress = myAddress / 2;

  // Output address to address pins,
  PORTF = currAddress & 0xFF;
  PORTK = (currAddress >> 8) & 0xFF;
  PORTC = (currAddress >> 16) & 0xFF;

  // Wait 30ns, Arduino running at 16Mhz -> one operation = 62.5ns
  __asm__("nop\n\t");

  // Pull CS_ROM(PH3) LOW to latch the address
  PORTH &= ~(1 << 3);

  // Wait 120ns between pulling CS and RD LOW
  __asm__("nop\n\t""nop\n\t");

  // Set address/data pins to LOW, this is important
  PORTF = 0x0;
  PORTK = 0x0;
  // Set address/data ports to input so we can read, but don't enable pullups
  DDRF = 0x0;
  DDRK = 0x0;

  // Pull RD(PH6) to LOW to read 16 bytes of data
  PORTH &= ~ (1 << 6);

  // Wait 120ns for the cartridge rom to write the data to the datalines
  __asm__("nop\n\t""nop\n\t");

  // Switch  CS_ROM(PH3) RD(PH6) to HIGH
  PORTH |= (1 << 3)  | (1 << 6);

  // Read the data off the data lines on the rising edge of the RD line.
  word tempWord = ((PINF << 8) + PINK) & 0xFFFF;

  // Set address/data pins to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;
  // Output a high signal so there are no floating pins
  PORTF = 0XFF;
  PORTK = 0XFF;
  PORTC = 0XFF;

  // Return the read word
  return tempWord;
}

// Read multiple bytes into an array by toggling both CS and RD for each byte
void readBlock_GBA(unsigned long myAddress, byte myArray[] , int numBytes) {
  for (int currByte = 0; currByte < numBytes; currByte += 2) {
    unsigned long currAddress = myAddress + currByte;
    word currWord = readWord_GBA(currAddress);
    myArray[currByte] = (currWord >> 8) & 0xFF;
    myArray[currByte + 1] = currWord & 0xFF;
  }
}
/******************************************
  Game Boy functions
*****************************************/
// Read info out of rom header
void getCartInfo_GBA() {
  // Read Header into array
  readBlock_GBA(0, cartBuffer, 192);

  // Nintendo logo check
  for (int currByte = 0x4; currByte < 0xA0; currByte++) {
    if (pgm_read_byte(&nintendoLogo[currByte]) != cartBuffer[currByte]) {
      print_Error(F("Nintendo Logo Error"), false);
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;
    }
  }

  // Get cart ID
  cartID[0] = char(cartBuffer[0xAC]);
  cartID[1] = char(cartBuffer[0xAD]);
  cartID[2] = char(cartBuffer[0xAE]);
  cartID[3] = char(cartBuffer[0xAF]);

  // Dump name into 8.3 compatible format
  byte myByte = 0;
  byte myLength = 0;
  for (int addr = 0xA0; addr <= 0xAB; addr++) {
    myByte = cartBuffer[addr];
    if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 8) {
      romName[myLength] = char(myByte);
      myLength++;
    }
  }

  // Get ROM version
  romVersion = cartBuffer[0xBC];

  // Get Checksum as string
  sprintf(checksumStr, "%02X", cartBuffer[0xBD]);

  // Calculate Checksum
  int calcChecksum = 0x00;
  for (int n = 0xA0; n < 0xBD; n++) {
    calcChecksum -= cartBuffer[n];
  }
  calcChecksum = (calcChecksum - 0x19) & 0xFF;
  // Turn into string
  sprintf(calcChecksumStr, "%02X", calcChecksum);

  // Compare checksum
  if (strcmp(calcChecksumStr, checksumStr) != 0) {
    print_Msg(F("Result: "));
    println_Msg(calcChecksumStr);
    print_Error(F("Checksum Error"), false);
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
}

// Dump ROM
void readROM_GBA() {
  // Get name, add extension and convert to char array for sd lib
  char fileName[26];
  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // create a new folder for the rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  //clear the screen
  display_Clear();
  println_Msg(F("Creating folder: "));
  println_Msg(folder);
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }

  // Read rom
  for (int myAddress = 0; myAddress < cartSize; myAddress += 512) {
    // Fill cartBuffer starting at myAddress and reading 512 bytes into array cartBuffer
    readBlock_GBA(myAddress, sdBuffer, 512);

    // Write to SD
    myFile.write(sdBuffer, 512);

    // Pause between blocks, increase if you get errors every numBytes bytes
    delayMicroseconds(10);
  }

  // Close the file:
  myFile.close();

  // Signal end of process
  print_Msg(F("Saved as "));
  println_Msg(fileName);
}

// Calculate the checksum of the dumped rom
boolean compare_checksum_GBA () {
  println_Msg(F("Calculating Checksum"));
  display_Update();

  char fileName[26];
  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // last used rom folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ROM/%s/%d", romName, foldern - 1);
  sd.chdir(folder);

  // If file exists
  if (myFile.open(fileName, O_READ)) {
    // Read rom header
    myFile.read(sdBuffer, 512);
    myFile.close();

    // Calculate Checksum
    int calcChecksum = 0x00;
    for (int n = 0xA0; n < 0xBD; n++) {
      calcChecksum -= sdBuffer[n];
    }
    calcChecksum = (calcChecksum - 0x19) & 0xFF;

    // Turn into string
    sprintf(calcChecksumStr, "%02X", calcChecksum);

    if (strcmp(calcChecksumStr, checksumStr) == 0) {
      println_Msg(F("Checksum matches"));
      display_Update();
      return 1;
    }
    else {
      print_Msg(F("Result: "));
      println_Msg(calcChecksumStr);
      print_Error(F("Checksum Error"), false);
      return 0;
    }
  }
  // Else show error
  else {
    print_Error(F("Failed to open rom"), false);
    return 0;
  }
}

void readSAVE_GBA() {
}

void writeSAVE_GBA() {
}

unsigned long verifySAVE_GBA() {
}

//******************************************
// End of File
//******************************************
