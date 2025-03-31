//******************************************
// SNES Satellaview 8M Memory pack code by tamanegi_taro
// Revision 1.0.0 October 22nd 2018
// Added BSX Sram, copied from skamans enhanced sketch //sanni
//******************************************
 #if (defined(ENABLE_SV) && defined(ENABLE_SNES))

/******************************************
   Satellaview 8M Memory Pack
******************************************/
/******************************************
   Prototype Declarations
 *****************************************/
/* Hoping that sanni will use this progressbar function */
extern void draw_progressbar(uint32_t processedsize, uint32_t totalsize);

//void svMenu();
void readROM_SV();
//void setup_SV();
void writeROM_SV(void);
void eraseCheck_SV(void);
void supplyCheck_SV(void);
void writeCheck_SV(void);
void detectCheck_SV(void);
void eraseAll_SV(void);

/******************************************
   Variables
 *****************************************/
//No global variables

/******************************************
  Menu
*****************************************/
// SV flash menu items
static const char svFlashMenuItem1[] PROGMEM = "Read Memory Pack";
static const char svFlashMenuItem2[] PROGMEM = "Write Memory Pack";
static const char svFlashMenuItem3[] PROGMEM = "Read BS-X Sram";
static const char svFlashMenuItem4[] PROGMEM = "Write BS-X Sram";
static const char* const menuOptionsSVFlash[] PROGMEM = { svFlashMenuItem1, svFlashMenuItem2, svFlashMenuItem3, svFlashMenuItem4, FSTRING_RESET };


void svMenu() {
  // create menu with title and 3 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSVFlash, 5);
  mainMenu = question_box(F("Satellaview 8M Memory"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    // Read memory pack
    case 0:
      // Change working dir to root
      sd.chdir("/");
      readROM_SV();
      break;

    // Write memory pack
    case 1:
      // Change working dir to root
      sd.chdir("/");
      writeROM_SV();
      break;

    // Read BS-X Sram
    case 2:
      // Change working dir to root
      sd.chdir("/");
      readSRAM_SV();
      break;

    // Write BS-X Sram
    case 3:
      // Change working dir to root
      sd.chdir("/");
      writeSRAM_SV();
      unsigned long wrErrors;
      wrErrors = verifySRAM_SV();
      if (wrErrors == 0) {
        println_Msg(F("Verified OK"));
        display_Update();
      } else {
        print_STR(error_STR, 0);
        print_Msg(wrErrors);
        print_STR(_bytes_STR, 1);
        print_Error(did_not_verify_STR);
      }
      wait();
      break;

    // Reset
    case 4:
      resetArduino();
      break;
  }
}

/******************************************
   Setup
 *****************************************/
void setup_SV() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal until we're ready to start
  PORTG |= (1 << 1);
  // Set cichstPin(PG0) to Input
  DDRG &= ~(1 << 0);

  // Adafruit Clock Generator
  i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if (i2c_found) {
    clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
    clockgen.set_freq(2147727200ULL, SI5351_CLK0);
    clockgen.set_freq(307200000ULL, SI5351_CLK2);
    clockgen.output_enable(SI5351_CLK0, 1);
    clockgen.output_enable(SI5351_CLK1, 0);
    clockgen.output_enable(SI5351_CLK2, 1);
  }
#ifdef ENABLE_CLOCKGEN
  else {
    display_Clear();
    print_FatalError(F("Clock Generator not found"));
  }
#endif

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7
  DDRL = 0xFF;
  //PA0-PA7
  DDRA = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Switch RST(PH0) and WR(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  // Set Refresh(PE5) to Output
  DDRE |= (1 << 5);
  // Switch Refresh(PE5) to LOW (needed for SA-1)
  PORTE &= ~(1 << 5);

  // Set CPU Clock(PH1) to Output
  DDRH |= (1 << 1);
  //PORTH &= ~(1 << 1);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set expand(PG5) to output
  DDRG |= (1 << 5);
  // Output High
  PORTG |= (1 << 5);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  //PORTC = 0xFF;

  // Unused pins
  // Set wram(PE4) to Output
  DDRE |= (1 << 4);
  //PORTE &= ~(1 << 4);
  // Set pawr(PJ1) to Output
  DDRJ |= (1 << 1);
  //PORTJ &= ~(1 << 1);
  // Set pard(PJ0) to Output
  DDRJ |= (1 << 0);
  //PORTJ &= ~(1 << 0);

  // Start CIC by outputting a low signal to cicrstPin(PG1)
  PORTG &= ~(1 << 1);

  // Wait for CIC reset
  delay(1000);
}

/******************************************
   Low level functions
 *****************************************/
// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank_SV(byte myBank, word myAddress, byte myData) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 60ns
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
          "nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
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
          "nop\n\t");
}

