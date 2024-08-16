//******************************************
// SUPER NINTENDO MODULE
//******************************************
#ifdef ENABLE_SNES

/******************************************
  Defines
 *****************************************/
// SNES Hi and LoRom, SA is HI with different Sram dumping
#define EX 4
#define SA 3
#define HI 1
#define LO 0

/******************************************
   Variables
 *****************************************/
// Define SNES Cart Reader Variables
int romSpeed = 0;  // 0 = SlowROM, 3 = FastROM
int romChips = 0;  // 0 = ROM only, 1 = ROM & RAM, 2 = ROM & Save RAM,  3 = ROM & DSP1, 4 = ROM & RAM & DSP1, 5 = ROM & Save RAM & DSP1, 19 = ROM & SFX
// 227 = ROM & RAM & GameBoy data, 243 = CX4, 246 = ROM & DSP2
byte romSizeExp = 0;  // ROM-Size Exponent
boolean NP = false;
byte cx4Type = 0;
byte cx4Map = 0;

/******************************************
  Menu
*****************************************/
// SNES/Nintendo Power SF Memory start menu
static const char snsMenuItem1[] PROGMEM = "SNES/SFC cartridge";
static const char snsMenuItem2[] PROGMEM = "SF Memory Cassette";
static const char snsMenuItem3[] PROGMEM = "Satellaview BS-X";
static const char snsMenuItem4[] PROGMEM = "Sufami Turbo";
static const char snsMenuItem5[] PROGMEM = "Game Processor RAM";
static const char snsMenuItem6[] PROGMEM = "Flash repro";
#ifdef OPTION_CLOCKGEN_CALIBRATION
static const char snsMenuItem7[] PROGMEM = "Calibrate Clock";
static const char* const menuOptionsSNS[] PROGMEM = { snsMenuItem1, snsMenuItem2, snsMenuItem3, snsMenuItem4, snsMenuItem5, snsMenuItem6, snsMenuItem7, FSTRING_RESET };
#else
static const char* const menuOptionsSNS[] PROGMEM = { snsMenuItem1, snsMenuItem2, snsMenuItem3, snsMenuItem4, snsMenuItem5, snsMenuItem6, FSTRING_RESET };
#endif

// SNES menu items
static const char SnesMenuItem4[] PROGMEM = "Test SRAM";
static const char SnesMenuItem6[] PROGMEM = "Force cart type";
static const char* const menuOptionsSNES[] PROGMEM = { FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, SnesMenuItem4, FSTRING_REFRESH_CART, SnesMenuItem6, FSTRING_RESET };

// Manual config menu items
static const char confMenuItem1[] PROGMEM = "Use header info";
static const char confMenuItem2[] PROGMEM = "4MB LoROM 256K SRAM";
static const char confMenuItem3[] PROGMEM = "4MB HiROM 64K SRAM";
static const char confMenuItem4[] PROGMEM = "6MB ExROM 256K SRAM";
static const char* const menuOptionsConfManual[] PROGMEM = { confMenuItem1, confMenuItem2, confMenuItem3, confMenuItem4, FSTRING_RESET };

#if (defined(ENABLE_FLASH) && defined(ENABLE_FLASH8))
// Repro menu items
static const char reproMenuItem1[] PROGMEM = "CFI LoROM";
static const char reproMenuItem2[] PROGMEM = "CFI HiROM";
static const char reproMenuItem3[] PROGMEM = "LoROM (P0)";
static const char reproMenuItem4[] PROGMEM = "HiROM (P0)";
static const char reproMenuItem5[] PROGMEM = "ExLoROM (P1)";
static const char reproMenuItem6[] PROGMEM = "ExHiROM (P1)";
static const char* const menuOptionsRepro[] PROGMEM = { reproMenuItem1, reproMenuItem2, reproMenuItem3, reproMenuItem4, reproMenuItem5, reproMenuItem6, FSTRING_RESET };

// CFI ROM config
static const char reproCFIItem1[] PROGMEM = "1x 2MB";
static const char reproCFIItem2[] PROGMEM = "2x 2MB";
static const char reproCFIItem3[] PROGMEM = "1x 4MB";
static const char reproCFIItem4[] PROGMEM = "2x 4MB";
static const char reproCFIItem5[] PROGMEM = "4x 2MB";
static const char reproCFIItem6[] PROGMEM = "1x 8MB";
static const char* const menuOptionsReproCFI[] PROGMEM = { reproCFIItem1, reproCFIItem2, reproCFIItem3, reproCFIItem4, reproCFIItem5, reproCFIItem6, FSTRING_RESET };

/* ROM file order
static const char reproEXItem1[] PROGMEM = "ExROM (CDAB)";
static const char reproEXItem2[] PROGMEM = "standard (ABCD)";
static const char* const menuOptionsReproEX[] PROGMEM = { reproEXItem1, reproEXItem2, FSTRING_RESET };
*/

/*boolean reproEXMenu() {
  boolean fileOrder = 0;
  // create menu with title and 3 options to choose from
  unsigned char snsReproEX;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsReproEX, 3);
  snsReproEX = question_box(F("ROM file order"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (snsReproEX) {
    case 0:
      fileOrder = 1;
      break;
    case 1:
      fileOrder = 0;
      break;
    case 2:
      resetArduino();
      break;
  }
  return fileOrder;
}*/

// Setup number of flashroms
void reproCFIMenu() {
  boolean reversed = 1;
  // create menu with title and 7 options to choose from
  unsigned char snsReproCFI;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsReproCFI, 7);
  snsReproCFI = question_box(F("Select Flash Config"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (snsReproCFI) {
    case 0:
      setupCFI();
      flashSize = 2097152;
      writeCFI_Flash(1, 1, 0);
      verifyFlash(1, 1, 0);
      break;

    case 1:
      setupCFI();
      flashSize = 4194304;
      // Write first rom chip
      writeCFI_Flash(1, 2, 0);
      verifyFlash(1, 2, 0);
      delay(300);

      // Switch to second ROM chip, see flash.ino low level functions line 811
      // LoROM
      if (mapping == 1)
        mapping = 122;
      // HiROM
      else if (mapping == 2)
        mapping = 222;

      // Write second rom chip
      display_Clear();
      writeCFI_Flash(2, 2, 0);
      verifyFlash(2, 2, 0);
      break;

    case 2:
      setupCFI();
      flashSize = 4194304;
      writeCFI_Flash(1, 1, 0);
      verifyFlash(1, 1, 0);
      break;

    case 3:
      //reversed = reproEXMenu();
      setupCFI();
      flashSize = 8388608;
      // Write first rom chip
      writeCFI_Flash(1, 2, reversed);
      verifyFlash(1, 2, reversed);
      delay(300);

      // Switch to second ROM chip, see flash.ino low level functions line 811
      // LoROM
      if (mapping == 1)
        mapping = 124;
      // HiROM
      else if (mapping == 2)
        mapping = 224;

      // Write second rom chip
      display_Clear();
      writeCFI_Flash(2, 2, reversed);
      verifyFlash(2, 2, reversed);
      break;

    case 4:
      //reversed = reproEXMenu();
      setupCFI();
      flashSize = 8388608;
      // Write first rom chip
      writeCFI_Flash(1, 4, reversed);
      verifyFlash(1, 4, reversed);
      delay(300);

      // Switch to second ROM chip, see flash.ino low level functions line 811
      // LoROM
      if (mapping == 1)
        mapping = 122;
      // HiROM
      else if (mapping == 2)
        mapping = 222;

      // Write second rom chip
      display_Clear();
      writeCFI_Flash(2, 4, reversed);
      verifyFlash(2, 4, reversed);

      // Switch to third ROM chip
      // LoROM
      if (mapping == 122)
        mapping = 124;
      // HiROM
      else if (mapping == 222)
        mapping = 224;

      // Write second rom chip
      display_Clear();
      writeCFI_Flash(3, 4, reversed);
      verifyFlash(3, 4, reversed);

      // Switch to fourth ROM chip
      // LoROM
      if (mapping == 124)
        mapping = 142;
      // HiROM
      else if (mapping == 224)
        mapping = 242;

      // Write second rom chip
      display_Clear();
      writeCFI_Flash(4, 4, reversed);
      verifyFlash(4, 4, reversed);
      break;

    case 5:
      //reversed = reproEXMenu();
      setupCFI();
      flashSize = 8388608;
      writeCFI_Flash(1, 1, reversed);
      verifyFlash(1, 1, reversed);
      break;

    case 6:
      resetArduino();
      break;
  }

  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 0);
  display_Update();
  wait();
  resetArduino();
}

