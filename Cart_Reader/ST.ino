/******************************************
  SUPER FAMICOM SUFAMI TURBO MODULE
******************************************/
#if (defined(ENABLE_ST) && defined(ENABLE_SNES))

/******************************************
  Menu
*****************************************/
// Sufami Turbo menu items
static const char stMenuItem1[] PROGMEM = "Read cart in Slot A";
static const char stMenuItem2[] PROGMEM = "Read cart in Slot B";
static const char* const menuOptionsST[] PROGMEM = { stMenuItem1, stMenuItem2, FSTRING_RESET };

void stMenu() {
  // Create ST menu with title and 3 options to choose from
  unsigned char mainMenu;
  convertPgm(menuOptionsST, 3);
  mainMenu = question_box(F("Sufami Turbo"), menuOptions, 3, 0);

  // Wait for user choice to come back from the question box menu
  switch (mainMenu) {
    // Read cart in ST slot A
    case 0:
      readSlot(0);
      break;

    // Read cart in ST slot B
    case 1:
      readSlot(1);
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
void setup_ST() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal until we're ready to start
  PORTG |= (1 << 1);
  // Set cichstPin(PG0) to Input
  DDRG &= ~(1 << 0);

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
  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Set expand(PG5) to Input
  DDRG &= ~(1 << 5);
  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Unused pins
  // Set wram(PE4) to Output
  DDRE |= (1 << 4);
  // Set pawr(PJ1) to Output
  DDRJ |= (1 << 1);
  // Set pard(PJ0) to Output
  DDRJ |= (1 << 0);

  //Check if Sufami Turbo adapter is inserted
  if (!getHeader(0)) {
    display_Clear();
    println_Msg(F("Sufami Turbo adapter"));
    println_Msg(F("was not found."));
    display_Update();
    wait();
    resetArduino();
  }
}

/******************************************
  ROM Functions
******************************************/
// Verify if 'BANDAI' header is present in specified bank
bool getHeader(unsigned int bank) {
  byte snesHeader[16];
  PORTL = bank;

  // Read first bytes
  for (uint16_t c = 0, currByte = 0; c < 16; c++, currByte++) {
    PORTF = (currByte & 0xFF);
    PORTK = ((currByte >> 8) & 0xFF);
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    snesHeader[c] = PINC;
  }

  // Format internal name
  buildRomName(romName, &snesHeader[0], 14);

  // Check if 'BANDAI' header is present
  if (strncmp(romName, "BANDAI SFC-ADX", 14) == 0)
    return(1);
  else
    return(0);
} 

// Select the slot to use and detect cart size
void readSlot(bool cartSlot) {
  // Set control
  dataIn();
  controlIn_SNES();
  sd.chdir("/");
  display_Clear();

  if(!cartSlot) {            // Slot A was selected
    if (getHeader(32)) {     // Look for a cart in slot A
      if (getHeader(48))     // Look for mirrored data in slot A
        readRom_ST(32,48);   // Dump 512KB cart
      else                   
        readRom_ST(32,64);   // Dump 1MB cart
    }
    else {
      println_Msg(F("No cart detected in Slot A"));
      display_Update();
      wait();
    }
  }
  else {                      // Slot B was selected
    if (getHeader(64)) {      // Look for a cart in slot B
      if (getHeader(80))      // Look for mirrored data in slot B
        readRom_ST(64,80);    // Dump 512KB cart
      else
        readRom_ST(64,96);    // Dump 1MB cart
    }
    else {
      println_Msg(F("No cart detected in Slot B"));
      display_Update();
      wait();
    }
  }
}

// Read ST rom to SD card
void readRom_ST(unsigned int bankStart, unsigned int bankEnd) {
  // create a new folder to save rom file
  createFolderAndOpenFile("ST", "ROM", "SUFAMI_TURBO", "st");

  // Read specified banks
  readLoRomBanks(bankStart + 0x80, bankEnd + 0x80, &myFile);

  // Close file:
  myFile.close();

  // Compare dump CRC with db values
  compareCRC("st.txt", 0, 1, 0);
  
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}
#endif

//******************************************
// End of File
//******************************************