// Read one byte of data from a location specified by bank and address, 00:0000
byte readBank_SV(byte myBank, word myAddress) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
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
          "nop\n\t");

  // Read
  byte tempByte = PINC;
  return tempByte;
}

/******************************************
   SatellaView BS-X Sram functions
 *****************************************/
void readSRAM_SV() {
  // set control
  controlIn_SNES();

  // Get name, add extension and convert to char array for sd lib
  createFolder("SNES", "SAVE", "BSX", "srm");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  readBank_SV(0x10, 0);  // Preconfigure to fix corrupt 1st byte

  //startBank = 0x10; endBank = 0x17; CS low
  for (byte BSBank = 0x10; BSBank < 0x18; BSBank++) {
    //startAddr = 0x5000
    for (long currByte = 0x5000; currByte < 0x6000; currByte += 512) {
      for (unsigned long c = 0; c < 512; c++) {
        sdBuffer[c] = readBank_SV(BSBank, currByte + c);
      }
      myFile.write(sdBuffer, 512);
    }
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  display_Clear();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();
  wait();
}

void writeSRAM_SV() {
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select srm file"));
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  //clear the screen
  display_Clear();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Set pins to output
    dataOut();

    // Set RST RD WR to High and CS to Low
    controlOut_SNES();

    println_Msg(F("Writing sram..."));
    display_Update();

    // Write to sram bank
    for (byte currBank = 0x10; currBank < 0x18; currBank++) {
      //startAddr = 0x5000
      for (long currByte = 0x5000; currByte < 0x6000; currByte += 512) {
        myFile.read(sdBuffer, 512);
        for (unsigned long c = 0; c < 512; c++) {
          //startBank = 0x10; CS low
          writeBank_SV(currBank, currByte + c, sdBuffer[c]);
        }
      }
      draw_progressbar(((currBank - 0x10) * 0x1000), 32768);
    }
    // Finish progressbar
    draw_progressbar(32768, 32768);
    delay(100);
    // Set pins to input
    dataIn();

    // Close the file:
    myFile.close();
    println_Msg("");
    println_Msg(F("SRAM writing finished"));
    display_Update();
  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

// Check if the SRAM was written without any error
unsigned long verifySRAM_SV() {
  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Variable for errors
    writeErrors = 0;

    // Set control
    controlIn_SNES();

    //startBank = 0x10; endBank = 0x17; CS low
    for (byte BSBank = 0x10; BSBank < 0x18; BSBank++) {
      //startAddr = 0x5000
      for (long currByte = 0x5000; currByte < 0x6000; currByte += 512) {
        //fill sdBuffer
        myFile.read(sdBuffer, 512);
        for (unsigned long c = 0; c < 512; c++) {
          if ((readBank_SV(BSBank, currByte + c)) != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  } else {
    print_Error(open_file_STR);
    return 1;
  }
}

/******************************************
   SatellaView 8M Memory Pack functions
 *****************************************/
// Read memory pack to SD card
void readROM_SV() {
  // Set control
  dataIn();
  controlIn_SNES();

  // Get name, add extension and convert to char array for sd lib
  createFolderAndOpenFile("SNES", "ROM", "MEMPACK", "bs");

  // Read Banks
  for (int currBank = 0x40; currBank < 0x50; currBank++) {
    // Dump the bytes to SD 512B at a time
    for (long currByte = 0; currByte < 65536; currByte += 512) {
      draw_progressbar((currBank - 0x40) * 0x10000 + currByte, 0x100000);
      for (int c = 0; c < 512; c++) {
        sdBuffer[c] = readBank_SV(currBank, currByte + c);
      }
      myFile.write(sdBuffer, 512);
    }
  }
  draw_progressbar(0x100000, 0x100000);  //Finish drawing progress bar

  // Close the file:
  myFile.close();
  println_Msg(F("Read pack completed"));
  display_Update();
  wait();
}

void writeROM_SV(void) {
  // Get Checksum as string to make sure that BS-X cart is inserted
  dataIn();
  controlIn_SNES();
  sprintf(checksumStr, "%02X%02X", readBank_SV(0, 65503), readBank_SV(0, 65502));

  //if CRC is not 8B86, BS-X cart is not inserted. Display error and reset
  if (strcmp("8B86", checksumStr) != 0) {
    display_Clear();
    print_FatalError(F("Error: Must use BS-X cart"));
  }

  //Display file Browser and wait user to select a file. Size must be 1MB.
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select BS file"));
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  display_Clear();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    fileSize = myFile.fileSize();
    if (fileSize != 0x100000) {
      println_Msg(F("File must be 1MB"));
      display_Update();
      myFile.close();
      wait();
      return;
    }

    //Disable 8M memory pack write protection
    dataOut();
    controlOut_SNES();
    writeBank_SV(0x0C, 0x5000, 0x80);  //Modify write enable register
    writeBank_SV(0x0E, 0x5000, 0x80);  //Commit register modification

    //Erase memory pack
    println_Msg(F("Erasing pack..."));
    display_Update();
    eraseAll_SV();

    //Blank check
    //Set pins to input
    dataIn();
    controlIn_SNES();
    println_Msg(F("Blank check..."));
    display_Update();
    for (int currBank = 0xC0; currBank < 0xD0; currBank++) {
      draw_progressbar(((currBank - 0xC0) * 0x10000), 0x100000);
      for (long currByte = 0; currByte < 65536; currByte++) {
        if (0xFF != readBank_SV(currBank, currByte)) {
          println_Msg(FS(FSTRING_EMPTY));
          println_Msg(F("Erase failed"));
          display_Update();
          myFile.close();
          wait();
          return;
        }
      }
    }
    draw_progressbar(0x100000, 0x100000);

    //Write memory pack
    dataOut();
    controlOut_SNES();
    println_Msg(F("Writing pack..."));
    display_Update();
    for (int currBank = 0xC0; currBank < 0xD0; currBank++) {
      draw_progressbar(((currBank - 0xC0) * 0x10000), 0x100000);
      for (long currByte = 0; currByte < 65536; currByte++) {

        writeBank_SV(0xC0, 0x0000, 0x10);  //Program Byte
        writeBank_SV(currBank, currByte, myFile.read());
        writeBank_SV(0xC0, 0x0000, 0x70);  //Status Mode
        writeCheck_SV();
      }
    }

    writeBank_SV(0xC0, 0x0000, 0x70);  //Status Mode
    writeCheck_SV();
    writeBank_SV(0xC0, 0x0000, 0xFF);  //Terminate write
    draw_progressbar(0x100000, 0x100000);


    //Verify
    dataIn();  //Set pins to input
    controlIn_SNES();
    myFile.seekSet(0);  // Go back to file beginning
    print_STR(verifying_STR, 1);
    display_Update();
    for (int currBank = 0xC0; currBank < 0xD0; currBank++) {
      draw_progressbar(((currBank - 0xC0) * 0x10000), 0x100000);
      for (long currByte = 0; currByte < 65536; currByte++) {
        if (myFile.read() != readBank_SV(currBank, currByte)) {
          println_Msg(FS(FSTRING_EMPTY));
          println_Msg(F("Verify failed"));
          display_Update();
          myFile.close();
          wait();
          return;
        }
      }
    }

    // Close the file:
    myFile.close();
    draw_progressbar(0x100000, 0x100000);
    println_Msg(F("Finished successfully"));
    display_Update();
    wait();

  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

void eraseCheck_SV(void) {
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0004);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x80) == 0x00) {  //Wait until X.bit7 = 1
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0004);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}

void supplyCheck_SV(void) {
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0004);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x08) == 0x08) {  //Wait until X.bit3 = 0
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0004);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}