// SNES repro menu
void reproMenu() {
  // create menu with title and 7 options to choose from
  unsigned char snsRepro;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsRepro, 7);
  snsRepro = question_box(F("Select Repro Type"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (snsRepro) {
    case 0:
      // CFI LoROM
      mapping = 1;
      reproCFIMenu();
      break;

    case 1:
      // CFI HiROM
      mapping = 2;
      reproCFIMenu();
      break;

    case 2:
      // LoRom
      display_Clear();
      display_Update();
      mapping = 1;
      setup_Flash8();
      id_Flash8();
      wait();
      mode = CORE_FLASH8;
      break;

    case 3:
      // HiRom
      display_Clear();
      display_Update();
      mapping = 2;
      setup_Flash8();
      id_Flash8();
      wait();
      mode = CORE_FLASH8;
      break;

    case 4:
      // ExLoRom
      display_Clear();
      display_Update();
      mapping = 124;
      setup_Flash8();
      id_Flash8();
      wait();
      mode = CORE_FLASH8;
      break;

    case 5:
      // ExHiRom
      display_Clear();
      display_Update();
      mapping = 224;
      setup_Flash8();
      id_Flash8();
      wait();
      mode = CORE_FLASH8;
      break;

    case 6:
      resetArduino();
      break;
  }
}
#endif

// SNES start menu
void snsMenu() {
  // create menu with title and 7 options to choose from
  unsigned char snsCart;
  // Copy menuOptions out of progmem
#ifdef OPTION_CLOCKGEN_CALIBRATION
  convertPgm(menuOptionsSNS, 8);
  snsCart = question_box(FS(FSTRING_SELECT_CART_TYPE), menuOptions, 8, 0);
#else
  convertPgm(menuOptionsSNS, 7);
  snsCart = question_box(FS(FSTRING_SELECT_CART_TYPE), menuOptions, 7, 0);
#endif

  // wait for user choice to come back from the question box menu
  switch (snsCart) {
    case 0:
      display_Clear();
      display_Update();
      setup_Snes();
      mode = CORE_SNES;
      break;

#ifdef ENABLE_SFM
    case 1:
      display_Clear();
      display_Update();
      setup_SFM();
      mode = CORE_SFM;
      break;
#endif

#ifdef ENABLE_SV
    case 2:
      display_Clear();
      display_Update();
      setup_SV();
      mode = CORE_SV;
      break;
#endif

#ifdef ENABLE_ST
    case 3:
      display_Clear();
      display_Update();
      setup_ST();
      mode = CORE_ST;
      break;
#endif

#ifdef ENABLE_GPC
    case 4:
      display_Clear();
      display_Update();
      setup_GPC();
      mode = CORE_GPC;
      break;
#endif

#if (defined(ENABLE_FLASH) && defined(ENABLE_FLASH8))
    case 5:
      setup_FlashVoltage();
      reproMenu();
      break;
#endif

    case 6:
#ifdef OPTION_CLOCKGEN_CALIBRATION
      clkcal();
      break;

    case 7:
#endif
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

// SNES Menu
void snesMenu() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSNES, 7);
  mainMenu = question_box(F("SNES Cart Reader"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      {
        if (numBanks > 0) {
          display_Clear();
          // Change working dir to root
          sd.chdir("/");
          // start reading from cart
          readROM_SNES();
          // Internal Checksum
          compare_checksum();
          // CRC32
          compareCRC("snes.txt", 0, 1, 0);
#ifdef ENABLE_GLOBAL_LOG
          save_log();
#endif
          display_Update();
        } else {
          display_Clear();
          print_Error(F("Does not have ROM"));
        }
      }
      break;

    case 1:
      if (sramSize > 0) {
        display_Clear();
        // Change working dir to root
        sd.chdir("/");
        readSRAM();
      } else {
        display_Clear();
        print_Error(F("Does not have SRAM"));
      }
      break;

    case 2:
      if (sramSize > 0) {
        display_Clear();
        // Change working dir to root
        sd.chdir("/");
        writeSRAM(1);
        unsigned long wrErrors;
        wrErrors = verifySRAM();
        if (wrErrors == 0) {
          println_Msg(F("Verified OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(wrErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else {
        display_Clear();
        print_Error(F("Does not have SRAM"));
      }
      break;

    case 3:
      if (sramSize > 0) {
        display_Clear();
        println_Msg(F("Warning:"));
        println_Msg(F("This can erase"));
        println_Msg(F("your save games"));
        println_Msg(FS(FSTRING_EMPTY));
        println_Msg(FS(FSTRING_EMPTY));
        println_Msg(F("Press any button to"));
        println_Msg(F("start sram testing"));
        display_Update();
        wait();
        display_Clear();
        // Change working dir to root
        sd.chdir("/");
        readSRAM();
        eraseSRAM(0x0F);
        eraseSRAM(0xF0);
        writeSRAM(0);
        unsigned long wrErrors = verifySRAM();
        if (wrErrors == 0) {
          println_Msg(F("Restored OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(wrErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else {
        display_Clear();
        print_Error(F("Does not have SRAM"));
      }
      break;

    case 4:
      // For arcademaster1 (Markfrizb) multi-game carts
      // Set reset pin to output (PH0)
      DDRH |= (1 << 0);
      // Switch RST(PH0) to LOW
      PORTH &= ~(1 << 0);

      // Note: It is probably not intended to reset CIC or clocks here
      // But if that's false, uncomment this:
      // stopSnesClocks_resetCic_resetCart();

      display_Clear();
      print_Msg(F("Resetting..."));
      display_Update();
      delay(3000);  // wait 3 secs to switch to next game
      resetArduino();
      break;

    case 5:
      confMenuManual();
      display_Clear();
      display_Update();
      break;

    case 6:
      stopSnesClocks_resetCic_resetCart();
      resetArduino();
      break;
  }
  //println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

// Menu for manual configuration
void confMenuManual() {
  // create menu with title and 5 options to choose from
  unsigned char subMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsConfManual, 5);
  subMenu = question_box(F("Choose mapping"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (subMenu) {
    case 0:
      break;

    case 1:
      romType = LO;
      numBanks = 128;
      sramSize = 256;
      strcpy(romName, "LoROM");
      break;

    case 2:
      romType = HI;
      numBanks = 64;
      sramSize = 64;
      strcpy(romName, "HiROM");
      break;

    case 3:
      romType = EX;
      numBanks = 96;
      sramSize = 256;
      strcpy(romName, "ExROM");
      break;

    case 4:
      // Reset
      stopSnesClocks_resetCic_resetCart();
      resetArduino();
      break;
  }
}

void stopSnesClocks_resetCic_resetCart() {
  DDRG |= (1 << 1);    // Set cicrstPin(PG1) to Output
  PORTG |= (1 << 1);   // pull high = reset CIC
  DDRH |= (1 << 0);    // Set RST(PH0) pin to Output
  PORTH &= ~(1 << 0);  // Switch RST(PH0) to LOW
  if (i2c_found) {
    clockgen.output_enable(SI5351_CLK1, 0);  // CPU clock
    clockgen.output_enable(SI5351_CLK2, 0);  // CIC clock
    clockgen.output_enable(SI5351_CLK0, 0);  // master clock
  }
}

/******************************************
   Setup
 *****************************************/
void setup_Snes() {
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
  //PORTH &= ~(1 << 1);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set expand(PG5) to Input
  DDRG &= ~(1 << 5);
  // Activate Internal Pullup Resistors
  //PORTG |= (1 << 5);

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

  // Adafruit Clock Generator
  initializeClockOffset();

  if (i2c_found) {
    // Set clocks to 4Mhz/1Mhz for better SA-1 unlocking
    clockgen.set_freq(100000000ULL, SI5351_CLK1);  // CPU
    clockgen.set_freq(100000000ULL, SI5351_CLK2);  // CIC
    clockgen.set_freq(400000000ULL, SI5351_CLK0);  // EXT

    // Start outputting master clock, CIC clock
    clockgen.output_enable(SI5351_CLK1, 0);  // no CPU clock yet; seems to affect SA-1 success a lot
    clockgen.output_enable(SI5351_CLK2, 1);  // CIC clock (should go before master clock)
    clockgen.output_enable(SI5351_CLK0, 1);  // master clock

    // Wait for clock generator
    clockgen.update_status();
    delay(500);
  }
#ifdef ENABLE_CLOCKGEN
  else {
    display_Clear();
    print_FatalError(F("Clock Generator not found"));
  }
#endif

  // Start CIC by outputting a low signal to cicrstPin(PG1)
  PORTG &= ~(1 << 1);

  // Wait for CIC reset
  delay(500);

  // Print all the info
  getCartInfo_SNES();

  if (i2c_found) {
    //Set clocks to standard or else SA-1 sram writing will fail
    clockgen.set_freq(2147727200ULL, SI5351_CLK0);
    clockgen.set_freq(357954500ULL, SI5351_CLK1);
    clockgen.set_freq(307200000ULL, SI5351_CLK2);
  }
}

/******************************************
   I/O Functions
 *****************************************/
// Switch control pins to write
void controlOut_SNES() {
  // Switch RD(PH6) and WR(PH5) to HIGH
  PORTH |= (1 << 6) | (1 << 5);
  // Switch CS(PH3) to LOW
  PORTH &= ~(1 << 3);
}

// Switch control pins to read
void controlIn_SNES() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
}

/******************************************
   Low level functions
 *****************************************/
// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank_SNES(byte myBank, word myAddress, byte myData) {
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
byte readBank_SNES(byte myBank, word myAddress) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Wait for the Byte to appear on the data bus
  // Arduino running at 16Mhz -> one nop = 62.5ns
  // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
  // let's be conservative and use 6 x 62.5 = 375ns
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Read
  byte tempByte = PINC;
  return tempByte;
}

void readLoRomBanks(unsigned int start, unsigned int total, FsFile* file) {
  byte buffer[1024] = { 0 };

  uint16_t c = 0;
  uint16_t currByte = 32768;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(total - start) * 1024;
  draw_progressbar(0, totalProgressBar);

  for (word currBank = start; currBank < total; currBank++) {
    PORTL = currBank;

    // Blink led
    blinkLED();

    currByte = 32768;
    while (1) {
      c = 0;
      while (c < 1024) {
        PORTF = (currByte & 0xFF);
        PORTK = ((currByte >> 8) & 0xFF);

        // Wait for the Byte to appear on the data bus
        // Arduino running at 16Mhz -> one nop = 62.5ns
        // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
        // let's be conservative and use 6 x 62.5 = 375ns
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;

        buffer[c] = PINC;
        c++;
        currByte++;
      }
      file->write(buffer, 1024);

      // exit while(1) loop once the uint16_t currByte overflows from 0xffff to 0 (current bank is done)
      if (currByte == 0) break;
    }

    // update progress bar
    processedProgressBar += 1024;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
}

void readHiRomBanks(unsigned int start, unsigned int total, FsFile* file) {
  byte buffer[1024] = { 0 };

  uint16_t c = 0;
  uint16_t currByte = 0;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(total - start) * 1024;
  draw_progressbar(0, totalProgressBar);

  for (word currBank = start; currBank < total; currBank++) {
    PORTL = currBank;

    // Blink led
    blinkLED();

    currByte = 0;
    while (1) {
      c = 0;
      while (c < 1024) {
        PORTF = (currByte & 0xFF);
        PORTK = ((currByte >> 8) & 0xFF);

        // Wait for the Byte to appear on the data bus
        // Arduino running at 16Mhz -> one nop = 62.5ns
        // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
        // let's be conservative and use 6 x 62.5 = 375ns
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;

        buffer[c] = PINC;
        c++;
        currByte++;
      }
      file->write(buffer, 1024);

      // exit while(1) loop once the uint16_t currByte overflows from 0xffff to 0 (current bank is done)
      if (currByte == 0) break;
    }

    // update progress bar
    processedProgressBar += 1024;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
}

/******************************************
  SNES ROM Functions
******************************************/
void getCartInfo_SNES() {
  boolean manualConfig = 0;

  //Prime SA1 cartridge
  PORTL = 192;
  for (uint16_t currByte = 0; currByte < 1024; currByte++) {
    PORTF = currByte & 0xFF;
    PORTK = currByte >> 8;

    // Wait for the Byte to appear on the data bus
    // Arduino running at 16Mhz -> one nop = 62.5ns
    // slowRom is good for 200ns, fastRom is <= 120ns; S-CPU best case read speed: 3.57MHz / 280ns
    // let's be conservative and use 6 x 62.5 = 375ns
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
  }

  // Print start page
  if (checkcart_SNES() == 0) {
    // Checksum either corrupt or 0000
    manualConfig = 1;
    errorLvl = 1;
    rgbLed(red_color);

    display_Clear();
    println_Msg(F("ERROR"));
    println_Msg(F("Rom header corrupt"));
    println_Msg(F("or missing"));
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("Press button for"));
    println_Msg(F("manual configuration"));
    println_Msg(F("or powercycle if SA1"));
    display_Update();
    wait();
    // Wait() clears errors but in this case we still have an error
    errorLvl = 1;
  }

  display_Clear();
  print_Msg(FS(FSTRING_NAME));
  println_Msg(romName);

  print_Msg(FS(FSTRING_REVISION));
  println_Msg(romVersion);

  print_Msg(F("Type: "));
  if (romType == HI)
    print_Msg(F("HiROM"));
  else if (romType == LO)
    print_Msg(F("LoROM"));
  else if (romType == EX)
    print_Msg(F("ExHiRom"));
  else
    print_Msg(romType);
  print_Msg(FS(FSTRING_SPACE));
  if (romSpeed == 0)
    println_Msg(F("SlowROM"));
  else if (romSpeed == 2)
    println_Msg(F("SlowROM"));
  else if (romSpeed == 3)
    println_Msg(F("FastROM"));
  else
    println_Msg(romSpeed);

  print_Msg(F("ICs: ROM "));
  if (romChips == 0)
    println_Msg(F("ONLY"));
  else if (romChips == 1)
    println_Msg(F("RAM"));
  else if (romChips == 2)
    println_Msg(F("SAVE"));
  else if (romChips == 3)
    println_Msg(F("DSP1"));
  else if (romChips == 4)
    println_Msg(F("DSP1 RAM"));
  else if (romChips == 5)
    println_Msg(F("DSP1 SAVE"));
  else if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26))
    println_Msg(F("SuperFX"));
  else if (romChips == 52) {
    println_Msg(F("SA1 RAM"));
    romType = SA;
  } else if (romChips == 53) {
    println_Msg(F("SA1 RAM BATT"));
    romType = SA;
  } else if (romChips == 67) {
    println_Msg(F("SDD1"));
  } else if (romChips == 69) {
    println_Msg(F("SDD1 BATT"));
  } else if (romChips == 85)
    println_Msg(F("SRTC RAM BATT"));
  else if (romChips == 227)
    println_Msg(F("RAM GBoy"));
  else if (romChips == 243)
    println_Msg(F("CX4"));
  else if (romChips == 246)
    println_Msg(F("DSP2"));
  else if (romChips == 245)
    println_Msg(F("SPC RAM BATT"));
  else if (romChips == 249)
    println_Msg(F("SPC RAM RTC"));
  else
    println_Msg(FS(FSTRING_EMPTY));


  print_Msg(FS(FSTRING_ROM_SIZE));
  if ((romSize >> 3) < 1) {
    print_Msg(1024 * romSize >> 3);
    print_Msg(F(" KB"));
  } else {
    print_Msg(romSize >> 3);
    print_Msg(F(" MB"));
  }
  print_Msg(F(" ("));
  print_Msg(numBanks);
  println_Msg(F(" banks)"));

  //print_Msg(F("Chips: "));
  //println_Msg(romChips);

  print_Msg(F("Save Size: "));
  print_Msg(sramSize >> 3);
  println_Msg(F(" KB"));

  print_Msg(FS(FSTRING_CHECKSUM));
  println_Msg(checksumStr);
  display_Update();

  // Wait for user input
#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif
#ifdef ENABLE_SERIAL
  println_Msg(FS(FSTRING_SPACE));
#endif

  // Start manual config
  if (manualConfig == 1) {
    confMenuManual();
  }
}

void checkAltConf(char crcStr[9]) {
  char tempStr2[5];
  char tempStr3[9];

  if (myFile.open("snes.txt", O_READ)) {
    // Get cart info
    display_Clear();
    println_Msg(F("Searching database..."));
    print_Msg(FS(FSTRING_CHECKSUM));
    println_Msg(checksumStr);
    display_Update();

    while (myFile.available()) {
      // Skip first line with name
      skip_line(&myFile);

      // Skip over the CRC checksum
      myFile.seekCur(9);

      // Get internal ROM checksum as string
      for (byte j = 0; j < 4; j++) {
        tempStr2[j] = char(myFile.read());
      }
      tempStr2[4] = '\0';

      // Check if checksum string is a match else go to next entry in database
      if (strcmp(tempStr2, checksumStr) == 0) {
        // Skip the , in the file
        myFile.seekCur(1);

        // Read the CRC32 of the SNES header out of database
        for (byte k = 0; k < 8; k++) {
          tempStr3[k] = char(myFile.read());
        }
        tempStr3[8] = '\0';

        print_Msg(F("Header CRC32: "));
        println_Msg(tempStr3);
        display_Update();

        // Skip the , in the file
        myFile.seekCur(1);

        // Read file size
        byte romSize2 = (myFile.read() - 48) * 10 + (myFile.read() - 48);

        // Skip the , in the file
        myFile.seekCur(1);

        // Read number of banks
        byte numBanks2 = (myFile.read() - 48) * 100 + (myFile.read() - 48) * 10 + (myFile.read() - 48);

        // skip CRLF
        myFile.seekCur(2);
        // skip third empty line
        skip_line(&myFile);

        // Some games have the same checksum, so compare CRC32 of header area with database too
        if (strcmp(tempStr3, crcStr) == 0) {
          println_Msg(F("Found"));
          display_Update();
          // Game found, check if ROM sizes differ but only change ROM size if non- standard size found in database, else trust the header to be right and the database to be wrong
          if (((romSize != romSize2) || (numBanks != numBanks2)) && ((romSize2 == 10) || (romSize2 == 12) || (romSize2 == 20) || (romSize2 == 24) || (romSize2 == 40) || (romSize2 == 48))) {
            // Correct size
            println_Msg(F("Correcting size"));
            print_Msg(FS(FSTRING_SIZE));
            print_Msg(romSize);
            print_Msg(F(" -> "));
            print_Msg(romSize2);
            println_Msg(F("Mbit"));
            print_Msg(F("Banks: "));
            print_Msg(numBanks);
            print_Msg(F(" -> "));
            println_Msg(numBanks2);
            display_Update();
            delay(1000);
            romSize = romSize2;
            numBanks = numBanks2;
          }
          break;
        }
      }
      // If no match go to next entry
      else {
        // skip rest of line
        myFile.seekCur(18);
        // skip third empty line
        skip_line(&myFile);
      }
    }
    // Close the file:
    myFile.close();
  }
}

// Read header
boolean checkcart_SNES() {
  // set control to read
  dataIn();

  uint16_t headerStart = 0xFFB0;
  byte snesHeader[80];
  PORTL = 0;
  for (uint16_t c = 0, currByte = headerStart; c < 80; c++, currByte++) {
    PORTF = (currByte & 0xFF);
    PORTK = ((currByte >> 8) & 0xFF);

    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;

    snesHeader[c] = PINC;
  }

  // Calculate CRC32 of header
  char crcStr[9];
  sprintf(crcStr, "%08lX", calculateCRC(snesHeader, 80));

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", snesHeader[0xFFDF - headerStart], snesHeader[0xFFDE - headerStart]);

  romType = snesHeader[0xFFD5 - headerStart];
  if ((romType >> 5) != 1) {  // Detect invalid romType byte due to too long ROM name (22 chars)
    romType = LO;             // LoROM   // Krusty's Super Fun House (U) 1.0 & Contra 3 (U)
  } else if (romType == 0x35) {
    romType = EX;  // Check if ExHiROM
  } else if (romType == 0x3A) {
    romType = HI;                                 // Check if SPC7110
  } else if (strcmp("3BB0", checksumStr) == 0) {  // invalid romType due to too long ROM name (Yuyu no Quiz de GO!GO!)
    romType = LO;
  } else {
    romType &= 1;  // Must be LoROM or HiROM
  }

  // Check RomSpeed
  romSpeed = (snesHeader[0xFFD5 - headerStart] >> 4);

  // Check RomChips
  romChips = snesHeader[0xFFD6 - headerStart];

  if (romChips == 69) {
    romSize = 48;
    numBanks = 96;
    romType = HI;
  } else if (romChips == 67) {
    romSize = 32;
    numBanks = 64;
    romType = HI;
  } else if (romChips == 243) {
    cx4Type = snesHeader[0xFFC9 - headerStart] & 0xF;
    if (cx4Type == 2) {  // X2
      romSize = 12;
      numBanks = 48;
    } else if (cx4Type == 3) {  // X3
      romSize = 16;
      numBanks = 64;
    }
  } else if ((romChips == 245) && (romType == HI)) {
    romSize = 24;
    numBanks = 48;
  } else if ((romChips == 249) && (romType == HI)) {
    romSize = 40;
    numBanks = 80;
  } else {
    // Check RomSize
    byte romSizeExp = snesHeader[0xFFD7 - headerStart] - 7;
    romSize = 1;
    while (romSizeExp--)
      romSize *= 2;

    if ((romType == EX) || (romType == SA)) {
      numBanks = long(romSize) * 2;
    } else {
      numBanks = (long(romSize) * 1024 * 1024 / 8) / (32768 + (long(romType) * 32768));
    }
  }

  //Check SD card for alt config, pass CRC32 of snesHeader but filter out 0000 and FFFF checksums
  if (!(strcmp(checksumStr, "0000") == 0) && !(strcmp(checksumStr, "FFFF") == 0)) {
    checkAltConf(crcStr);
  }

  // Get name
  byte myLength = buildRomName(romName, &snesHeader[0xFFC0 - headerStart], 21);

  // If name consists out of all japanese characters use game code
  if (myLength == 0) {
    // Get rom code
    romName[0] = 'S';
    romName[1] = 'H';
    romName[2] = 'V';
    romName[3] = 'C';
    romName[4] = '-';
    for (unsigned int i = 0; i < 4; i++) {
      byte myByte;
      myByte = snesHeader[0xFFB2 + i - headerStart];
      if (((myByte >= '0' && myByte <= '9') || (myByte >= 'A' && myByte <= 'z')) && myLength < 4) {
        romName[myLength + 5] = myByte;
        myLength++;
      }
    }
    if (myLength == 0) {
      // Rom code unknown
      romName[0] = 'U';
      romName[1] = 'N';
      romName[2] = 'K';
      romName[3] = 'N';
      romName[4] = 'O';
      romName[5] = 'W';
      romName[6] = 'N';
    }
  }

  // Read sramSizeExp
  byte sramSizeExp;
  if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {
    // SuperFX
    if (snesHeader[0xFFDA - headerStart] == 0x33) {
      sramSizeExp = snesHeader[0xFFBD - headerStart];
    } else {
      if (strncmp(romName, "STARFOX2", 8) == 0) {
        sramSizeExp = 6;
      } else {
        sramSizeExp = 5;
      }
    }
  } else {
    // No SuperFX
    sramSizeExp = snesHeader[0xFFD8 - headerStart];
  }

  // Calculate sramSize
  // Fail states usually have sramSizeExp at 255 (no cart inserted, SA-1 failure, etc)
  if (sramSizeExp != 0 && sramSizeExp != 255) {
    sramSizeExp = sramSizeExp + 3;
    sramSize = 1;
    while (sramSizeExp--)
      sramSize *= 2;
  } else {
    sramSize = 0;
  }

  // Check Cart Country
  //int cartCountry = snesHeader[0xFFD9 - headerStart];

  // ROM Version
  romVersion = snesHeader[0xFFDB - headerStart];

  // Test if checksum is equal to reverse checksum
  if (((word(snesHeader[0xFFDC - headerStart]) + (word(snesHeader[0xFFDD - headerStart]) * 256)) + (word(snesHeader[0xFFDE - headerStart]) + (word(snesHeader[0xFFDF - headerStart]) * 256))) == 65535) {
    if (strcmp("0000", checksumStr) == 0) {
      return 0;
    } else {
      return 1;
    }
  }
  // Either rom checksum is wrong or no cart is inserted
  else {
    return 0;
  }
}

unsigned int calc_checksum(char* fileName, char* folder) {
  unsigned int calcChecksum = 0;
  unsigned int calcChecksumChunk = 0;
  int calcFilesize = 0;
  unsigned int c = 0;
  unsigned long i = 0;
  unsigned long j = 0;

  if (strcmp(folder, "root") != 0)
    sd.chdir(folder);

  // If file exists
  if (myFile.open(fileName, O_READ)) {
    calcFilesize = myFile.fileSize() * 8 / 1024 / 1024;

    // Nintendo Power (SF Memory Cassette)
    // Read up to 0x60000 then add FFs to 0x80000
    if (NP == true) {
      for (i = 0; i < (0x60000 / 512); i++) {
        myFile.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
        calcChecksum = calcChecksumChunk;
      }
      calcChecksum += 0xF47C;  // FFs from 0x60000-0x80000
    } else if ((calcFilesize == 10) || (calcFilesize == 12) || (calcFilesize == 20) || (calcFilesize == 24)) {
      unsigned long calcBase = 0;
      unsigned long calcMirror = 0;
      byte calcMirrorCount = 0;
      if (calcFilesize > 16)
        calcBase = 2097152;
      else
        calcBase = 1048576;
      calcMirror = myFile.fileSize() - calcBase;
      calcMirrorCount = calcBase / calcMirror;

      // Momotarou Dentetsu Happy Fix 3MB (24Mbit)
      if ((calcFilesize == 24) && (romChips == 245)) {
        for (i = 0; i < (myFile.fileSize() / 512); i++) {
          myFile.read(sdBuffer, 512);
          for (c = 0; c < 512; c++) {
            calcChecksumChunk += sdBuffer[c];
          }
        }
        calcChecksum = 2 * calcChecksumChunk;
      } else {
        // Base 8/16 Mbit chunk
        for (j = 0; j < (calcBase / 512); j++) {
          myFile.read(sdBuffer, 512);
          for (c = 0; c < 512; c++) {
            calcChecksumChunk += sdBuffer[c];
          }
        }
        calcChecksum = calcChecksumChunk;
        calcChecksumChunk = 0;
        // Add the mirrored chunk
        for (j = 0; j < (calcMirror / 512); j++) {
          myFile.read(sdBuffer, 512);
          for (c = 0; c < 512; c++) {
            calcChecksumChunk += sdBuffer[c];
          }
        }
        calcChecksum += calcMirrorCount * calcChecksumChunk;
      }
    } else if ((calcFilesize == 40) && (romChips == 85)) {
      // Daikaijuu Monogatari 2 Fix 5MB (40Mbit)
      // Add the 4MB (32Mbit) start
      for (j = 0; j < (4194304 / 512); j++) {
        myFile.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
        calcChecksum = calcChecksumChunk;
      }
      calcChecksumChunk = 0;
      // Add the 1MB (8Mbit) end
      for (j = 0; j < (1048576 / 512); j++) {
        myFile.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
      }
      calcChecksum += 4 * calcChecksumChunk;
    } else if (calcFilesize == 48) {
      // Star Ocean/Tales of Phantasia Fix 6MB (48Mbit)
      // Add the 4MB (32Mbit) start
      for (j = 0; j < (4194304 / 512); j++) {
        myFile.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
        calcChecksum = calcChecksumChunk;
      }
      calcChecksumChunk = 0;
      // Add the 2MB (16Mbit) end
      for (j = 0; j < (2097152 / 512); j++) {
        myFile.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
      }
      calcChecksum += 2 * calcChecksumChunk;
    } else {
      //calcFilesize == 2 || 4 || 8 || 16 || 32 || 40 || etc
      for (i = 0; i < (myFile.fileSize() / 512); i++) {
        myFile.read(sdBuffer, 512);
        for (c = 0; c < 512; c++) {
          calcChecksumChunk += sdBuffer[c];
        }
        calcChecksum = calcChecksumChunk;
      }
    }
    myFile.close();
    sd.chdir();
    return (calcChecksum);
  } else {
    // Else show error
    print_Error(F("DUMP ROM 1ST"));
    return 0;
  }
}

boolean compare_checksum() {
  print_Msg(F("Checksum... "));
  display_Update();

  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum(fileName, folder));
  print_Msg(calcsumStr);

  if (strcmp(calcsumStr, checksumStr) == 0) {
    println_Msg(F(" -> OK"));
    display_Update();
    return 1;
  } else {
    print_Msg(F(" != "));
    println_Msg(checksumStr);
    print_Error(F("Invalid Checksum"));
    display_Update();
    return 0;
  }
}

// Read rom to SD card
void readROM_SNES() {
  // Set control
  dataIn();
  controlIn_SNES();

  // Get name, add extension and convert to char array for sd lib
  createFolderAndOpenFile("SNES", "ROM", romName, "sfc");

  //Dump Derby Stallion '96 (Japan) and Sound Novel Tsukuru (Japan) - Actual Size is 24Mb
  if ((romType == LO) && (numBanks == 96) && ((strcmp("CC86", checksumStr) == 0) || (strcmp("A77B", checksumStr) == 0))) {
    // Read Banks 0x00-0x3F for the 1st/2nd MB
    for (int currBank = 0; currBank < 64; currBank++) {
      // Dump the bytes to SD 512B at a time
      for (long currByte = 32768; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SNES(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
    //Read Bank 0x80-9F for the 3rd MB
    for (int currBank = 128; currBank < 160; currBank++) {
      // Dump the bytes to SD 512B at a time
      for (long currByte = 32768; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SNES(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
  }

  //Dump Low-type ROM
  else if (romType == LO) {
    if (romChips == 243) {                    //0xF3
      cx4Map = readBank_SNES(0, 32594);       //0x7F52
      if ((cx4Type == 2) && (cx4Map != 0)) {  //X2
        dataOut();
        controlOut_SNES();
        writeBank_SNES(0, 32594, 0);  // Set 0x7F52 to 0
        dataIn();
        controlIn_SNES();
      } else if ((cx4Type == 3) && (cx4Map == 0)) {  //X3
        dataOut();
        controlOut_SNES();
        writeBank_SNES(0, 32594, 1);  // Set 0x7F52 to 1
        dataIn();
        controlIn_SNES();
      }
    }
    if (romSize > 24) {
      // ROM > 96 banks (up to 128 banks)
      readLoRomBanks(0x80, numBanks + 0x80, &myFile);
    } else {
      // Read up to 96 banks starting at bank 0×00.
      readLoRomBanks(0, numBanks, &myFile);
    }
    if (romChips == 243) {  //0xF3
      // Restore CX4 Mapping Register
      dataOut();
      controlOut_SNES();
      writeBank_SNES(0, 32594, cx4Map);  // 0x7F52
      dataIn();
      controlIn_SNES();
    }
  }

  // Dump SDD1 High-type ROM
  else if ((romType == HI) && (romChips == 69 || romChips == 67)) {
    println_Msg(F("Dumping SDD1 HiRom"));
    display_Update();
    controlIn_SNES();
    byte initialSOMap = readBank_SNES(0, 18439);

    for (word currMemmap = 0; currMemmap < (numBanks / 16); currMemmap++) {

      dataOut();
      controlOut_SNES();

      writeBank_SNES(0, 18439, currMemmap);

      dataIn();
      controlIn_SNES();

      readHiRomBanks(240, 256, &myFile);
      if (currMemmap == 2) display_Clear();  // need more space for the progress bars
    }

    dataOut();
    controlOut_SNES();

    writeBank_SNES(0, 18439, initialSOMap);

    dataIn();
    controlIn_SNES();
  }

  // Dump SPC7110 High-type ROM
  else if ((romType == HI) && ((romChips == 245) || (romChips == 249))) {
    println_Msg(F("Dumping SPC7110 HiRom"));
    display_Update();

    // 0xC00000-0xDFFFFF
    //print_Msg(F("Part 1"));
    display_Update();
    readHiRomBanks(192, 224, &myFile);

    if (numBanks > 32) {
      dataOut();
      controlOut_SNES();
      // Set 0x4834 to 0xFF
      writeBank_SNES(0, 0x4834, 0xFF);

      dataIn();
      controlIn_SNES();

      // 0xE00000-0xEFFFFF
      //print_Msg(F(" 2"));
      display_Update();
      readHiRomBanks(224, 240, &myFile);

      if (numBanks > 48) {
        // 0xF00000-0xFFFFFF
        //print_Msg(F(" 3"));
        display_Update();
        readHiRomBanks(240, 256, &myFile);

        dataOut();
        controlOut_SNES();

        // Set 0x4833 to 3
        writeBank_SNES(0, 0x4833, 3);

        dataIn();
        controlIn_SNES();

        // 0xF00000-0xFFFFFF
        //print_Msg(F(" 4"));
        display_Update();
        readHiRomBanks(240, 256, &myFile);
      }
      //println_Msg(FS(FSTRING_EMPTY));
      display_Clear();  // need more space due to the 4 progress bars

      // Return mapping registers to initial settings...
      dataOut();
      controlOut_SNES();

      writeBank_SNES(0, 0x4833, 2);
      writeBank_SNES(0, 0x4834, 0);

      dataIn();
      controlIn_SNES();
    }
  }

  // Dump standard High-type ROM
  else if ((romType == HI) || (romType == SA) || (romType == EX)) {
    println_Msg(F("Dumping HiRom..."));
    display_Update();

    if (romChips == 85) {
      // Daikaijuu Monogatari 2, keeps out S-RTC register area
      readHiRomBanks(192, 192 + 64, &myFile);
      readHiRomBanks(64, numBanks, &myFile);  // (64 + (numBanks - 64))
    } else {
      readHiRomBanks(192, numBanks + 192, &myFile);
    }
  }

  // Close the file:
  myFile.close();
}

/******************************************
  SNES SRAM Functions
*****************************************/
// Write file to SRAM
void writeSRAM(boolean browseFile) {
  if (browseFile) {
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser(F("Select srm file"));
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    display_Clear();
  } else
    sprintf(filePath, "%s", fileName);

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Set pins to output
    dataOut();

    // Set RST RD WR to High and CS to Low
    controlOut_SNES();

    int sramBanks = 0;
    // LoRom
    if (romType == LO) {
      // Sram size
      long lastByte = (long(sramSize) * 128);

      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {  // SuperFX
        if (lastByte > 0x10000) {                                                          // Large SuperFX SRAM (no known carts)
          sramBanks = lastByte / 0x10000;
          for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
            for (long currByte = 0x0000; currByte < 0x10000; currByte++) {
              writeBank_SNES(currBank, currByte, myFile.read());
            }
          }
        } else {  // SuperFX SRAM
          for (long currByte = 0; currByte < lastByte; currByte++) {
            writeBank_SNES(0x70, currByte, myFile.read());
          }
        }
      } else if (lastByte > 0x8000) {  // Large SRAM Fix
        sramBanks = lastByte / 0x8000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0x0000; currByte < 0x8000; currByte++) {
            writeBank_SNES(currBank, currByte, myFile.read());
          }
        }
      } else {
        for (long currByte = 0; currByte < lastByte; currByte++) {
          writeBank_SNES(0x70, currByte, myFile.read());
        }
      }
    }
    // HiRom
    else if (romType == HI) {
      if ((romChips == 245) || (romChips == 249)) {  // SPC7110 SRAM
        // Configure SPC7110 SRAM Register
        // Set 0x4830 to 0x80
        writeBank_SNES(0, 0x4830, 0x80);
        // Sram size
        long lastByte = (long(sramSize) * 128) + 0x6000;
        // Write to sram bank
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          writeBank_SNES(0x30, currByte, myFile.read());
        }
        // Reset SPC7110 SRAM Register
        dataOut();
        // Reset 0x4830 to 0x0
        writeBank_SNES(0, 0x4830, 0);
        dataIn();
      } else {
        // Writing SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);
        // Sram size
        long lastByte = (long(sramSize) * 128);
        if (lastByte > 0x2000) {  // Large SRAM Fix
          sramBanks = lastByte / 0x2000;
          for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
            for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
              writeBank_SNES(currBank, currByte, myFile.read());
            }
          }
        } else {
          lastByte += 0x6000;
          // Write to sram bank
          for (long currByte = 0x6000; currByte < lastByte; currByte++) {
            writeBank_SNES(0x30, currByte, myFile.read());
          }
        }
      }
    }
    // ExHiRom
    else if (romType == EX) {
      // Writing SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) {  // Large EX SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
            writeBank_SNES(currBank, currByte, myFile.read());
          }
        }
      } else {
        lastByte += 0x6000;
        // Write to sram bank
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          writeBank_SNES(0xB0, currByte, myFile.read());
        }
      }
    }
    // SA1
    else if (romType == SA) {
      long lastByte = (long(sramSize) * 128);
      if (i2c_found) {
        // Enable CPU Clock
        clockgen.output_enable(SI5351_CLK1, 1);
      }

      // Direct writes to BW-RAM (SRAM) in banks 0x40-0x43 don't work
      // Break BW-RAM (SRAM) into 0x2000 blocks
      byte lastBlock = 0;
      lastBlock = lastByte / 0x2000;

      // Writing SRAM on SA1 needs CS(PH3) to be high
      // PORTH |=  (1 << 3);

      // Setup BW-RAM
      // Set 0x2224 (SNES BMAPS) to map SRAM Block 0 to 0x6000-0x7FFF
      writeBank_SNES(0, 0x2224, 0);
      // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
      writeBank_SNES(0, 0x2226, 0x80);
      // Set 0x2228 (SNES BWPA) to 0x00 BW-RAM Write-Protected Area
      writeBank_SNES(0, 0x2228, 0);
      delay(1000);

      // Use $2224 (SNES) to map BW-RAM block to 0x6000-0x7FFF
      // Use $2226 (SNES) to write enable the BW-RAM
      byte firstByte = 0;
      for (byte currBlock = 0; currBlock < lastBlock; currBlock++) {
        // Set 0x2224 (SNES BMAPS) to map SRAM Block to 0x6000-0x7FFF
        writeBank_SNES(0, 0x2224, currBlock);
        // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
        writeBank_SNES(0, 0x2226, 0x80);
        for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
          myFile.read(sdBuffer, 512);
          if ((currBlock == 0) && (currByte == 0x6000)) {
            firstByte = sdBuffer[0];
          }
          for (int c = 0; c < 512; c++) {
            writeBank_SNES(0, currByte + c, sdBuffer[c]);
          }
        }
      }
      // Rewrite First Byte
      writeBank_SNES(0, 0x2224, 0);
      writeBank_SNES(0, 0x2226, 0x80);
      writeBank_SNES(0, 0x6000, firstByte);
      if (i2c_found) {
        // Disable CPU clock
        clockgen.output_enable(SI5351_CLK1, 0);
      }
    }

    // Set pins to input
    dataIn();

    // Close the file:
    myFile.close();
    println_Msg(F("SRAM writing finished"));
    display_Update();

  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

void readSRAM() {
  // set control
  controlIn_SNES();

  // Get name, add extension and convert to char array for sd lib
  createFolder("SNES", "SAVE", romName, "srm");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }
  int sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {  // SuperFX
      if (lastByte > 0x10000) {                                                          // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0x0000; currByte < 0x10000; currByte++) {
            myFile.write(readBank_SNES(currBank, currByte));
          }
        }
      } else {  // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte++) {
          myFile.write(readBank_SNES(0x70, currByte));
        }
      }
    } else if (lastByte > 0x8000) {  // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0x0000; currByte < 0x8000; currByte++) {
          myFile.write(readBank_SNES(currBank, currByte));
        }
      }
    } else {
      for (long currByte = 0; currByte < lastByte; currByte++) {
        myFile.write(readBank_SNES(0x70, currByte));
      }
    }
  } else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) {  // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      dataOut();
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      dataIn();
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        myFile.write(readBank_SNES(0x30, currByte));
      }
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    } else {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) {  // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
            myFile.write(readBank_SNES(currBank, currByte));
          }
        }
      } else {
        lastByte += 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          myFile.write(readBank_SNES(0x30, currByte));
        }
      }
    }
  } else if (romType == EX) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |= (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x2000) {  // Large EX SRAM Fix
      sramBanks = lastByte / 0x2000;
      for (int currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++) {
        for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
          myFile.write(readBank_SNES(currBank, currByte));
        }
      }
    } else {
      lastByte += 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        myFile.write(readBank_SNES(0xB0, currByte));
      }
    }
  } else if (romType == SA) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |= (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x10000) {
      sramBanks = lastByte / 0x10000;
      for (int currBank = 0x40; currBank < sramBanks + 0x40; currBank++) {
        for (long currByte = 0; currByte < 0x10000; currByte++) {
          myFile.write(readBank_SNES(currBank, currByte));
        }
      }
    } else {
      for (long currByte = 0x0; currByte < lastByte; currByte++) {
        myFile.write(readBank_SNES(0x40, currByte));
      }
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
}

