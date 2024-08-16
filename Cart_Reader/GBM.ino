//******************************************
// GB MEMORY MODULE
//******************************************
#ifdef ENABLE_GBX

/******************************************
  Menu
*****************************************/
// GBM menu items
static const char gbmMenuItem1[] PROGMEM = "Read ID";
static const char gbmMenuItem2[] PROGMEM = "Read Flash";
static const char gbmMenuItem3[] PROGMEM = "Erase Flash";
static const char gbmMenuItem4[] PROGMEM = "Blankcheck";
static const char gbmMenuItem5[] PROGMEM = "Write Flash";
static const char gbmMenuItem6[] PROGMEM = "Read Mapping";
static const char gbmMenuItem7[] PROGMEM = "Write Mapping";
static const char* const menuOptionsGBM[] PROGMEM = { gbmMenuItem1, gbmMenuItem2, gbmMenuItem3, gbmMenuItem4, gbmMenuItem5, gbmMenuItem6, gbmMenuItem7 };

void gbmMenu() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGBM, 7);
  mainMenu = question_box(F("GB Memory Menu"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
#if defined(ENABLE_FLASH)
    // Read Flash ID
    case 0:
      // Clear screen
      display_Clear();
      readFlashID_GBM();
      break;
#endif

    // Read Flash
    case 1:
      // Clear screen
      display_Clear();
      // Print warning
      println_Msg(F("Attention"));
      println_Msg(F("Always power cycle"));
      println_Msg(F("cartreader directly"));
      println_Msg(F("before reading"));
      println_Msg("");
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Enable access to ports 0120h
      send_GBM(0x09);
      // Map entire flashrom
      send_GBM(0x04);
      // Disable ports 0x0120...
      send_GBM(0x08);
      // Read 1MB rom
      readROM_GBM(64);
      break;

#if defined(ENABLE_FLASH)
    // Erase Flash
    case 2:
      // Clear screen
      display_Clear();
      // Print warning
      println_Msg(F("Attention"));
      println_Msg(F("This will erase your"));
      println_Msg(F("NP Cartridge."));
      println_Msg("");
      println_Msg("");
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      // Clear screen
      display_Clear();
      eraseFlash_GBM();
      break;

    // Blankcheck Flash
    case 3:
      // Clear screen
      display_Clear();
      if (blankcheckFlash_GBM()) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
      } else {
        println_Msg(F("ERROR"));
        display_Update();
      }
      break;

    // Write Flash
    case 4:
      // Clear screen
      display_Clear();

      filePath[0] = '\0';
      sd.chdir("/");
      // Launch file browser
      fileBrowser(F("Select 1MB file"));
      display_Clear();
      sprintf(filePath, "%s/%s", filePath, fileName);

      // Write rom
      writeFlash_GBM();
      break;

    // Read mapping
    case 5:
      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Read mapping
      readMapping_GBM();
      break;

    // Write mapping
    case 6:
      // Clear screen
      display_Clear();

      // Print warning
      println_Msg(F("Attention"));
      println_Msg(F("This will erase your"));
      println_Msg(F("NP Cartridge's"));
      println_Msg(F("mapping data"));
      println_Msg("");
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();

      // Reset to root directory
      sd.chdir("/");

      // Clear screen
      display_Clear();

      // Clear filepath
      filePath[0] = '\0';

      // Reset to root directory
      sd.chdir("/");

      // Launch file browser
      fileBrowser(F("Select MAP file"));
      display_Clear();
      sprintf(filePath, "%s/%s", filePath, fileName);
      display_Update();

      // Clear screen
      display_Clear();

      // Erase mapping
      eraseMapping_GBM();
      if (blankcheckMapping_GBM()) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
      } else {
        print_Error(F("Erasing failed"));
        break;
      }

      // Write mapping
      writeMapping_GBM();
      break;
#endif

    default:
      print_MissingModule();  // does not return
  }
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

/******************************************
  Setup
*****************************************/
void setup_GBM() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set RST(PH0) to Input
  DDRH &= ~(1 << 0);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  delay(400);

  // Check for Nintendo Power GB Memory cart
  byte timeout = 0;

  // First byte of NP register is always 0x21
  while (readByte_GBM(0x120) != 0x21) {
    // Enable ports 0x120h (F2)
    send_GBM(0x09);
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    timeout++;
    if (timeout > 10) {
      println_Msg(F("Error: Time Out"));
      print_FatalError(F("Please power cycle"));
    }
  }
}

/**********************
  LOW LEVEL
**********************/
// Read one word out of the cartridge
byte readByte_GBM(word myAddress) {
  // Set data pins to Input
  DDRC = 0x0;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~(1 << 3);
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch CS(PH3) and RD(PH6) to HIGH
  PORTH |= (1 << 6);
  PORTH |= (1 << 3);

  return tempByte;
}

