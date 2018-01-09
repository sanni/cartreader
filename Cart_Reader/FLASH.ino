//******************************************
// FLASHROM
//******************************************

/******************************************
   Variables
 *****************************************/
// Flashrom
unsigned long flashSize;
byte flashromType;
byte secondID = 1;
unsigned long time;
unsigned long blank;

/******************************************
   Menu
 *****************************************/
// Flash start menu
static const char flashMenuItem1[] PROGMEM = "8bit slot";
static const char flashMenuItem2[] PROGMEM = "16bit slot";
static const char* const menuOptionsFlash[] PROGMEM = {flashMenuItem1, flashMenuItem2};

// 8bit Flash menu items
static const char flash8MenuItem1[] PROGMEM = "Blankcheck";
static const char flash8MenuItem2[] PROGMEM = "Erase";
static const char flash8MenuItem3[] PROGMEM = "Read";
static const char flash8MenuItem4[] PROGMEM = "Write";
static const char flash8MenuItem5[] PROGMEM = "ID";
static const char flash8MenuItem6[] PROGMEM = "Print";
static const char flash8MenuItem7[] PROGMEM = "Reset";
static const char* const menuOptionsFLASH8[] PROGMEM = {flash8MenuItem1, flash8MenuItem2, flash8MenuItem3, flash8MenuItem4, flash8MenuItem5, flash8MenuItem6, flash8MenuItem7};

// 16bit Flash menu items
static const char flash16MenuItem1[] PROGMEM = "Blankcheck";
static const char flash16MenuItem2[] PROGMEM = "Erase";
static const char flash16MenuItem3[] PROGMEM = "Read";
static const char flash16MenuItem4[] PROGMEM = "Write";
static const char flash16MenuItem5[] PROGMEM = "ID";
static const char flash16MenuItem6[] PROGMEM = "Print";
static const char flash16MenuItem7[] PROGMEM = "Reset";
static const char* const menuOptionsFLASH16[] PROGMEM = {flash16MenuItem1, flash16MenuItem2, flash16MenuItem3, flash16MenuItem4, flash16MenuItem5, flash16MenuItem6, flash16MenuItem7};

void flashMenu() {
  // create menu with title and 2 options to choose from
  unsigned char flashSlot;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFlash, 2);
  flashSlot = question_box("Select flashrom slot", menuOptions, 2, 0);

  // wait for user choice to come back from the question box menu
  switch (flashSlot)
  {
    case 0:
      display_Clear();
      display_Update();
      setup_Flash8();
      mode =  mode_FLASH8;
      break;

    case 1:
      display_Clear();
      display_Update();
      setup_Flash16();
      mode =  mode_FLASH16;
      break;
  }
}

void flashromMenu8() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH8, 7);
  mainMenu = question_box("Flashrom Writer 8", menuOptions, 7, 0);

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
      fileBrowser("Select file");
      display_Clear();
      time = millis();
      if (flashromType == 1)
        writeFlash29F032();
      else if (flashromType == 2) {
        if (strcmp(flashid, "C2F3") == 0)
          writeFlash29F1601();
        else if ((strcmp(flashid, "C2F1") == 0) || (strcmp(flashid, "C2F9") == 0))
          writeFlash29F1610();
        else if ((strcmp(flashid, "C2C4") == 0) || (strcmp(flashid, "C2A8") == 0) || (strcmp(flashid, "C2C9") == 0))
          writeFlash29LV640();
      }
      delay(100);
      // Reset twice just to be sure
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
      if (flashromType == 1)
        resetFlash29F032();
      else
        resetFlash29F1610();
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