void writeCheck_SV(void) {
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0000);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x80) == 0x00) {
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0000);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}


void detectCheck_SV(void) {
  int i = 0;
  byte ret;
  dataIn();
  controlIn_SNES();

  // Read register
  ret = readBank_SV(0xC0, 0x0002);

  // CE or OE must be toggled with each subsequent status read or the
  // completion of a program or erase operation will not be evident.
  while ((ret & 0x80) == 0x00) {
    i++;
    if (i > 10000) {
      //timeout
      break;
    }
    controlOut_SNES();
    // Switch CS(PH3) High
    PORTH |= (1 << 3);
    // Leave CE high for at least 60ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    controlIn_SNES();
    // Leave CE low for at least 50ns
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
    // Read register
    ret = readBank_SV(0xC0, 0x0002);
  }
  // Switch to write
  dataOut();
  controlOut_SNES();
}


void eraseAll_SV(void) {
  dataOut();
  controlOut_SNES();
  writeBank_SV(0xC0, 0x0000, 0x50);  //Clear Status Registers
  writeBank_SV(0xC0, 0x0000, 0x71);  //Status Mode
  supplyCheck_SV();
  writeBank_SV(0xC0, 0x0000, 0xA7);  //Chip Erase
  writeBank_SV(0xC0, 0x0000, 0xD0);  //Confirm
  writeBank_SV(0xC0, 0x0000, 0x71);  //Status Mode
  eraseCheck_SV();
  writeBank_SV(0xC0, 0x0000, 0xFF);  //Teriminate
}

#endif

//******************************************
// End of File
//******************************************