// Write one word to data pins of the cartridge
void writeByte_GBM(word myAddress, byte myData) {
  // Set data pins to Output
  DDRC = 0xFF;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Pull CS(PH3) and write(PH5) low
  PORTH &= ~(1 << 3);
  PORTH &= ~(1 << 5);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Pull CS(PH3) and write(PH5) high
  PORTH |= (1 << 5);
  PORTH |= (1 << 3);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Set data pins to Input (or read errors??!)
  DDRC = 0x0;
}

/**********************
  HELPER FUNCTIONS
**********************/
void printSdBuffer(word startByte, word numBytes) {
  for (word currByte = 0; currByte < numBytes; currByte += 10) {
    for (byte c = 0; c < 10; c++) {
      // Convert to char array so we don't lose leading zeros
      char currByteStr[2];
      sprintf(currByteStr, "%02X", sdBuffer[startByte + currByte + c]);
      print_Msg(currByteStr);
    }
    // Add a new line every 10 bytes
    println_Msg("");
  }
  display_Update();
}

void readROM_GBM(word numBanks) {
  println_Msg(F("Reading Rom..."));
  display_Update();

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "GBM%d", foldern);
  strcat(fileName, ".bin");
  sd.mkdir("NP", true);
  sd.chdir("NP");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  } else {
    // Read rom
    word currAddress = 0;

    for (word currBank = 1; currBank < numBanks; currBank++) {
      // Set rom bank
      writeByte_GBM(0x2100, currBank);

      // Switch bank start address
      if (currBank > 1) {
        currAddress = 0x4000;
      }

      for (; currAddress < 0x7FFF; currAddress += 512) {
        for (int currByte = 0; currByte < 512; currByte++) {
          sdBuffer[currByte] = readByte_GBM(currAddress + currByte);
        }
        myFile.write(sdBuffer, 512);
      }
    }

    // Close the file:
    myFile.close();

    // Signal end of process
    print_Msg(F("Saved to NP/"));
    println_Msg(fileName);
    display_Update();
  }
}

/**********************
  GB Memory Functions
**********************/
void send_GBM(byte myCommand) {
  switch (myCommand) {
    case 0x01:
      //CMD_01h -> ???
      writeByte_GBM(0x0120, 0x01);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x02:
      //CMD_02h -> Write enable Step 2
      writeByte_GBM(0x0120, 0x02);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x03:
      //CMD_03h -> Undo write Step 2
      writeByte_GBM(0x0120, 0x03);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x04:
      //CMD_04h -> Map entire flashrom (MBC4 mode)
      writeByte_GBM(0x0120, 0x04);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x05:
      //CMD_05h -> Map menu (MBC5 mode)
      writeByte_GBM(0x0120, 0x05);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x08:
      //CMD_08h -> disable writes/reads to/from special Nintendo Power registers (those at 0120h..013Fh)
      writeByte_GBM(0x0120, 0x08);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x09:
      //CMD_09h Wakeup -> re-enable access to ports 0120h..013Fh
      writeByte_GBM(0x0120, 0x09);
      writeByte_GBM(0x0121, 0xAA);
      writeByte_GBM(0x0122, 0x55);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x0A:
      //CMD_0Ah -> Write enable Step 1
      writeByte_GBM(0x0120, 0x0A);
      writeByte_GBM(0x0125, 0x62);
      writeByte_GBM(0x0126, 0x04);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x10:
      //CMD_10h -> disable writes to normal MBC registers (such like 2100h)
      writeByte_GBM(0x0120, 0x10);
      writeByte_GBM(0x013F, 0xA5);
      break;

    case 0x11:
      //CMD_11h -> re-enable access to MBC registers like 0x2100
      writeByte_GBM(0x0120, 0x11);
      writeByte_GBM(0x013F, 0xA5);
      break;

    default:
      print_FatalError(F("Unknown Command"));
      break;
  }
}

#if defined(ENABLE_FLASH)
void send_GBM(byte myCommand, word myAddress, byte myData) {
  byte myAddrLow = myAddress & 0xFF;
  byte myAddrHigh = (myAddress >> 8) & 0xFF;

  switch (myCommand) {
    case 0x0F:
      // CMD_0Fh -> Write address/byte to flash
      writeByte_GBM(0x0120, 0x0F);
      writeByte_GBM(0x0125, myAddrHigh);
      writeByte_GBM(0x0126, myAddrLow);
      writeByte_GBM(0x0127, myData);
      writeByte_GBM(0x013F, 0xA5);
      break;

    default:
      print_FatalError(F("Unknown Command"));
      break;
  }
}

