//******************************************
// SNES Game Processor RAM Cassette code by LuigiBlood
// Revision 1.0.0 February 2024
//******************************************
#ifdef ENABLE_GPC

/******************************************
   Game Processor RAM Cassette
******************************************/
/******************************************
   Prototype Declarations
 *****************************************/
/* Hoping that sanni will use this progressbar function */
extern void draw_progressbar(uint32_t processedsize, uint32_t totalsize);

//void gpcMenu();
void readRAM_GPC();
//void setup_GPC();
void writeRAM_GPC(void);

/******************************************
   Variables
 *****************************************/
//No global variables

/******************************************
  Menu
*****************************************/
// GPC flash menu items
static const char gpcFlashMenuItem1[] PROGMEM = "Read RAM";
static const char gpcFlashMenuItem2[] PROGMEM = "Write RAM";
static const char* const menuOptionsGPCFlash[] PROGMEM = { gpcFlashMenuItem1, gpcFlashMenuItem2, FSTRING_RESET };


void gpcMenu() {
  // create menu with title and 3 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGPCFlash, 3);
  mainMenu = question_box(F("Game Processor RAM"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    // Read ram
    case 0:
      // Change working dir to root
      sd.chdir("/");
      readRAM_GPC();
      break;

    // Write ram
    case 1:
      // Change working dir to root
      sd.chdir("/");
      writeRAM_GPC();
      unsigned long wrErrors;
      wrErrors = verifyRAM_GPC();
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
    case 2:
      resetArduino();
      break;
  }
}

/******************************************
   Setup
 *****************************************/
void setup_GPC() {
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
void writeBank_GPC(byte myBank, word myAddress, byte myData) {
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
byte readBank_GPC(byte myBank, word myAddress) {
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
   Game Processor RAM Cassette functions
 *****************************************/
// Read RAM cassette to SD card
void readRAM_GPC() {
  // Set control
  dataIn();
  controlIn_SNES();

  // Get name, add extension and convert to char array for sd lib
  createFolderAndOpenFile("SNES", "ROM", "GPC4M", "sfc");

  // Read Banks
  for (int currBank = 0xC0; currBank < 0xC8; currBank++) {
    // Dump the bytes to SD 512B at a time
    for (long currByte = 0; currByte < 65536; currByte += 512) {
      draw_progressbar((currBank - 0xC0) * 0x10000 + currByte, 0x80000);
      for (int c = 0; c < 512; c++) {
        sdBuffer[c] = readBank_GPC(currBank, currByte + c);
      }
      myFile.write(sdBuffer, 512);
    }
  }
  draw_progressbar(0x80000, 0x80000);  //Finish drawing progress bar

  // Close the file:
  myFile.close();
  println_Msg(F("Read ram completed"));
  display_Update();
  wait();
}

void writeRAM_GPC(void) {
  //Display file Browser and wait user to select a file. Size must be 512KB.
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select SFC file"));
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  display_Clear();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    fileSize = myFile.fileSize();
    if (fileSize != 0x80000) {
      println_Msg(F("File must be 512KB"));
      display_Update();
      myFile.close();
      wait();
      return;
    }

    //Disable ram cassette write protection
    dataOut();
    controlOut_SNES();
    for (int countProtect = 0; countProtect < 15; countProtect++) {
      writeBank_GPC(0x20, 0x6000, 0x00);
    }

    //Write ram
    dataOut();
    controlOut_SNES();
    println_Msg(F("Writing ram..."));
    display_Update();
    for (int currBank = 0xC0; currBank < 0xC8; currBank++) {
      //startAddr = 0x0000
      for (long currByte = 0x0000; currByte < 0x10000; currByte += 512) {
        myFile.read(sdBuffer, 512);
        for (unsigned long c = 0; c < 512; c++) {
          //startBank = 0x10; CS low
          writeBank_GPC(currBank, currByte + c, sdBuffer[c]);
        }
      }
      draw_progressbar(((currBank - 0xC0) * 0x10000), 0x80000);
    }

    //reenable write protection
    dataIn();
    controlIn_SNES();
    byte keepByte = readBank_GPC(0x20, 0x6000);
    delay(100);
    dataOut();
    controlOut_SNES();    
    writeBank_GPC(0x20, 0x6000, keepByte);

    draw_progressbar(0x80000, 0x80000);
    delay(100);
    // Set pins to input
    dataIn();

    // Close the file:
    myFile.close();
    println_Msg("");
    println_Msg(F("RAM writing finished"));
    display_Update();
  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

// Check if the RAM was written without any error
unsigned long verifyRAM_GPC() {
  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Variable for errors
    writeErrors = 0;

    // Set control
    controlIn_SNES();

    //startBank = 0xC0; endBank = 0xC7; CS low
    for (byte currBank = 0xC0; currBank < 0xC8; currBank++) {
      //startAddr = 0x0000
      for (long currByte = 0x0000; currByte < 0x10000; currByte += 512) {
        //fill sdBuffer
        myFile.read(sdBuffer, 512);
        for (unsigned long c = 0; c < 512; c++) {
          if ((readBank_GPC(currBank, currByte + c)) != sdBuffer[c]) {
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

#endif

//******************************************
// End of File
//******************************************