// Check if the SRAM was written without any error
unsigned long verifySRAM() {
  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Variable for errors
    writeErrors = 0;

    // Set control
    controlIn_SNES();

    int sramBanks = 0;
    if (romType == LO) {
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {  // SuperFX
        if (lastByte > 0x10000) {                                                          // Large SuperFX SRAM (no known carts)
          sramBanks = lastByte / 0x10000;
          for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
            for (long currByte = 0; currByte < 0x10000; currByte += 512) {
              //fill sdBuffer
              myFile.read(sdBuffer, 512);
              for (int c = 0; c < 512; c++) {
                if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                  writeErrors++;
                }
              }
            }
          }
        } else {  // SuperFX SRAM
          for (long currByte = 0; currByte < lastByte; currByte += 512) {
            //fill sdBuffer
            myFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(0x70, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      } else if (lastByte > 0x8000) {  // Large SRAM Fix
        sramBanks = lastByte / 0x8000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0; currByte < 0x8000; currByte += 512) {
            //fill sdBuffer
            myFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      } else {
        for (long currByte = 0; currByte < lastByte; currByte += 512) {
          //fill sdBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x70, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
    } else if (romType == HI) {
      if ((romChips == 245) || (romChips == 249)) {  // SPC7110 SRAM
        // Configure SPC7110 SRAM Register
        dataOut();
        // Set 0x4830 to 0x80
        writeBank_SNES(0, 0x4830, 0x80);
        dataIn();
        // Sram size
        long lastByte = (long(sramSize) * 128) + 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
          //fill sdBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x30, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
        dataOut();
        // Reset 0x4830 to 0x0
        writeBank_SNES(0, 0x4830, 0);
        dataIn();
      } else {
        // Dumping SRAM on HiRom needs CS(PH3) to be high
        PORTH |= (1 << 3);
        // Sram size
        long lastByte = (long(sramSize) * 128);
        if (lastByte > 0x2000) {  // Large SRAM Fix
          sramBanks = lastByte / 0x2000;
          for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
            for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
              //fill sdBuffer
              myFile.read(sdBuffer, 512);
              for (int c = 0; c < 512; c++) {
                if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                  writeErrors++;
                }
              }
            }
          }
        } else {
          lastByte += 0x6000;
          for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
            //fill sdBuffer
            myFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(0x30, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      }
    } else if (romType == EX) {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) {  // Large EX SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
            //fill sdBuffer
            myFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      } else {
        lastByte += 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
          //fill sdBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0xB0, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
    } else if (romType == SA) {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);

      if (lastByte > 0x10000) {
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x40; currBank < sramBanks + 0x40; currBank++) {
          for (long currByte = 0x0; currByte < 0x10000; currByte += 512) {
            //fill sdBuffer
            myFile.read(sdBuffer, 512);
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != sdBuffer[c]) {
                writeErrors++;
              }
            }
          }
        }
      } else {
        for (long currByte = 0x0; currByte < lastByte; currByte += 512) {
          //fill sdBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x40, currByte + c)) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
      // Reset SA1
      // Set pins to input
      dataIn();
      // Close the file:
      myFile.close();
      if (writeErrors == 0) {
        println_Msg(F("Verified OK"));
      } else {
        print_STR(error_STR, 0);
        print_Msg(writeErrors);
        print_STR(_bytes_STR, 1);
        print_Error(did_not_verify_STR);
      }
      display_Update();
      wait();

      stopSnesClocks_resetCic_resetCart();

      display_Clear();
      print_Msg(F("Resetting..."));
      display_Update();
      delay(3000);  // wait 3 secs
      resetArduino();
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  } else {
    print_Error(open_file_STR);
    return 1;
  }
}