void switchGame_GBM(byte myData) {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  //CMD_C0h -> map selected game without reset
  writeByte_GBM(0x0120, 0xC0 & myData);
  writeByte_GBM(0x013F, 0xA5);
}

void resetFlash_GBM() {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  // Send reset command
  writeByte_GBM(0x2100, 0x01);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0xF0);
  delay(100);
}

boolean readFlashID_GBM() {
  // Enable ports 0x0120 (F2)
  send_GBM(0x09);

  writeByte_GBM(0x2100, 0x01);
  // Read ID command
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x90);

  // Read the two id bytes into a string
  flashid = readByte_GBM(0) << 8;
  flashid |= readByte_GBM(1);
  sprintf(flashid_str, "%04X", flashid);
  if (flashid == 0xC289) {
    print_Msg(F("Flash ID: "));
    println_Msg(flashid_str);
    display_Update();
    resetFlash_GBM();
    return 1;
  } else {
    print_Msg(F("Flash ID: "));
    println_Msg(flashid_str);
    print_FatalError(F("Unknown Flash ID"));
    resetFlash_GBM();
    return 0;
  }
}

void eraseFlash_GBM() {
  println_Msg(F("Erasing..."));
  display_Update();

  //enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Unprotect sector 0
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x60);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x40);

  // Wait for unprotect to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Send erase command
  send_GBM(0x0F, 0x5555, 0xaa);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x80);
  send_GBM(0x0F, 0x5555, 0xaa);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x10);

  // Wait for erase to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Reset flashrom
  resetFlash_GBM();
}

boolean blankcheckFlash_GBM() {
  print_Msg(F("Blankcheck..."));
  display_Update();

  //enable access to ports 0120h (F2)
  send_GBM(0x09);

  // Map entire flashrom
  send_GBM(0x04);
  // Disable ports 0x0120...
  send_GBM(0x08);

  // Read rom
  word currAddress = 0;

  for (byte currBank = 1; currBank < 64; currBank++) {
    // Set rom bank
    writeByte_GBM(0x2100, currBank);

    // Switch bank start address
    if (currBank > 1) {
      currAddress = 0x4000;
    }

    for (; currAddress < 0x7FFF; currAddress++) {
      if (readByte_GBM(currAddress) != 0xFF) {
        return 0;
      }
    }
  }
  return 1;
}

void writeFlash_GBM() {
  print_Msg(F("Writing..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if ((fileSize / 0x4000) > 64) {
      print_FatalError(F("File is too big."));
    }

    // Enable access to ports 0120h
    send_GBM(0x09);
    // Enable write
    send_GBM(0x0A);
    send_GBM(0x2);

    // Map entire flash rom
    send_GBM(0x4);

    // Set bank for unprotect command, writes to 0x5555 need odd bank number
    writeByte_GBM(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Unprotect sector 0
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0x60);
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0x40);

    // Check if flashrom is ready for writing or busy
    while ((readByte_GBM(0) & 0x80) != 0x80) {}

    // first bank: 0x0000-0x7FFF,
    word currAddress = 0x0;

    // Write 63 banks
    for (byte currBank = 0x1; currBank < (fileSize / 0x4000); currBank++) {
      // Blink led
      blinkLED();

      // all following banks: 0x4000-0x7FFF
      if (currBank > 1) {
        currAddress = 0x4000;
      }

      // Write single bank in 128 byte steps
      for (; currAddress < 0x7FFF; currAddress += 128) {
        // Fill SD buffer
        myFile.read(sdBuffer, 128);

        // Enable access to ports 0x120 and 0x2100
        send_GBM(0x09);
        send_GBM(0x11);

        // Set bank
        writeByte_GBM(0x2100, 0x1);

        // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
        send_GBM(0x10);
        send_GBM(0x08);

        // Write flash buffer command
        writeByte_GBM(0x5555, 0xAA);
        writeByte_GBM(0x2AAA, 0x55);
        writeByte_GBM(0x5555, 0xA0);

        // Wait until flashrom is ready again
        while ((readByte_GBM(0) & 0x80) != 0x80) {}

        // Enable access to ports 0x120 and 0x2100
        send_GBM(0x09);
        send_GBM(0x11);

        // Set bank
        writeByte_GBM(0x2100, currBank);

        // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
        send_GBM(0x10);
        send_GBM(0x08);

        // Fill flash buffer
        for (word currByte = 0; currByte < 128; currByte++) {
          writeByte_GBM(currAddress + currByte, sdBuffer[currByte]);
        }
        // Execute write
        writeByte_GBM(currAddress + 127, 0xFF);

        // Wait for write to complete
        while ((readByte_GBM(currAddress) & 0x80) != 0x80) {}
      }
    }
    // Close the file:
    myFile.close();
    print_STR(done_STR, 1);
  } else {
    print_Error(open_file_STR);
  }
}