void flashromMenu16() {
  // create menu with title "Flashrom Writer 16" and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH16, 7);
  mainMenu = question_box("Flashrom Writer 16", menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      resetFlash16();
      blankcheck16();
      break;

    case 1:
      display_Clear();
      println_Msg(F("Erase Flashrom"));
      display_Update();
      time = millis();
      resetFlash16();
      eraseFlash16();
      println_Msg(F("Flashrom erased."));
      display_Update();
      break;

    case 2:
      display_Clear();
      time = millis();
      resetFlash16();
      readFlash16();
      break;

    case 3:
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser("Select file");
      display_Clear();
      time = millis();
      if (strcmp(flashid, "C2F3") == 0) {
        writeFlash16_29F1601();
      }
      else {
        writeFlash16();
      }
      delay(100);
      resetFlash16();
      delay(100);
      verifyFlash16();
      break;

    case 4:
      time = 0;
      display_Clear();
      println_Msg(F("ID Flashrom"));
      idFlash16();
      println_Msg(flashid);
      display_Update();
      resetFlash16();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      resetFlash16();
      printFlash16(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      resetFlash16();
      asm volatile ("  jmp 0");
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took: "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg("s");
    display_Update();
  }
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_Flash8() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) OE(PH1) WE(PH4) CE(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 4) | (1 << 6);
  // Setting RST(PH0) OE(PH1) WE(PH4) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 4);
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
  else if (strcmp(flashid, "C2F3") == 0) {
    println_Msg(F("MX29F1601 detected"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F9") == 0) {
    println_Msg(F("MX29L3211 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2C4") == 0) {
    println_Msg(F("MX29LV160 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2A8") == 0) {
    println_Msg(F("MX29LV320"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2C9") == 0) {
    println_Msg(F("MX29LV640"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
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
  else if (strcmp(flashid, "20AD") == 0) {
    println_Msg(F("AM29F016D"));
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

void setup_Flash16() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output OE(PH1) BYTE(PH3) WE(PH4) CE(PH6)
  DDRH |=  (1 << 1) | (1 << 3) | (1 << 4) | (1 << 6);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
  PORTA = 0x00;

  // Setting OE(PH1) BYTE(PH3) WE(PH4) HIGH
  PORTH |= (1 << 1) | (1 << 3) | (1 << 4);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  delay(100);

  // ID flash
  idFlash16();
  resetFlash16();

  println_Msg(F("Flashrom Writer 16bit"));
  println_Msg(" ");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid);
  if (strcmp(flashid, "C2F1") == 0) {
    println_Msg(F("MX29F1610 detected"));
    println_Msg(" ");
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F3") == 0) {
    println_Msg(F("MX29F1601 detected"));
    flashSize = 2097152;
    flashromType = 2;
  }
  else if (strcmp(flashid, "C2F9") == 0) {
    println_Msg(F("MX29L3211 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  }
  else {
    print_Error(F("Unknown flashrom"), true);
    println_Msg(" ");
  }
  println_Msg(" ");
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   I/O Functions
 *****************************************/
// Switch data pins to read
void dataIn8() {
  // Set to Input
  DDRC = 0x00;
}

// Switch data pins to write
void dataOut16() {
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// Switch data pins to read
void dataIn16() {
  DDRC = 0x00;
  DDRA = 0x00;
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

void writeWord_Flash(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

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
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

word readWord_Flash(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting OE(PH1) LOW
  PORTH &= ~(1 << 1);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  word tempWord = ( ( PINA & 0xFF ) << 8 ) | ( PINC & 0xFF );

  __asm__("nop\n\t");

  // Setting OE(PH1) HIGH
  PORTH |= (1 << 1);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempWord;
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
  dataIn8();

  delay(500);
}

void idFlash29F032() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x90);

  // Set data pins to input again
  dataIn8();

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
  dataIn8();

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
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    // Set data pins to output
    dataOut();

    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 2048 == 0)
        PORTB ^= (1 << 4);

      for (int c = 0; c < 512; c++) {
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
    dataIn8();

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
  dataIn8();

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
  dataIn8();

  delay(500);
}

void writeFlash29F1610() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Flashing file "));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    // Set data pins to output
    dataOut();

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 128) {
      // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 3072 == 0)
        PORTB ^= (1 << 4);

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck29F1610();

      // Write command sequence
      writeByte_Flash(0x5555 << 1, 0xaa);
      writeByte_Flash(0x2aaa << 1, 0x55);
      writeByte_Flash(0x5555 << 1, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);
      }
    }

    // Check if write is complete
    busyCheck29F1610();

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD"));
    display_Update();
  }
}

void writeFlash29F1601() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Flashing file "));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    // Set data pins to output
    dataOut();

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 128) {
      // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 3072 == 0)
        PORTB ^= (1 << 4);

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck29F1610();

      // Write command sequence
      writeByte_Flash(0x5555 << 1, 0xaa);
      writeByte_Flash(0x2aaa << 1, 0x55);
      writeByte_Flash(0x5555 << 1, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);

        if (c == 127) {
          // Write the last byte twice or else it won't write at all
          writeByte_Flash(currByte + c, sdBuffer[c]);
        }
      }
    }

    // Check if write is complete
    busyCheck29F1610();

    // Set data pins to input again
    dataIn8();

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
  dataIn8();

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
  dataIn8();

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
  dataIn8();

  busyCheck29F1610();
}

// Delay between write operations based on status register
void busyCheck29F1610() {
  // Set data pins to input
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  while ((statusReg & 0x80) != 0x80) {
    statusReg = readByte_Flash(0);
  }

  // Set data pins to output
  dataOut();
}

/******************************************
  MX29LV flashrom functions
*****************************************/
void busyCheck29LV640(unsigned long myAddress, byte myData) {
  // Set data pins to input
  dataIn8();
  // Read the status register
  byte statusReg = readByte_Flash(myAddress);
  while ((statusReg & 0x80) != (myData & 0x80)) {
    statusReg = readByte_Flash(myAddress);
  }
  // Set data pins to output
  dataOut();
}

void writeFlash29LV640() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Flashing file "));
  println_Msg(filePath);
  display_Update();
  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    // Set data pins to output
    dataOut();
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      // Fill sdBuffer
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 4096 == 0)
        PORTB ^= (1 << 4);
      for (int c = 0; c < 512; c++) {
        // Write command sequence
        writeByte_Flash(0x555 << 1, 0xaa);
        writeByte_Flash(0x2aa << 1, 0x55);
        writeByte_Flash(0x555 << 1, 0xa0);
        // Write current byte
        writeByte_Flash(currByte + c, sdBuffer[c]);
        // Check if write is complete
        busyCheck29LV640(currByte + c, sdBuffer[c]);
      }
    }
    // Set data pins to input again
    dataIn8();
    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD"));
    display_Update();
  }
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
  println_Msg(F("Verifying..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    blank = 0;
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 512; c++) {
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
  EEPROM_readAnything(10, foldern);
  sd.mkdir("FLASH", true);
  sd.chdir("FLASH");
  sprintf(fileName, "FL%d", foldern);
  strcat(fileName, ".bin");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(10, foldern);

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
    for (int c = 0; c < 512; c++) {
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

/******************************************
  29L3211 16bit flashrom functions
*****************************************/
void resetFlash16() {
  // Set data pins to output
  dataOut16();

  // Reset command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0xf0);

  // Set data pins to input again
  dataIn16();

  delay(500);
}

void writeFlash16() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Flashing file "));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    // Set data pins to output
    dataOut16();

    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    int d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 2048 == 0)
        PORTB ^= (1 << 4);

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck16();

      // Write command sequence
      writeWord_Flash(0x5555, 0xaa);
      writeWord_Flash(0x2aaa, 0x55);
      writeWord_Flash(0x5555, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 64; c++) {
        word currWord = ( ( sdBuffer[d + 1] & 0xFF ) << 8 ) | ( sdBuffer[d] & 0xFF );
        writeWord_Flash(currByte + c, currWord);
        d += 2;
      }
      d = 0;
    }

    // Check if write is complete
    busyCheck16();

    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void writeFlash16_29F1601() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Flashing file "));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_Error(F("File size exceeds flash size."), true);

    // Set data pins to output
    dataOut16();

    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    int d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 2048 == 0)
        PORTB ^= (1 << 4);

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck16();

      // Write command sequence
      writeWord_Flash(0x5555, 0xaa);
      writeWord_Flash(0x2aaa, 0x55);
      writeWord_Flash(0x5555, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 64; c++) {
        word currWord = ( ( sdBuffer[d + 1] & 0xFF ) << 8 ) | ( sdBuffer[d] & 0xFF );
        writeWord_Flash(currByte + c, currWord);

        if (c == 63) {
          // Write the last byte twice or else it won't write at all
          writeWord_Flash(currByte + c, sdBuffer[d + 1]);
        }
        d += 2;
      }
      d = 0;
    }

    // Check if write is complete
    busyCheck16();

    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void idFlash16() {
  // Set data pins to output
  dataOut16();

  // ID command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x90);

  // Set data pins to input again
  dataIn16();

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readWord_Flash(0) & 0xFF, readWord_Flash(1) & 0xFF);
}

byte readStatusReg16() {
  // Set data pins to output
  dataOut16();

  // Status reg command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x70);

  // Set data pins to input again
  dataIn16();

  // Read the status register
  byte statusReg = readWord_Flash(0);
  return statusReg;
}

void eraseFlash16() {
  // Set data pins to output
  dataOut16();

  // Erase command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x80);
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x10);

  // Set data pins to input again
  dataIn16();

  busyCheck16();
}

void blankcheck16() {

  println_Msg(F("Please wait..."));
  display_Update();

  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte++) {
    if (readWord_Flash(currByte) != 0xFFFF) {
      currByte = flashSize / 2;
      blank = 0;
    }
  }
  if (blank) {
    println_Msg(F("Flashrom is empty."));
    display_Update();
  }
  else {
    print_Error(F("Error: Not blank"), false);
  }
}