// Overwrite the entire SRAM
boolean eraseSRAM(byte b) {
  print_Msg(F("0x"));
  print_Msg(b, HEX);
  print_Msg(F(": "));
  display_Update();

  // Set pins to output
  dataOut();

  // Set control pins
  controlOut_SNES();

  int sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);

    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {  // SuperFX
      if (lastByte > 0x10000) {                                                          // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0x0000; currByte < 0x10000; currByte++) {
            writeBank_SNES(currBank, currByte, b);
          }
        }
      } else {  // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte++) {
          writeBank_SNES(0x70, currByte, b);
        }
      }
    } else if (lastByte > 0x8000) {  // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0x0000; currByte < 0x8000; currByte++) {
          writeBank_SNES(currBank, currByte, b);
        }
      }
    } else {
      for (long currByte = 0; currByte < lastByte; currByte++) {
        writeBank_SNES(0x70, currByte, b);
      }
    }
  } else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) {  // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      // Write to sram bank
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        writeBank_SNES(0x30, currByte, b);
      }
      // Reset SPC7110 SRAM Register
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    } else {
      // Writing SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) {  // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
            writeBank_SNES(currBank, currByte, b);
          }
        }
      } else {
        lastByte += 0x6000;
        // Write to sram bank
        for (long currByte = 0x6000; currByte < lastByte; currByte++) {
          writeBank_SNES(0x30, currByte, b);
        }
      }
    }
  }
  // ExHiRom
  else if (romType == EX) {
    // Writing SRAM on HiRom needs CS(PH3) to be high
    PORTH |= (1 << 3);
    /// Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x2000) {  // Large EX SRAM Fix
      sramBanks = lastByte / 0x2000;
      for (int currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++) {
        for (long currByte = 0x6000; currByte < 0x8000; currByte++) {
          writeBank_SNES(currBank, currByte, b);
        }
      }
    } else {
      lastByte += 0x6000;
      // Write to sram bank
      for (long currByte = 0x6000; currByte < lastByte; currByte++) {
        writeBank_SNES(0xB0, currByte, b);
      }
    }
  }
  // SA1
  else if (romType == SA) {
    long lastByte = (long(sramSize) * 128);
    if (i2c_found) {
      // Enable CPU Clock
      clockgen.output_enable(SI5351_CLK1, 1);
    }

    // Direct writes to BW-RAM (SRAM) in banks 0x40-0x43 don't work
    // Break BW-RAM (SRAM) into 0x2000 blocks
    // Use $2224 to map BW-RAM block to 0x6000-0x7FFF
    byte lastBlock = 0;
    lastBlock = lastByte / 0x2000;

    // Writing SRAM on SA1 needs CS(PH3) to be high
    // PORTH |=  (1 << 3);

    // Setup BW-RAM
    // Set 0x2224 (SNES BMAPS) to map SRAM Block 0 to 0x6000-0x7FFF
    writeBank_SNES(0, 0x2224, 0);
    // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
    writeBank_SNES(0, 0x2226, 0x80);
    // Set 0x2228 (SNES BWPA) to 0x00 BW-RAM Write-Protected Area
    writeBank_SNES(0, 0x2228, 0);
    delay(1000);

    // Use $2224 (SNES) to map BW-RAM block to 0x6000-0x7FFF
    // Use $2226 (SNES) to write enable the BW-RAM
    for (byte currBlock = 0; currBlock < lastBlock; currBlock++) {
      // Set 0x2224 (SNES BMAPS) to map SRAM Block to 0x6000-0x7FFF
      writeBank_SNES(0, 0x2224, currBlock);
      // Set 0x2226 (SNES SBWE) to 0x80 Write Enable
      writeBank_SNES(0, 0x2226, 0x80);
      for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          writeBank_SNES(0, currByte + c, b);
        }
      }
    }
    // Rewrite First Byte
    writeBank_SNES(0, 0x2224, 0);
    writeBank_SNES(0, 0x2226, 0x80);
    writeBank_SNES(0, 0x6000, b);

    if (i2c_found) {
      // Disable CPU clock
      clockgen.output_enable(SI5351_CLK1, 0);
    }
  }

  dataIn();

  // Variable for errors
  writeErrors = 0;

  // Set control
  controlIn_SNES();

  sramBanks = 0;
  if (romType == LO) {
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if ((romChips == 19) || (romChips == 20) || (romChips == 21) || (romChips == 26)) {  // SuperFX
      if (lastByte > 0x10000) {                                                          // Large SuperFX SRAM (no known carts)
        sramBanks = lastByte / 0x10000;
        for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
          for (long currByte = 0; currByte < 0x10000; currByte += 512) {
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != b) {
                writeErrors++;
              }
            }
          }
        }
      } else {  // SuperFX SRAM
        for (long currByte = 0; currByte < lastByte; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x70, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    } else if (lastByte > 0x8000) {  // Large SRAM Fix
      sramBanks = lastByte / 0x8000;
      for (int currBank = 0x70; currBank < sramBanks + 0x70; currBank++) {
        for (long currByte = 0; currByte < 0x8000; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    } else {
      for (long currByte = 0; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x70, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
    }
  } else if (romType == HI) {
    if ((romChips == 245) || (romChips == 249)) {  // SPC7110 SRAM
      // Configure SPC7110 SRAM Register
      dataOut();
      // Set 0x4830 to 0x80
      writeBank_SNES(0, 0x4830, 0x80);
      dataIn();
      // Sram size
      long lastByte = (long(sramSize) * 128) + 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x30, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
      dataOut();
      // Reset 0x4830 to 0x0
      writeBank_SNES(0, 0x4830, 0);
      dataIn();
    } else {
      // Dumping SRAM on HiRom needs CS(PH3) to be high
      PORTH |= (1 << 3);
      // Sram size
      long lastByte = (long(sramSize) * 128);
      if (lastByte > 0x2000) {  // Large SRAM Fix
        sramBanks = lastByte / 0x2000;
        for (int currBank = 0x30; currBank < sramBanks + 0x30; currBank++) {
          for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
            for (int c = 0; c < 512; c++) {
              if ((readBank_SNES(currBank, currByte + c)) != b) {
                writeErrors++;
              }
            }
          }
        }
      } else {
        lastByte += 0x6000;
        for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(0x30, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    }
  } else if (romType == EX) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |= (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x2000) {  // Large EX SRAM Fix
      sramBanks = lastByte / 0x2000;
      for (int currBank = 0xB0; currBank < sramBanks + 0xB0; currBank++) {
        for (long currByte = 0x6000; currByte < 0x8000; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    } else {
      lastByte += 0x6000;
      for (long currByte = 0x6000; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0xB0, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
    }
  } else if (romType == SA) {
    // Dumping SRAM on HiRom needs CS(PH3) to be high
    PORTH |= (1 << 3);
    // Sram size
    long lastByte = (long(sramSize) * 128);
    if (lastByte > 0x10000) {
      sramBanks = lastByte / 0x10000;
      for (int currBank = 0x40; currBank < sramBanks + 0x40; currBank++) {
        for (long currByte = 0x0; currByte < 0x10000; currByte += 512) {
          for (int c = 0; c < 512; c++) {
            if ((readBank_SNES(currBank, currByte + c)) != b) {
              writeErrors++;
            }
          }
        }
      }
    } else {
      for (long currByte = 0x0; currByte < lastByte; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          if ((readBank_SNES(0x40, currByte + c)) != b) {
            writeErrors++;
          }
        }
      }
    }
  }
  if (writeErrors == 0) {
    println_Msg(FS(FSTRING_OK));
    return 1;
  } else {
    println_Msg(F("ERROR"));
    return 0;
  }
  display_Update();
}

#endif

//******************************************
// End of File
//******************************************