void readMapping_GBM() {
  // Enable ports 0x0120
  send_GBM(0x09);

  // Set WE and WP
  send_GBM(0x0A);
  send_GBM(0x2);

  // Enable hidden mapping area
  writeByte_GBM(0x2100, 0x01);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);

  // Read mapping
  println_Msg(F("Reading Mapping..."));
  display_Update();

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "GBM%d", foldern);
  strcat(fileName, ".map");
  sd.mkdir("NP", true);
  sd.chdir("NP");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  } else {
    for (byte currByte = 0; currByte < 128; currByte++) {
      sdBuffer[currByte] = readByte_GBM(currByte);
    }
    myFile.write(sdBuffer, 128);

    // Close the file:
    myFile.close();

    // Signal end of process
    printSdBuffer(0, 20);
    printSdBuffer(102, 20);
    println_Msg("");
    print_Msg(F("Saved to NP/"));
    println_Msg(fileName);
    display_Update();
  }

  // Reset flash to leave hidden mapping area
  resetFlash_GBM();
}

void eraseMapping_GBM() {
  println_Msg(F("Erasing..."));
  display_Update();

  //enable access to ports 0120h
  send_GBM(0x09);
  // Enable write
  send_GBM(0x0A);
  send_GBM(0x2);

  // Unprotect sector 0
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x60);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x40);

  // Wait for unprotect to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Send erase command
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x60);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x04);

  // Wait for erase to complete
  while ((readByte_GBM(0) & 0x80) != 0x80) {}

  // Reset flashrom
  resetFlash_GBM();
}

boolean blankcheckMapping_GBM() {
  print_Msg(F("Blankcheck..."));
  display_Update();

  // Enable ports 0x0120
  send_GBM(0x09);

  // Set WE and WP
  send_GBM(0x0A);
  send_GBM(0x2);

  // Enable hidden mapping area
  writeByte_GBM(0x2100, 0x01);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);
  send_GBM(0x0F, 0x5555, 0xAA);
  send_GBM(0x0F, 0x2AAA, 0x55);
  send_GBM(0x0F, 0x5555, 0x77);

  // Disable ports 0x0120...
  send_GBM(0x08);

  // Read rom
  for (byte currByte = 0; currByte < 128; currByte++) {
    if (readByte_GBM(currByte) != 0xFF) {
      return 0;
    }
  }
  return 1;
}

void writeMapping_GBM() {
  print_Msg(F("Writing..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get map file size and check if it exceeds 128KByte
    if (myFile.fileSize() > 0x80) {
      print_FatalError(F("File is too big."));
    }

    // Enable access to ports 0120h
    send_GBM(0x09);

    // Enable write
    send_GBM(0x0A);
    send_GBM(0x2);

    // Map entire flash rom
    send_GBM(0x4);

    // Set bank, writes to 0x5555 need odd bank number
    writeByte_GBM(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Unlock write to map area
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0x60);
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0xE0);

    // Check if flashrom is ready for writing or busy
    while ((readByte_GBM(0) & 0x80) != 0x80) {}

    // Fill SD buffer
    myFile.read(sdBuffer, 128);

    // Enable access to ports 0x120 and 0x2100
    send_GBM(0x09);
    send_GBM(0x11);

    // Set bank
    writeByte_GBM(0x2100, 0x1);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Write flash buffer command
    writeByte_GBM(0x5555, 0xAA);
    writeByte_GBM(0x2AAA, 0x55);
    writeByte_GBM(0x5555, 0xA0);

    // Wait until flashrom is ready again
    while ((readByte_GBM(0) & 0x80) != 0x80) {}

    // Enable access to ports 0x120 and 0x2100
    send_GBM(0x09);
    send_GBM(0x11);

    // Set bank
    writeByte_GBM(0x2100, 0);

    // Disable ports 0x2100 and 0x120 or else those addresses will not be writable
    send_GBM(0x10);
    send_GBM(0x08);

    // Fill flash buffer
    for (word currByte = 0; currByte < 128; currByte++) {
      // Blink led
      blinkLED();

      writeByte_GBM(currByte, sdBuffer[currByte]);
    }
    // Execute write
    writeByte_GBM(127, 0xFF);

    // Close the file:
    myFile.close();
    print_STR(done_STR, 1);
  } else {
    print_Error(open_file_STR);
  }
}
#endif
#endif

//******************************************
// End of File
//******************************************