void verifyFlash16() {
  println_Msg(F("Verifying..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize) {
      print_Error(F("File size exceeds flash size."), true);
    }

    blank = 0;
    word d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 256) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 256; c++) {
        word currWord = ((sdBuffer[d + 1] << 8) | sdBuffer[d]);

        if (readWord_Flash(currByte + c) != currWord) {
          blank++;
        }
        d += 2;
      }
      d = 0;
    }
    if (blank == 0) {
      println_Msg(F("Flashrom verified OK"));
      display_Update();
    }
    else {
      println_Msg(F("Verification ERROR!"));
      print_Msg(blank);
      print_Error(F("B did not verify."), false);
      display_Update();
    }
    // Close the file:
    myFile.close();
  }
  else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void readFlash16() {
  // Reset to root directory
  sd.chdir("/");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(10, foldern);
  sd.mkdir("FLASH", true);
  sd.chdir("FLASH");
  sprintf(fileName, "FL%d", foldern);
  strcat(fileName, ".bin");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(10, foldern);

  display_Clear();
  print_Msg(F("Saving as "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("Can't create file on SD."));
    display_Update();
    while (1);
  }
  word d = 0;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte += 256) {
    for (word c = 0; c < 256; c++) {
      word currWord = readWord_Flash(currByte + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = (( currWord >> 8 ) & 0xFF);
      // Left
      sdBuffer[d] = (currWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
  }

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading."));
  display_Update();
}

void printFlash16(int numBytes) {
  /*
    right_byte = short_val & 0xFF;
    left_byte = ( short_val >> 8 ) & 0xFF
    short_val = ( ( left_byte & 0xFF ) << 8 ) | ( right_byte & 0xFF );
  */

  char buf[3];

  for (int currByte = 0; currByte < numBytes / 2; currByte += 5) {
    // 5 words per line
    for (int c = 0; c < 5; c++) {
      word currWord = readWord_Flash(currByte + c);

      // Split word into two bytes
      byte left_byte = currWord & 0xFF;
      byte right_byte = ( currWord >> 8 ) & 0xFF;


      sprintf (buf, "%x", left_byte);
      for (int i = 0; i < 2 - strlen(buf); i++) {
        print_Msg("0");
      }
      // Now print the significant bits
      print_Msg(buf);

      sprintf (buf, "%x", right_byte);
      for (int i = 0; i < 2 - strlen(buf); i++) {
        print_Msg("0");
      }
      // Now print the significant bits
      print_Msg(buf);
    }
    println_Msg("");
  }
  display_Update();
}

// Delay between write operations based on status register
void busyCheck16() {
  // Set data pins to input
  dataIn16();

  // Read the status register
  word statusReg = readWord_Flash(0);

  while ((statusReg | 0xFF7F) != 0xFFFF) {
    statusReg = readWord_Flash(0);
  }

  // Set data pins to output
  dataOut16();
}

//******************************************
// End of File
//******************************************
