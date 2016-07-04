//******************************************
// FLASHROM
//******************************************

/******************************************
   Variables
 *****************************************/
// Flashrom
char flashid[5];
unsigned long flashSize;
byte flashromType;
byte secondID = 1;
unsigned long time;
unsigned long blank;

/******************************************
   Menu
 *****************************************/
// Flash menu items
const char flashMenuItem1[] PROGMEM = "Blankcheck";
const char flashMenuItem2[] PROGMEM = "Erase";
const char flashMenuItem3[] PROGMEM = "Read";
const char flashMenuItem4[] PROGMEM = "Write";
const char flashMenuItem5[] PROGMEM = "ID";
const char flashMenuItem6[] PROGMEM = "Print";
const char flashMenuItem7[] PROGMEM = "Reset";
const char* const menuOptionsFLASH[] PROGMEM = {flashMenuItem1, flashMenuItem2, flashMenuItem3, flashMenuItem4, flashMenuItem5, flashMenuItem6, flashMenuItem7};

void flashromMenu() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions of of progmem
  convertPgm(menuOptionsFLASH, 7);
  mainMenu = question_box("Flashrom Writer", menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      blankcheck_Flash();
      break;

    case 1:
      display_Clear();
      println_Msg(F("Erasing Flashrom"));
      println_Msg(F("Please wait..."));
      display_Update();
      time = millis();
      if (flashromType == 1) {
        eraseFlash29F032();
      }
      else {
        eraseFlash29F1610();
        delay(1000);
        busyCheck29F1610();
      }
      println_Msg(F("Flashrom erased"));
      display_Update();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      break;

    case 2:
      time = millis();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      readFlash();
      break;

    case 3:
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser("Select File");
      display_Clear();
      time = millis();
      if (flashromType == 1)
        writeFlash29F032();
      else if (flashromType == 2)
        writeFlash29F1610();

      // Reset twice just to be sure
      delay(1000);
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      delay(1000);
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      delay(1000);
      verifyFlash();
      break;

    case 4:
      time = 0;
      display_Clear();
      println_Msg(F("ID Flashrom"));
      if (flashromType == 1)
        idFlash29F032();
      else
        idFlash29F1610();
      println_Msg(flashid);
      display_Update();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      printFlash(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      delay(500);
      asm volatile ("  jmp 0");
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took : "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  print_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_Flash() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output OE(PH1) WE(PH4) CE(PH6)
  DDRH |=  (1 << 1) | (1 << 4) | (1 << 6);
  // Setting OE(PH1) WE(PH4) HIGH
  PORTH |= (1 << 1) | (1 << 4);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;

  // ID flash
  idFlash29F032();

  // Print start screen
idtheflash:
  display_Clear();
  display_Update();
  println_Msg(F("Flashrom Writer 8bit"));
  println_Msg(" ");
  println_Msg(" ");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid);

  if (strcmp(flashid, "C2F1") == 0) {
    println_Msg(F("MX29F1610 detected"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F9") == 0) {
    println_Msg(F("MX29L3211 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if (strcmp(flashid, "0141") == 0) {
    println_Msg(F("AM29F032B"));
    flashSize = 4194304;
    flashromType = 1;
  }
  else if (strcmp(flashid, "01AD") == 0) {
    println_Msg(F("AM29F016B"));
    flashSize = 2097152;
    flashromType = 1;
  }
  else if (strcmp(flashid, "04D4") == 0) {
    println_Msg(F("MBM29F033C"));
    flashSize = 4194304;
    flashromType = 1;
  }
  else if (secondID) {
    idFlash29F1610();
    secondID = 0;
    goto idtheflash;
  }
  else {
    print_Error(F("Unknown flashrom"), true);
  }
  println_Msg(" ");
  println_Msg(F("Press Button..."));
  display_Update();
  if (flashromType == 1)
    resetFlash29F032();
  else
    resetFlash29F1610();
  wait();
}

/******************************************
   I/O Functions
 *****************************************/
// Switch data pins to read
void dataIn_Flash() {
  // Set to Input
  DDRC = 0x00;
}

/******************************************
   Low level functions
 *****************************************/
void writeByte_Flash(unsigned long myAddress, byte myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t");

  // Switch WE(PH4) to LOW
  PORTH &= ~(1 << 4);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE(PH4) to HIGH
  PORTH |= (1 << 4);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t");
}

byte readByte_Flash(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting OE(PH1) LOW
  PORTH &= ~(1 << 1);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Setting OE(PH1) HIGH
  PORTH |= (1 << 1);
  __asm__("nop\n\t");

  return tempByte;
}

/******************************************
  29F032 flashrom functions
*****************************************/
void resetFlash29F032() {
  // Set data pins to output
  dataOut();

  // Reset command sequence
  writeByte_Flash(0x555, 0xf0);

  // Set data pins to input again
  dataIn_Flash();
}

void idFlash29F032() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x90);

  // Set data pins to input again
  dataIn_Flash();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_Flash(0), readByte_Flash(1));
}

void eraseFlash29F032() {
  // Set data pins to output
  dataOut();

  // Erase command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x80);
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x10);

  // Set data pins to input again
  dataIn_Flash();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  // After a completed erase D7 will output 1
  while ((statusReg & 0x80) != 0x80) {
    // Blink led
    PORTB ^= (1 << 4);
    delay(100);
    // Update Status
    statusReg = readByte_Flash(0);
  }
}

void writeFlash29F032() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_Msg(F("Flashing file "));
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Set data pins to output
    dataOut();

    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 2048 == 0)
        PORTB ^= (1 << 4);
      for (unsigned long c = 0; c < 512; c++) {
        // Write command sequence
        writeByte_Flash(0x555, 0xaa);
        writeByte_Flash(0x2aa, 0x55);
        writeByte_Flash(0x555, 0xa0);
        // Write current byte
        writeByte_Flash(currByte + c, sdBuffer[c]);
        busyCheck29F032(sdBuffer[c]);
      }
    }
    // Set data pins to input again
    dataIn_Flash();

    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file"));
    display_Update();
  }
}

void busyCheck29F032(byte c) {
  // Set data pins to input
  dataIn_Flash();

  // Setting OE(PH1) CE(PH6)LOW
  PORTH &= ~((1 << 1) | (1 << 6));
  // Setting WE(PH4) HIGH
  PORTH |=  (1 << 4);

  //When the Embedded Program algorithm is complete, the device outputs the datum programmed to D7
  while ((PINC & 0x80) != (c & 0x80)) {}

  // Set data pins to output
  dataOut();

  // Setting OE(PH1) HIGH
  PORTH |= (1 << 1);
}
/******************************************
  29F1610 flashrom functions
*****************************************/

void resetFlash29F1610() {
  // Set data pins to output
  dataOut();

  // Reset command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0xf0);

  // Set data pins to input again
  dataIn_Flash();
}

void writeFlash29F1610() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Flashing file "));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Set data pins to output
    dataOut();

    for (unsigned long currByte = 0; currByte < flashSize; currByte += 128) {
      // Fill SDBuffer with 1 page at a time then write it repeat until all bytes are written
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 3072 == 0)
        PORTB ^= (1 << 4);

      // Write command sequence
      writeByte_Flash(0x5555 << 1, 0xaa);
      writeByte_Flash(0x2aaa << 1, 0x55);
      writeByte_Flash(0x5555 << 1, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);
      }

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck29F1610();
    }

    // Set data pins to input again
    dataIn_Flash();

    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD"));
    display_Update();
  }
}

void idFlash29F1610() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x90);

  // Set data pins to input again
  dataIn_Flash();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByte_Flash(0), readByte_Flash(2));
}

byte readStatusReg() {
  // Set data pins to output
  dataOut();

  // Status reg command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x70);

  // Set data pins to input again
  dataIn_Flash();

  // Read the status register
  byte statusReg = readByte_Flash(0);
  return statusReg;
}

void eraseFlash29F1610() {
  // Set data pins to output
  dataOut();

  // Erase command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x80);
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x10);

  // Set data pins to input again
  dataIn_Flash();
}

// Delay between write operations based on status register
void busyCheck29F1610() {
  // Set data pins to input
  dataIn_Flash();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  while ((statusReg & 0x80) != 0x80) {
    statusReg = readByte_Flash(0);
  }

  // Set data pins to output
  dataOut();
}

/******************************************
  Common flashrom functions
*****************************************/
void blankcheck_Flash() {
  println_Msg(F("Please wait..."));
  display_Update();

  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize; currByte++) {
    // Check if all bytes are 0xFF
    if (readByte_Flash(currByte) != 0xFF) {
      currByte = flashSize;
      blank = 0;
    }
  }
  if (blank) {
    println_Msg(F("Flashrom is empty"));
    display_Update();
  }
  else {
    print_Error(F("Error: Not blank"), false);
  }
}

void verifyFlash() {
  println_Msg(F("Verifying against"));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    blank = 0;
    for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
      //fill SDBuffer
      myFile.read(sdBuffer, 512);
      for (unsigned long c = 0; c < 512; c++) {
        if (readByte_Flash(currByte + c) != sdBuffer[c]) {
          blank++;
        }
      }
    }
    if (blank == 0) {
      println_Msg(F("Flashrom verified OK"));
      display_Update();
    }
    else {
      print_Msg(F("Error: "));
      print_Msg(blank);
      println_Msg(F(" bytes "));
      print_Error(F("did not verify."), false);
    }
    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD"));
    display_Update();
  }
}

void readFlash() {
  // Reset to root directory
  sd.chdir("/");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sd.mkdir("FLASH", true);
  sd.chdir("FLASH");
  sprintf(fileName, "FL%d", foldern);
  strcat(fileName, ".bin");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  display_Clear();
  print_Msg(F("Saving as "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }
  for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
    for (unsigned long c = 0; c < 512; c++) {
      sdBuffer[c] = readByte_Flash(currByte + c);
    }
    myFile.write(sdBuffer, 512);
  }

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading"));
  display_Update();
}

void printFlash(int numBytes) {
  char myBuffer[3];

  for (int currByte = 0; currByte < numBytes; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      itoa (readByte_Flash(currByte + c), myBuffer, 16);
      for (int i = 0; i < 2 - strlen(myBuffer); i++) {
        print_Msg("0");
      }
      // Now print the significant bits
      print_Msg(myBuffer);
    }
    println_Msg("");
  }
  display_Update();
}

//******************************************
// End of File
//******************************************
