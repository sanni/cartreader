//******************************************
// NES MODULE
//******************************************
// mostly copy&pasted from "Famicom Dumper" 2019-08-31 by skaman
// also based on "CoolArduino" by HardWareMan
// Pinout changes: LED and CIRAM_A10

#ifdef enable_NES

//Line Content
//28   Supported Mappers
//106  Defines
//136  Variables
//197  Menus
//383  Setup
//412  No-Intro SD Database Functions
//1125 Low Level Functions
//1372 CRC Functions
//1426 File Functions
//1527 NES 2.0 Header Functions
//1957 Config Functions
//2760 ROM Functions
//3951 RAM Functions
//4384 Eeprom Functions
//4574 NESmaker Flash Cart Functions

/******************************************
  Supported Mappers
 *****************************************/
// Supported Mapper Array (iNES Mapper #s)
// Format = {mapper,prglo,prghi,chrlo,chrhi,ramlo,ramhi}
static const byte PROGMEM mapsize [] = {
  0, 0, 1, 0, 1, 0, 2, // nrom                                                [sram r/w]
  1, 1, 5, 0, 5, 0, 3, // mmc1                                                [sram r/w]
  2, 3, 4, 0, 0, 0, 0, // uxrom
  3, 0, 1, 0, 3, 0, 0, // cnrom
  4, 1, 5, 0, 6, 0, 1, // mmc3/mmc6                                           [sram/prgram r/w]
  5, 3, 5, 5, 7, 0, 3, // mmc5                                                [sram r/w]
  7, 2, 4, 0, 0, 0, 0, // axrom
  9, 3, 3, 5, 5, 0, 0, // mmc2 (punch out)
  10, 3, 4, 4, 5, 1, 1, // mmc4                                               [sram r/w]
  13, 1, 1, 0, 0, 0, 0, // cprom (videomation)
  16, 3, 4, 5, 6, 0, 1, // bandai x24c02                                      [eep r/w]
  18, 3, 4, 5, 6, 0, 1, // jaleco ss8806                                      [sram r/w]
  19, 3, 4, 5, 6, 0, 1, // namco 106/163                                      [sram/prgram r/w]
  21, 4, 4, 5, 6, 0, 1, // vrc4a/vrc4c                                        [sram r/w]
  22, 3, 3, 5, 5, 0, 0, // vrc2a
  23, 3, 3, 5, 6, 0, 0, // vrc2b/vrc4e
  24, 4, 4, 5, 5, 0, 0, // vrc6a (akumajou densetsu)
  25, 3, 4, 5, 6, 0, 1, // vrc2c/vrc4b/vrc4d                                  [sram r/w]
  26, 4, 4, 5, 6, 1, 1, // vrc6b                                              [sram r/w]
  30, 4, 5, 0, 0, 0, 0, // unrom 512 (NESmaker) [UNLICENSED]
  32, 3, 4, 5, 5, 0, 0, // irem g-101
  33, 3, 4, 5, 6, 0, 0, // taito tc0190
  34, 3, 3, 0, 0, 0, 0, // bnrom [nina-1 NOT SUPPORTED]
  37, 4, 4, 6, 6, 0, 0, // (super mario bros + tetris + world cup)
  45, 3, 6, 0, 8, 0, 0, // ga23c asic multicart [UNLICENSED]
  47, 4, 4, 6, 6, 0, 0, // (super spike vball + world cup)
  48, 3, 4, 6, 6, 0, 0, // taito tc0690
  64, 2, 3, 4, 5, 0, 0, // tengen rambo-1 [UNLICENSED]
  65, 3, 4, 5, 6, 0, 0, // irem h-3001
  66, 2, 3, 2, 3, 0, 0, // gxrom/mhrom
  67, 3, 3, 5, 5, 0, 0, // sunsoft 3
  68, 3, 3, 5, 6, 0, 1, // sunsoft 4                                          [sram r/w]
  69, 3, 4, 5, 6, 0, 1, // sunsoft fme-7/5a/5b                                [sram r/w]
  70, 3, 3, 5, 5, 0, 0, // bandai
  71, 2, 4, 0, 0, 0, 0, // camerica/codemasters [UNLICENSED]
  72, 3, 3, 5, 5, 0, 0, // jaleco jf-17
  73, 3, 3, 0, 0, 0, 0, // vrc3 (salamander)
  75, 3, 3, 5, 5, 0, 0, // vrc1
  76, 3, 3, 5, 5, 0, 0, // namco 109 variant (megami tensei: digital devil story)
  77, 3, 3, 3, 3, 0, 0, // (napoleon senki)
  78, 3, 3, 5, 5, 0, 0, // irem 74hc161/32
  80, 3, 3, 5, 6, 0, 1, // taito x1-005                                       [prgram r/w]
  82, 3, 3, 5, 6, 0, 1, // taito x1-017                                       [prgram r/w]
  85, 3, 5, 0, 5, 0, 1, // vrc7                                               [sram r/w]
  86, 3, 3, 4, 4, 0, 0, // jaleco jf-13 (moero pro yakyuu)
  87, 0, 1, 2, 3, 0, 0,
  88, 3, 3, 5, 5, 0, 0, // namco (dxrom variant)
  89, 3, 3, 5, 5, 0, 0, // sunsoft 2 variant (tenka no goikenban: mito koumon)
  92, 4, 4, 5, 5, 0, 0, // jaleco jf-19/jf-21
  93, 3, 3, 0, 0, 0, 0, // sunsoft 2
  94, 3, 3, 0, 0, 0, 0, // hvc-un1rom (senjou no ookami)
  95, 3, 3, 3, 3, 0, 0, // namcot-3425 (dragon buster)
  96, 3, 3, 0, 0, 0, 0, // (oeka kids)
  97, 4, 4, 0, 0, 0, 0, // irem tam-s1 (kaiketsu yanchamaru)
  105, 4, 4, 0, 0, 0, 0, // (nintendo world Championships 1990) [UNTESTED]
  118, 3, 4, 5, 5, 0, 1, // txsrom/mmc3                                       [sram r/w]
  119, 3, 3, 4, 4, 0, 0, // tqrom/mmc3
  140, 3, 3, 3, 5, 0, 0, // jaleco jf-11/jf-14
  152, 2, 3, 5, 5, 0, 0,
  153, 5, 5, 0, 0, 1, 1, // (famicom jump ii)                                 [sram r/w]
  154, 3, 3, 5, 5, 0, 0, // namcot-3453 (devil man)
  155, 3, 3, 3, 5, 0, 1, // mmc1 variant                                      [sram r/w]
  158, 3, 3, 5, 5, 0, 0, // tengen rambo-1 variant (alien syndrome (u)) [UNLICENSED]
  159, 3, 4, 5, 6, 1, 1, // bandai x24c01                                     [eep r/w]
  180, 3, 3, 0, 0, 0, 0, // unrom variant (crazy climber)
  184, 1, 1, 2, 3, 0, 0, // sunsoft 1
  185, 0, 1, 1, 1, 0, 0, // cnrom lockout
  206, 1, 3, 2, 4, 0, 0, // dxrom
  207, 4, 4, 5, 5, 0, 0, // taito x1-005 variant (fudou myouou den)
  210, 3, 5, 5, 6, 0, 0, // namco 175/340
};

/******************************************
  Defines
 *****************************************/
#define ROMSEL_HI PORTF |= (1<<1)
#define ROMSEL_LOW PORTF &= ~(1<<1)
#define PHI2_HI PORTF |= (1<<0)
#define PHI2_LOW PORTF &= ~(1<<0)
#define PRG_READ PORTF |= (1<<7)
#define PRG_WRITE PORTF &= ~(1<<7)
#define CHR_READ_HI PORTF |= (1<<5)
#define CHR_READ_LOW PORTF &= ~(1<<5)
#define CHR_WRITE_HI PORTF |= (1<<2)
#define CHR_WRITE_LOW PORTF &= ~(1<<2)

// RGB LED COMMON ANODE
#define LED_RED_OFF setColor_RGB(0, 0, 0)
#define LED_RED_ON setColor_RGB(255, 0, 0)
#define LED_GREEN_OFF setColor_RGB(0, 0, 0)
#define LED_GREEN_ON setColor_RGB(0, 255, 0)
#define LED_BLUE_OFF setColor_RGB(0, 0, 0)
#define LED_BLUE_ON setColor_RGB(0, 0, 255)

#define MODE_READ { PORTK = 0xFF; DDRK = 0; }
#define MODE_WRITE DDRK = 0xFF

#define press 1
#define doubleclick 2
#define hold 3
#define longhold 4

/******************************************
  Variables
*****************************************/
// Mapper
byte mapcount = (sizeof(mapsize) / sizeof(mapsize[0])) / 7;
boolean mapfound = false;
byte mapselect;

int PRG[] = {16, 32, 64, 128, 256, 512, 1024};
byte prglo = 0; // Lowest Entry
byte prghi = 6; // Highest Entry

int CHR[] = {0, 8, 16, 32, 64, 128, 256, 512, 1024};
byte chrlo = 0; // Lowest Entry
byte chrhi = 8; // Highest Entry

byte RAM[] = {0, 8, 16, 32};
byte ramlo = 0; // Lowest Entry
byte ramhi = 3; // Highest Entry

int banks;
int prg;
int chr;
byte ram;
boolean vrc4e = false;
byte prgchk0;
byte prgchk1;
boolean mmc6 = false;
byte prgchk2;
byte prgchk3;
int eepsize;
byte bytecheck;
byte firstbyte;
boolean flashfound = false; // NESmaker 39SF040 Flash Cart

// Files
char fileCount[3];

#ifndef no-intro
FsFile nesFile;
uint32_t prg_crc32;
uint32_t chr_crc32;
char filePRG[] = "PRG.bin";
char fileCHR[] = "CHR.bin";
char fileNES[] = "CART.nes";
char fileBIN[] = "CART.bin";
#endif

// Cartridge Config
byte mapper;
byte newmapper;
byte prgsize;
byte newprgsize;
byte chrsize;
byte newchrsize;
byte ramsize;
byte newramsize;

// Button
int b = 0;

/******************************************
  Menus
*****************************************/
// NES start menu
static const char nesMenuItem1[] PROGMEM = "Change Mapper";
static const char nesMenuItem2[] PROGMEM = "Read iNES Rom";
static const char nesMenuItem3[] PROGMEM = "Read PRG/CHR";
static const char nesMenuItem4[] PROGMEM = "Read Sram";
static const char nesMenuItem5[] PROGMEM = "Write Sram";
static const char nesMenuItem6[] PROGMEM = "Flash NESMaker";
static const char nesMenuItem7[] PROGMEM = "Reset";
static const char* const menuOptionsNES[] PROGMEM = {nesMenuItem1, nesMenuItem2, nesMenuItem3, nesMenuItem4, nesMenuItem5, nesMenuItem6, nesMenuItem7};

// NES chips menu
#ifndef no-intro
static const char  nesChipsMenuItem1[] PROGMEM = "Read PRG & CHR";
#else
static const char  nesChipsMenuItem1[] PROGMEM = "Combined PRG+CHR";
#endif
static const char  nesChipsMenuItem2[] PROGMEM = "Read only PRG";
static const char  nesChipsMenuItem3[] PROGMEM = "Read only CHR";
static const char  nesChipsMenuItem4[] PROGMEM = "Back";
static const char* const menuOptionsNESChips[] PROGMEM = {nesChipsMenuItem1, nesChipsMenuItem2, nesChipsMenuItem3, nesChipsMenuItem4};

// NES start menu
void nesMenu() {
  // create menu with title "NES CART READER" and 7 options to choose from
  convertPgm(menuOptionsNES, 7);
  unsigned char answer = question_box(F("NES CART READER"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (answer) {
    // Change Mapper
    case 0:
      setMapper();
      checkMapperSize();
      setPRGSize();
      setCHRSize();
      setRAMSize();
      checkStatus_NES();
      break;

    // Read Rom
    case 1:
#ifndef no-intro
      CartStart();
      readPRG(false);
      delay(2000);
      readCHR(false);
      delay(2000);
      outputNES();
      delay(2000);
      readRAM();
      delay(2000);
      resetROM();
      CartFinish();
#else
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readRom_NES();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
#ifdef global_log
      save_log();
#endif
      display_Update();
      wait();
#endif
      break;

    // Read single chip
    case 2:
      nesChipMenu();
      break;

    // Read RAM
    case 3:
      CreateROMFolderInSD();
      readRAM();
      resetROM();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    // Write RAM
    case 4:
      writeRAM();
      resetROM();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    // Write FLASH
    case 5:
      if (mapper == 30) {
        writeFLASH();
        resetROM();
      }
      else {
        display_Clear();
        println_Msg(F("Error:"));
        println_Msg(F("Can't write to this cartridge"));
        println_Msg(F(""));
        println_Msg(F("Press Button..."));
        display_Update();
      }
      wait();
      break;

    // Reset
    case 6:
      resetArduino();
      break;
  }
}

void nesChipMenu() {
  // create menu with title "Select NES Chip" and 4 options to choose from
  convertPgm(menuOptionsNESChips, 4);
  unsigned char answer = question_box(F("Select NES Chip"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (answer) {
    // Read combined PRG/CHR
    case 0:
#ifndef no-intro
      CreateROMFolderInSD();
      readPRG(false);
      resetROM();

      CreateROMFolderInSD();
      readCHR(false);
      resetROM();

      println_Msg(F(""));
      println_Msg(F("Press Button..."));
#else
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readRaw_NES();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
#ifdef global_log
      save_log();
#endif
#endif
      display_Update();
      wait();
      break;

    // Read PRG
    case 1:
      CreateROMFolderInSD();
      readPRG(false);
      resetROM();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    // Read CHR
    case 2:
      CreateROMFolderInSD();
      readCHR(false);
      resetROM();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    // Return to Main Menu
    case 3:
      nesMenu();
      wait();
      break;
  }
}

/******************************************
   Setup
 *****************************************/
void setup_NES() {
  // CPU R/W, IRQ, PPU /RD, PPU /A13, CIRAM /CE, PPU /WR, /ROMSEL, PHI2
  DDRF = 0b10110111;
  // CPU R/W, IRQ, PPU /RD, PPU /A13, CIRAM /CE, PPU /WR, /ROMSEL, PHI2
  PORTF = 0b11111111;

  // A0-A7 to Output
  DDRL = 0xFF;
  // A8-A14 to Output
  DDRA = 0xFF;

  // Set CIRAM A10 to Input
  DDRC &= ~(1 << 2);
  // Activate Internal Pullup Resistors
  PORTC |= (1 << 2);

  // Set D0-D7 to Input
  PORTK = 0xFF;
  DDRK = 0;

  set_address(0);
  LED_RED_OFF;
  LED_GREEN_OFF;
  LED_BLUE_OFF;
}

/******************************************
   Get Mapping from no-intro SD database
 *****************************************/
#ifdef no-intro
// no clue (taken from fceux)
uint32_t uppow2(uint32_t n) {
  int x;

  for (x = 31; x >= 0; x--)
    if (n & (1u << x))
    {
      if ((1u << x) != n)
        return (1u << (x + 1));
      break;
    }
  return n;
}

boolean getMapping() {
  display_Clear();
  println_Msg(F("Searching database"));
  display_Update();

  // Read first 512 bytes of prg rom
  for (int x = 0; x < 512; x++) {
    sdBuffer[x] = read_prg_byte(0x8000 + x);
  }

  // Calculate CRC32
  uint32_t oldcrc32 = 0xFFFFFFFF;
  for (int c = 0; c < 512; c++) {
    oldcrc32 = updateCRC(sdBuffer[c], oldcrc32);
  }
  char crcStr[9];
  char tempStr2[2];
  char iNES_STR[33];
  sprintf(crcStr, "%08lX", ~oldcrc32);

  print_Msg(F("for "));
  print_Msg(crcStr);
  println_Msg(F("..."));
  display_Update();

  // Filter out 0xFF checksum
  if (strcmp(crcStr, "BD7BC39F") == 0) {
    delay(500);
    println_Msg(F(""));
    println_Msg(F("No data found at 0x8000"));
    println_Msg(F("Using manual selection"));
    display_Update();
    delay(1000);
    romName[0] = 'C';
    romName[1] = 'A';
    romName[2] = 'R';
    romName[3] = 'T';
    return 0;
  }
  else {
    //Search for CRC32 in file
    char gamename[100];
    char crc_search[9];

    //go to root
    sd.chdir();
    if (myFile.open("nes.txt", O_READ)) {
      //Search for same CRC in list
      while (myFile.available()) {
        // Read game name
        get_line(gamename, &myFile, 96);

        // Read CRC32 checksum
        sprintf(checksumStr, "%c", myFile.read());
        for (byte i = 0; i < 7; i++) {
          sprintf(tempStr2, "%c", myFile.read());
          strcat(checksumStr, tempStr2);
        }

        // Skip over semicolon
        myFile.seekSet(myFile.curPosition() + 1);

        // Read CRC32 of first 512 bytes
        sprintf(crc_search, "%c", myFile.read());
        for (byte i = 0; i < 7; i++) {
          sprintf(tempStr2, "%c", myFile.read());
          strcat(crc_search, tempStr2);
        }

        // Skip over semicolon
        myFile.seekSet(myFile.curPosition() + 1);

        // Read iNES header
        get_line(iNES_STR, &myFile, 33);

        //Skip every 3rd line
        skip_line(&myFile);

        //if checksum search successful set mapper and end search
        if (strcmp(crc_search, crcStr) == 0) {

          // Rewind to start of entry
          for (byte count_newline = 0; count_newline < 4; count_newline++) {
            while (1) {
              if (myFile.curPosition() == 0) {
                break;
              }
              else if (myFile.peek() == '\n') {
                myFile.seekSet(myFile.curPosition() - 1);
                break;
              }
              else {
                myFile.seekSet(myFile.curPosition() - 1);
              }
            }
          }
          if (myFile.curPosition() != 0)
            myFile.seekSet(myFile.curPosition() + 2);


          // Display database
          while (myFile.available()) {
            display_Clear();

            // Read game name
#if defined(enable_OLED)
            get_line(gamename, &myFile, 42);
#else
            get_line(gamename, &myFile, 96);
#endif

            // Read CRC32 checksum
            sprintf(checksumStr, "%c", myFile.read());
            for (byte i = 0; i < 7; i++) {
              sprintf(tempStr2, "%c", myFile.read());
              strcat(checksumStr, tempStr2);
            }

            // Skip over semicolon
            myFile.seekSet(myFile.curPosition() + 1);

            // Read CRC32 of first 512 bytes
            sprintf(crc_search, "%c", myFile.read());
            for (byte i = 0; i < 7; i++) {
              sprintf(tempStr2, "%c", myFile.read());
              strcat(crc_search, tempStr2);
            }

            // Skip over semicolon
            myFile.seekSet(myFile.curPosition() + 1);

            // Read iNES header
            get_line(iNES_STR, &myFile, 33);

            // Skip every 3rd line
            skip_line(&myFile);

            // Convert "4E4553" to (0x4E, 0x45, 0x53)
            byte iNES_BUF[2];
            for (byte j = 0; j < 16; j++) {
              sscanf(iNES_STR + j * 2, "%2X", iNES_BUF);
              iNES_HEADER[j] = iNES_BUF[0];
            }

            // Convert iNES garbage to useful info (thx to fceux)
            mapper = (iNES_HEADER[6] >> 4);
            mapper |= (iNES_HEADER[7] & 0xF0);
            mapper |= ((iNES_HEADER[8] & 0x0F) << 8);

            // PRG size
            if ((iNES_HEADER[9] & 0x0F) != 0x0F) {
              // simple notation
              prgsize = (iNES_HEADER[4] | ((iNES_HEADER[9] & 0x0F) << 8)); //*16
            }
            else {
              // exponent-multiplier notation
              prgsize = (((1 << (iNES_HEADER[4] >> 2)) * ((iNES_HEADER[4] & 0b11) * 2 + 1)) >> 14); //*16
            }
            if (prgsize != 0)
              prgsize = (int(log(prgsize) / log(2)));

            // CHR size
            if ((iNES_HEADER[9] & 0xF0) != 0xF0) {
              // simple notation
              chrsize = (uppow2(iNES_HEADER[5] | ((iNES_HEADER[9] & 0xF0) << 4))) * 2; //*4
            }
            else {
              chrsize = (((1 << (iNES_HEADER[5] >> 2)) * ((iNES_HEADER[5] & 0b11) * 2 + 1)) >> 13) * 2; //*4
            }
            if (chrsize != 0)
              chrsize = (int(log(chrsize) / log(2)));

            // RAM size
            ramsize = ((iNES_HEADER[10] & 0xF0) ? (64 << ((iNES_HEADER[10] & 0xF0) >> 4)) : 0) / 4096; //*4
            if (ramsize != 0)
              ramsize = (int(log(ramsize) / log(2)));

            prg = (int_pow(2, prgsize)) * 16;
            if (chrsize == 0)
              chr = 0; // 0K
            else
              chr = (int_pow(2, chrsize)) * 4;
            if (ramsize == 0)
              ram = 0; // 0K
            else if (mapper == 82)
              ram = 5; // 5K
            else
              ram = (int_pow(2, ramsize)) * 4;

            // Mapper Variants
            // Identify variant for use across multiple functions
            if (mapper == 4) { // Check for MMC6/MMC3
              checkMMC6();
              if (mmc6)
                ram = 1; // 1K
            }

            println_Msg(gamename);
            print_Msg(F("MAPPER:   "));
            println_Msg(mapper);
            print_Msg(F("PRG SIZE: "));
            print_Msg(prg);
            println_Msg(F("K"));
            print_Msg(F("CHR SIZE: "));
            print_Msg(chr);
            println_Msg(F("K"));
            print_Msg(F("RAM SIZE: "));
            if (mapper == 0) {
              print_Msg(ram / 4);
              println_Msg(F("K"));
            }
            else if ((mapper == 16) || (mapper == 80) || (mapper == 159)) {
              if (mapper == 16)
                print_Msg(ram * 32);
              else
                print_Msg(ram * 16);
              println_Msg(F("B"));
            }
            else if (mapper == 19) {
              if (ramsize == 2)
                println_Msg(F("128B"));
              else {
                print_Msg(ram);
                println_Msg(F("K"));
              }
            }
            else {
              print_Msg(ram);
              println_Msg(F("K"));
            }
#if defined(enable_OLED)
            println_Msg(F("Press left to Change"));
            println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
            println_Msg(F("Rotate to Change"));
            println_Msg(F("Press to Select"));
#elif defined(SERIAL_MONITOR)
            println_Msg(F("U/D to Change"));
            println_Msg(F("Space to Select"));
#endif
            display_Update();

            int b = 0;
            while (1) {
              // Check button input
              b = checkButton();

              // Next
              if (b == 1) {
                break;
              }

              // Previous
              else if (b == 2) {
                for (byte count_newline = 0; count_newline < 7; count_newline++) {
                  while (1) {
                    if (myFile.curPosition() == 0) {
                      break;
                    }
                    else if (myFile.peek() == '\n') {
                      myFile.seekSet(myFile.curPosition() - 1);
                      break;
                    }
                    else {
                      myFile.seekSet(myFile.curPosition() - 1);
                    }
                  }
                }
                if (myFile.curPosition() != 0)
                  myFile.seekSet(myFile.curPosition() + 2);
                break;
              }

              // Selection
              else if (b == 3) {
                // Get name
                byte myLength = 0;
                for (unsigned int i = 0; i < 20; i++) {
                  // Stop at first "(" to remove "(Country)"
                  if (char(gamename[i]) == 40) {
                    break;
                  }
                  if (((char(gamename[i]) >= 48 && char(gamename[i]) <= 57) || (char(gamename[i]) >= 65 && char(gamename[i]) <= 90) || (char(gamename[i]) >= 97 && char(gamename[i]) <= 122)) && (myLength < 15)) {
                    romName[myLength] = char(gamename[i]);
                    myLength++;
                  }
                }

                // If name consists out of all japanese characters use CART as name
                if (myLength == 0) {
                  romName[0] = 'C';
                  romName[1] = 'A';
                  romName[2] = 'R';
                  romName[3] = 'T';
                }

                // Save Mapper
                EEPROM_writeAnything(7, mapper);
                EEPROM_writeAnything(8, prgsize);
                EEPROM_writeAnything(9, chrsize);
                EEPROM_writeAnything(10, ramsize);
                myFile.close();
                return 1;
                break;
              }
            }
          }
        }
      }
      // File searched until end but nothing found
      if (strcmp(crc_search, crcStr) != 0) {
        println_Msg(F(""));
        println_Msg(F("CRC not found in database"));
        println_Msg(F("Using manual selection"));
        display_Update();
        delay(1000);
        romName[0] = 'C';
        romName[1] = 'A';
        romName[2] = 'R';
        romName[3] = 'T';
        return 0;
      }
    }
    else {
      println_Msg(F("Database file not found"));
      return 0;
    }
  }
}

void selectMapping() {
  char gamename[100];
  char tempStr2[2];
  char iNES_STR[33];
  char crc_search[9];

  //go to root
  sd.chdir();

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("nes.txt", O_READ)) {
    // Skip ahead to selected starting letter
    if ((myLetter > 0) && (myLetter <= 26)) {
      while (myFile.available()) {
        // Read current name
#if defined(enable_OLED)
        get_line(gamename, &myFile, 42);
#else
        get_line(gamename, &myFile, 96);
#endif

        // Compare selected letter with first letter of current name until match
        while (gamename[0] != 64 + myLetter) {
          skip_line(&myFile);
          skip_line(&myFile);
#if defined(enable_OLED)
          get_line(gamename, &myFile, 42);
#else
          get_line(gamename, &myFile, 96);
#endif
        }
        break;
      }

      // Rewind one line
      for (byte count_newline = 0; count_newline < 2; count_newline++) {
        while (1) {
          if (myFile.curPosition() == 0) {
            break;
          }
          else if (myFile.peek() == '\n') {
            myFile.seekSet(myFile.curPosition() - 1);
            break;
          }
          else {
            myFile.seekSet(myFile.curPosition() - 1);
          }
        }
      }
      if (myFile.curPosition() != 0)
        myFile.seekSet(myFile.curPosition() + 2);
    }

    // Display database
    while (myFile.available()) {
      display_Clear();

      // Read game name
#if defined(enable_OLED)
      get_line(gamename, &myFile, 42);
#else
      get_line(gamename, &myFile, 96);
#endif

      // Read CRC32 checksum
      sprintf(checksumStr, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(checksumStr, tempStr2);
      }

      // Skip over semicolon
      myFile.seekSet(myFile.curPosition() + 1);

      // Read CRC32 of first 512 bytes
      sprintf(crc_search, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(crc_search, tempStr2);
      }

      // Skip over semicolon
      myFile.seekSet(myFile.curPosition() + 1);

      // Read iNES header
      get_line(iNES_STR, &myFile, 33);

      // Skip every 3rd line
      skip_line(&myFile);

      // Convert "4E4553" to (0x4E, 0x45, 0x53)
      byte iNES_BUF[2];
      for (byte j = 0; j < 16; j++) {
        sscanf(iNES_STR + j * 2, "%2X", iNES_BUF);
        iNES_HEADER[j] = iNES_BUF[0];
      }

      // Convert iNES garbage to useful info (thx to fceux)
      mapper = (iNES_HEADER[6] >> 4);
      mapper |= (iNES_HEADER[7] & 0xF0);
      mapper |= ((iNES_HEADER[8] & 0x0F) << 8);

      // PRG size
      if ((iNES_HEADER[9] & 0x0F) != 0x0F) {
        // simple notation
        prgsize = (iNES_HEADER[4] | ((iNES_HEADER[9] & 0x0F) << 8)); //*16
      }
      else {
        // exponent-multiplier notation
        prgsize = (((1 << (iNES_HEADER[4] >> 2)) * ((iNES_HEADER[4] & 0b11) * 2 + 1)) >> 14); //*16
      }
      if (prgsize != 0)
        prgsize = (int(log(prgsize) / log(2)));

      // CHR size
      if ((iNES_HEADER[9] & 0xF0) != 0xF0) {
        // simple notation
        chrsize = (uppow2(iNES_HEADER[5] | ((iNES_HEADER[9] & 0xF0) << 4))) * 2; //*4
      }
      else {
        chrsize = (((1 << (iNES_HEADER[5] >> 2)) * ((iNES_HEADER[5] & 0b11) * 2 + 1)) >> 13) * 2; //*4
      }
      if (chrsize != 0)
        chrsize = (int(log(chrsize) / log(2)));

      // RAM size
      ramsize = ((iNES_HEADER[10] & 0xF0) ? (64 << ((iNES_HEADER[10] & 0xF0) >> 4)) : 0) / 4096; //*4
      if (ramsize != 0)
        ramsize = (int(log(ramsize) / log(2)));

      prg = (int_pow(2, prgsize)) * 16;
      if (chrsize == 0)
        chr = 0; // 0K
      else
        chr = (int_pow(2, chrsize)) * 4;
      if (ramsize == 0)
        ram = 0; // 0K
      else if (mapper == 82)
        ram = 5; // 5K
      else
        ram = (int_pow(2, ramsize)) * 4;

      // Mapper Variants
      // Identify variant for use across multiple functions
      if (mapper == 4) { // Check for MMC6/MMC3
        checkMMC6();
        if (mmc6)
          ram = 1; // 1K
      }

      println_Msg(gamename);
      print_Msg(F("MAPPER:   "));
      println_Msg(mapper);
      print_Msg(F("PRG SIZE: "));
      print_Msg(prg);
      println_Msg(F("K"));
      print_Msg(F("CHR SIZE: "));
      print_Msg(chr);
      println_Msg(F("K"));
      print_Msg(F("RAM SIZE: "));
      if (mapper == 0) {
        print_Msg(ram / 4);
        println_Msg(F("K"));
      }
      else if ((mapper == 16) || (mapper == 80) || (mapper == 159)) {
        if (mapper == 16)
          print_Msg(ram * 32);
        else
          print_Msg(ram * 16);
        println_Msg(F("B"));
      }
      else if (mapper == 19) {
        if (ramsize == 2)
          println_Msg(F("128B"));
        else {
          print_Msg(ram);
          println_Msg(F("K"));
        }
      }
      else {
        print_Msg(ram);
        println_Msg(F("K"));
      }
#if defined(enable_OLED)
      println_Msg(F("Press left to Change"));
      println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
      println_Msg(F("Rotate to Change"));
      println_Msg(F("Press to Select"));
#elif defined(SERIAL_MONITOR)
      println_Msg(F("U/D to Change"));
      println_Msg(F("Space to Select"));
#endif
      display_Update();

      int b = 0;
      while (1) {
        // Check button input
        b = checkButton();

        // Next
        if (b == 1) {
          break;
        }

        // Previous
        else if (b == 2) {
          for (byte count_newline = 0; count_newline < 7; count_newline++) {
            while (1) {
              if (myFile.curPosition() == 0) {
                break;
              }
              else if (myFile.peek() == '\n') {
                myFile.seekSet(myFile.curPosition() - 1);
                break;
              }
              else {
                myFile.seekSet(myFile.curPosition() - 1);
              }
            }
          }
          if (myFile.curPosition() != 0)
            myFile.seekSet(myFile.curPosition() + 2);
          break;
        }

        // Selection
        else if (b == 3) {
          // Get name
          byte myLength = 0;
          for (unsigned int i = 0; i < 20; i++) {
            // Stop at first "(" to remove "(Country)"
            if (char(gamename[i]) == 40) {
              break;
            }
            if (((char(gamename[i]) >= 48 && char(gamename[i]) <= 57) || (char(gamename[i]) >= 65 && char(gamename[i]) <= 90) || (char(gamename[i]) >= 97 && char(gamename[i]) <= 122)) && (myLength < 15)) {
              romName[myLength] = char(gamename[i]);
              myLength++;
            }
          }

          // If name consists out of all japanese characters use CART as name
          if (myLength == 0) {
            romName[0] = 'C';
            romName[1] = 'A';
            romName[2] = 'R';
            romName[3] = 'T';
          }

          // Save Mapper
          EEPROM_writeAnything(7, mapper);
          EEPROM_writeAnything(8, prgsize);
          EEPROM_writeAnything(9, chrsize);
          EEPROM_writeAnything(10, ramsize);
          myFile.close();
          break;
        }
      }
    }
  }
  else {
    print_Error(F("Database file not found"), true);
  }
}

void readRom_NES() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".nes");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "NES/ROM/%s/%d", romName, foldern);
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

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(16 + prgsize * 16  * 1024 + chrsize * 4 * 1024);
  draw_progressbar(0, totalProgressBar);

  //Write iNES header
  myFile.write(iNES_HEADER, 16);

  // update progress bar
  processedProgressBar += 16;
  draw_progressbar(processedProgressBar, totalProgressBar);

  //Write PRG
  readPRG(true);

  // update progress bar
  processedProgressBar += prgsize * 16 * 1024;
  draw_progressbar(processedProgressBar, totalProgressBar);

  //Write CHR
  readCHR(true);

  // update progress bar
  processedProgressBar += chrsize * 4 * 1024;
  draw_progressbar(processedProgressBar, totalProgressBar);

  // Close the file:
  myFile.close();

  // Compare CRC32 with database
  compareCRC("nes.txt", 0, 1, 16);
}

void readRaw_NES() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "NES/ROM/%s/%d", romName, foldern);
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

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(prgsize * 16  * 1024 + chrsize * 4 * 1024);
  draw_progressbar(0, totalProgressBar);

  //Write PRG
  readPRG(true);

  // update progress bar
  processedProgressBar += prgsize * 16 * 1024;
  draw_progressbar(processedProgressBar, totalProgressBar);

  //Write CHR
  readCHR(true);

  // update progress bar
  processedProgressBar += chrsize * 4 * 1024;
  draw_progressbar(processedProgressBar, totalProgressBar);

  // Close the file:
  myFile.close();

  // Compare CRC32 with database
  compareCRC("nes.txt", 0, 0, 0);
}
#endif

/******************************************
   Low Level Functions
 *****************************************/
static void phi2_init() {
  int i = 0x80;
  unsigned char h = PORTF |= (1 << 0);
  unsigned char l = PORTF &= ~(1 << 0);
  while (i != 0) {
    PORTL = l;
    PORTL = h;
    i--;
  }
}

static void set_address(unsigned int address) {
  unsigned char l = address & 0xFF;
  unsigned char h = address >> 8;
  PORTL = l;
  PORTA = h;

  // PPU /A13
  if ((address >> 13) & 1)
    PORTF &= ~(1 << 4);
  else
    PORTF |= 1 << 4;
}

static void set_romsel(unsigned int address) {
  if (address & 0x8000) {
    ROMSEL_LOW;
  } else {
    ROMSEL_HI;
  }
}

static unsigned char read_prg_byte(unsigned int address) {
  MODE_READ;
  PRG_READ;
  set_address(address);
  PHI2_HI;
  set_romsel(address);
  _delay_us(1);
  return PINK;
}

static unsigned char read_chr_byte(unsigned int address) {
  MODE_READ;
  PHI2_HI;
  ROMSEL_HI;
  set_address(address);
  CHR_READ_LOW;
  _delay_us(1);
  uint8_t result = PINK;
  CHR_READ_HI;
  return result;
}

static void write_prg_byte(unsigned int address, uint8_t data) {
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PRG_WRITE;
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  //  _delay_us(1);
  PHI2_HI;
  //_delay_us(10);
  set_romsel(address); // ROMSEL is low if need, PHI2 high
  _delay_us(1); // WRITING
  //_delay_ms(1); // WRITING
  // PHI2 low, ROMSEL high
  PHI2_LOW;
  _delay_us(1);
  ROMSEL_HI;
  // Back to read mode
  //  _delay_us(1);
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  //  _delay_us(1);
  PHI2_HI;
  //  _delay_us(1);
}

static void write_chr_byte(unsigned int address, uint8_t data) {
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  //_delay_us(10);
  CHR_WRITE_LOW;
  _delay_us(1); // WRITING
  //_delay_ms(1); // WRITING
  CHR_WRITE_HI;
  //_delay_us(1);
  MODE_READ;
  set_address(0);
  PHI2_HI;
  //_delay_us(1);
}

static void write_prg(unsigned int address, unsigned int len, uint8_t* data) {
  LED_RED_ON;
  while (len > 0) {
    write_prg_byte(address, *data);
    address++;
    len--;
    data++;
  }
  //_delay_ms(1);
  LED_RED_OFF;
}

static void write_chr(unsigned int address, unsigned int len, uint8_t* data) {
  LED_RED_ON;
  while (len > 0) {
    write_chr_byte(address, *data);
    address++;
    len--;
    data++;
  }
  //_delay_ms(1);
  LED_RED_OFF;
}

static void reset_phi2() {
  LED_RED_ON;
  LED_GREEN_ON;
  PHI2_LOW;
  ROMSEL_HI;
  _delay_ms(100);
  PHI2_HI;
  LED_RED_OFF;
  LED_GREEN_OFF;
}

void resetROM() {
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
}

void write_mmc1_byte(unsigned int address, uint8_t data) { // write loop for 5 bit register
  if (address >= 0xE000) {
    for (int i = 0; i < 5; i++) {
      write_reg_byte(address, data >> i); // shift 1 bit into temp register [WRITE RAM SAFE]
    }
  }
  else {
    for (int j = 0; j < 5; j++) {
      write_prg_byte(address, data >> j); // shift 1 bit into temp register
    }
  }
}

// REFERENCE FOR REGISTER WRITE TO 0xE000/0xF000
// PORTF 7 = CPU R/W = 0
// PORTF 6 = /IRQ = 1
// PORTF 5 = PPU /RD = 1
// PORTF 4 = PPU /A13 = 1
// PORTF 3 = CIRAM /CE = 1
// PORTF 2 = PPU /WR = 1
// PORTF 1 = /ROMSEL
// PORTF 0 = PHI2 (M2)

// WRITE RAM SAFE TO REGISTERS 0xE000/0xF000
static void write_reg_byte(unsigned int address, uint8_t data) { // FIX FOR MMC1 RAM CORRUPTION
  PHI2_LOW;
  ROMSEL_HI; // A15 HI = E000
  MODE_WRITE;
  PRG_WRITE; // CPU R/W LO
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  // DIRECT PIN TO PREVENT RAM CORRUPTION
  // DIFFERENCE BETWEEN M2 LO AND ROMSEL HI MUST BE AROUND 33ns
  // IF TIME IS GREATER THAN 33ns THEN WRITES TO 0xE000/0xF000 WILL CORRUPT RAM AT 0x6000/0x7000
  PORTF = 0b01111101; // ROMSEL LO/M2 HI
  PORTF = 0b01111110; // ROMSEL HI/M2 LO
  _delay_us(1);
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

static void write_ram_byte(unsigned int address, uint8_t data) { // Mapper 19 (Namco 106/163) WRITE RAM SAFE ($E000-$FFFF)
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PRG_WRITE;
  PORTK = data;

  set_address(address); // PHI2 low, ROMSEL always HIGH
  PHI2_HI;
  ROMSEL_LOW; // SET /ROMSEL LOW OTHERWISE CORRUPTS RAM
  _delay_us(1); // WRITING
  // PHI2 low, ROMSEL high
  PHI2_LOW;
  _delay_us(1);
  ROMSEL_HI;
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

static void write_wram_byte(unsigned int address, uint8_t data) { // Mapper 5 (MMC5) RAM
  PHI2_LOW;
  ROMSEL_HI;
  set_address(address);
  PORTK = data;

  _delay_us(1);
  MODE_WRITE;
  PRG_WRITE;
  PHI2_HI;
  _delay_us(1); // WRITING
  PHI2_LOW;
  ROMSEL_HI;
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

int int_pow(int base, int exp) { // Power for int
  int result = 1;
  while (exp) {
    if (exp & 1)
      result *= base;
    exp /= 2;
    base *= base;
  }
  return result;
}

/******************************************
   CRC Functions
 *****************************************/
FsFile crcFile;
char tempCRC[9];

uint32_t crc32(FsFile & file, uint32_t &charcnt) {
  uint32_t oldcrc32 = 0xFFFFFFFF;
  charcnt = 0;
  while (file.available()) {
    crcFile.read(sdBuffer, 512);
    for (int x = 0; x < 512; x++) {
      uint8_t c = sdBuffer[x];
      charcnt++;
      oldcrc32 = updateCRC(c, oldcrc32);
    }
  }
  return ~oldcrc32;
}

uint32_t crc32EEP(FsFile & file, uint32_t &charcnt) {
  uint32_t oldcrc32 = 0xFFFFFFFF;
  charcnt = 0;
  while (file.available()) {
    crcFile.read(sdBuffer, 128);
    for (int x = 0; x < 128; x++) {
      uint8_t c = sdBuffer[x];
      charcnt++;
      oldcrc32 = updateCRC(c, oldcrc32);
    }
  }
  return ~oldcrc32;
}

void calcCRC(char* checkFile, unsigned long filesize, uint32_t* crcCopy, unsigned long offset) {
  uint32_t crc;
  crcFile = sd.open(checkFile);
  crcFile.seek(offset);
  if (filesize < 1024)
    crc = crc32EEP(crcFile, filesize);
  else
    crc = crc32(crcFile, filesize);
  crcFile.close();
  sprintf(tempCRC, "%08lX", crc);

  if (crcCopy != NULL) {
    *crcCopy = crc;
  }

  print_Msg(F("CRC: "));
  println_Msg(tempCRC);
  display_Update();
}

/******************************************
   File Functions
 *****************************************/
void CreateROMFolderInSD() {
  sd.chdir();
  sprintf(folder, "NES/ROM");
  sd.mkdir(folder, true);
  sd.chdir(folder);
}

void CreatePRGFileInSD() {
  strcpy(fileName, "PRG");
  strcat(fileName, ".bin");
  for (byte i = 0; i < 100; i++) {
    if (!sd.exists(fileName)) {
      myFile = sd.open(fileName, O_RDWR | O_CREAT);
      break;
    }
    sprintf(fileCount, "%02d", i);
    strcpy(fileName, "PRG.");
    strcat(fileName, fileCount);
    strcat(fileName, ".bin");
  }
  if (!myFile) {
    LED_RED_ON;

    display_Clear();
    println_Msg(F("PRG FILE FAILED!"));
    display_Update();
    print_Error(F("SD Error"), true);

    LED_RED_OFF;
  }
}

void CreateCHRFileInSD() {
  strcpy(fileName, "CHR");
  strcat(fileName, ".bin");
  for (byte i = 0; i < 100; i++) {
    if (!sd.exists(fileName)) {
      myFile = sd.open(fileName, O_RDWR | O_CREAT);
      break;
    }
    sprintf(fileCount, "%02d", i);
    strcpy(fileName, "CHR.");
    strcat(fileName, fileCount);
    strcat(fileName, ".bin");
  }
  if (!myFile) {
    LED_RED_ON;

    display_Clear();
    println_Msg(F("CHR FILE FAILED!"));
    display_Update();
    print_Error(F("SD Error"), true);

    LED_RED_OFF;
  }
}

void CreateRAMFileInSD() {
  strcpy(fileName, "RAM");
  strcat(fileName, ".bin");
  for (byte i = 0; i < 100; i++) {
    if (!sd.exists(fileName)) {
      myFile = sd.open(fileName, O_RDWR | O_CREAT);
      break;
    }
    sprintf(fileCount, "%02d", i);
    strcpy(fileName, "RAM.");
    strcat(fileName, fileCount);
    strcat(fileName, ".bin");
  }
  if (!myFile) {
    LED_RED_ON;

    display_Clear();
    println_Msg(F("RAM FILE FAILED!"));
    display_Update();
    print_Error(F("SD Error"), true);

    LED_RED_OFF;
  }
}

#ifndef no-intro
void CartStart() {
  sd.chdir();
  EEPROM_readAnything(0, foldern); // FOLDER #
  sprintf(folder, "NES/CART/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);
}

void CartFinish() {
  foldern += 1;
  EEPROM_writeAnything(0, foldern); // FOLDER #
  sd.chdir();
}
#endif

/******************************************
   NES 2.0 Header Functions
 *****************************************/
#ifndef no-intro
int32_t atoi32_signed(const char* input_string) {
  if (input_string == NULL) {
    return 0;
  }

  int int_sign = 1;
  int i = 0;

  if (input_string[0] == '-') {
    int_sign = -1;
    i = 1;
  }

  int32_t return_val = 0;

  while (input_string[i] != '\0') {
    if (input_string[i] >= '0' && input_string[i] <= '9') {
      return_val = (return_val * 10) + (input_string[i] - '0');
    } else if (input_string[i] != '\0') {
      return 0;
    }

    i++;
  }

  return_val = return_val * int_sign;

  return return_val;
}

uint32_t atoi32_unsigned(const char* input_string) {
  if (input_string == NULL) {
    return 0;
  }

  int i = 0;

  uint32_t return_val = 0;

  while (input_string[i] != '\0') {
    if (input_string[i] >= '0' && input_string[i] <= '9') {
      return_val = (return_val * 10) + (input_string[i] - '0');
    } else if (input_string[i] != '\0') {
      return 0;
    }

    i++;
  }

  return return_val;
}

void outputNES() {
  display_Clear();
  char* outputFile;
  unsigned long crcOffset = 0;
  uint32_t prg_size_bytes = 1024 * (uint32_t)prg;
  uint32_t chr_size_bytes = 1024 * (uint32_t)chr;
  int has_header = 0;

  unsigned char* nes_header_bytes = getNESHeaderForFileInfo(prg_size_bytes, chr_size_bytes, prg_crc32, chr_crc32);

  if (nes_header_bytes != NULL) {
    has_header = 1;
  }

  LED_RED_ON;
  LED_GREEN_ON;
  LED_BLUE_ON;
  if (!myFile.open(filePRG, FILE_READ)) {
    LED_GREEN_OFF;
    LED_BLUE_OFF;

    display_Clear();
    println_Msg(F("PRG FILE FAILED!"));
    display_Update();
    print_Error(F("SD Error"), true);

  }

  if (has_header) {
    outputFile = fileNES;
    crcOffset = 16;
  } else {
    outputFile = fileBIN;
  }

  if (!sd.exists(outputFile)) {
    nesFile = sd.open(outputFile, O_RDWR | O_CREAT);
  }
  if (!nesFile) {
    LED_GREEN_OFF;
    LED_BLUE_OFF;

    display_Clear();
    println_Msg(F("NES FILE FAILED!"));
    display_Update();
    print_Error(F("SD Error"), true);
  }

  if (has_header)
  {
    nesFile.write(nes_header_bytes, 16);
    free(nes_header_bytes);
    display_Clear();
    println_Msg(F("SET HEADER"));
    display_Update();
  }

  size_t n;
  while ((n = myFile.read(sdBuffer, sizeof(sdBuffer))) > 0) {
    nesFile.write(sdBuffer, n);
  }
  myFile.close();
  if (sd.exists(fileCHR)) {
    if (!myFile.open(fileCHR, FILE_READ)) {
      LED_GREEN_OFF;
      LED_BLUE_OFF;

      display_Clear();
      println_Msg(F("CHR FILE FAILED!"));
      display_Update();
      print_Error(F("SD Error"), true);

    }
    while ((n = myFile.read(sdBuffer, sizeof(sdBuffer))) > 0) {
      nesFile.write(sdBuffer, n);
    }
    myFile.close();
  }
  nesFile.flush();
  nesFile.close();

  display_Clear();
  if (has_header) {
    println_Msg(F("NES FILE OUTPUT!"));
  } else {
    println_Msg(F("BIN FILE OUTPUT!"));
  }
  println_Msg(F(""));
  display_Update();

  calcCRC(outputFile, (prg + chr) * 1024, NULL, crcOffset);
  LED_RED_OFF;
  LED_GREEN_OFF;
  LED_BLUE_OFF;
}

unsigned char* getNESHeaderForFileInfo(uint32_t prg_size, uint32_t chr_size, uint32_t prg_crc32, uint32_t chr_crc32) {
  if (prg_size == 0) {
    return NULL;
  }

  char* temp_line;
  unsigned char* nes20_header;
  int i;

  if (!myFile.open("/nes20db.txt", FILE_READ)) {
    return NULL;
  } else {
    display_Clear();
    println_Msg(F("SEARCHING DB"));
    display_Update();
  }

  temp_line = (char*)malloc(256 * sizeof(char));
  while (myFile.available()) {
    // We're reading fixed-length lines
    // padded with null characters
    myFile.read(temp_line, 256);

    uint32_t prg_size_db;
    uint32_t chr_size_db;
    uint32_t prg_crc32_db;
    uint32_t chr_crc32_db;

    // Match PRG and CHR sizes first, then
    // match PRG CRC32 and, if the CHR size
    // is greater than zero, the CHR CRC32
    // as well.
    prg_size_db = getPRGSizeFromDatabaseRow(temp_line);
    if (prg_size == prg_size_db) {
      chr_size_db = getCHRSizeFromDatabaseRow(temp_line);
      if (chr_size == chr_size_db) {
        prg_crc32_db = getPRGCRC32FromDatabaseRow(temp_line);
        if (prg_crc32 == prg_crc32_db) {
          if (chr_size == 0) {
            nes20_header = getNES20HeaderBytesFromDatabaseRow(temp_line);
            free(temp_line);
            myFile.close();
            return nes20_header;
          } else {
            chr_crc32_db = getCHRCRC32FromDatabaseRow(temp_line);
            if (chr_crc32 == chr_crc32_db) {
              nes20_header = getNES20HeaderBytesFromDatabaseRow(temp_line);
              free(temp_line);
              myFile.close();
              return nes20_header;
            }
          }
        }
      }
    }
  }

  free(temp_line);
  myFile.close();
  return NULL;
}

// IMPORTANT: The string returned from this function MUST
// be passed to free() when ready to be disposed of, in
// order to avoid a memory leak.
char* getDatabaseFieldFromRow(const char* dbstr, uint8_t fieldnum) {
  uint8_t field_start_pos = 0;
  uint8_t field_end_pos = 1;
  uint8_t current_field = 0;
  char* return_field;

  // Field order, beginning with field 0:
  // PRG Size, CHR Size, PRG CRC32, CHR CRC32, Game Title, NES 2.0 Header (as ASCII)
  //
  // Each entry is on its own line, with a field delimeter of ^^
  // I'm assuming that nothing will ever use ^^ in a game title, but it's possible
  // that could be wrong, in which case a different field delimeter would need
  // to be used, and the logic here updated.
  if (dbstr == NULL || fieldnum > 5) {
    return NULL;
  }

  if (dbstr[0] == 0 || dbstr[0] == '\n') {
    return NULL;
  }

  for (; field_end_pos < 255 && current_field < fieldnum; field_end_pos++) {
    if (field_start_pos < 254 && dbstr[field_start_pos] == '^' && dbstr[field_start_pos + 1] == '^') {
      current_field++;
      field_start_pos = field_end_pos;
      field_end_pos = field_start_pos + 1;
    }

    if (current_field < fieldnum && dbstr[field_end_pos - 1] == '^' && dbstr[field_end_pos] == '^' || dbstr[field_end_pos] == 0 || dbstr[field_end_pos] == '\n') {
      current_field++;
      field_start_pos = field_end_pos + 1;
      field_end_pos = field_start_pos + 1;
    }
  }

  field_end_pos = field_start_pos;

  while ((dbstr[field_end_pos - 1] != '^' || dbstr[field_end_pos] != '^') && dbstr[field_end_pos] != 0 && dbstr[field_end_pos] != '\n') {
    field_end_pos++;
  }

  if (dbstr[field_end_pos] == '^') {
    field_end_pos = field_end_pos - 2;
  } else {
    field_end_pos = field_end_pos - 1;
  }

  if ((field_end_pos - field_start_pos + 2) == 0) {
    return NULL;
  }

  return_field = (char*)malloc((field_end_pos - field_start_pos + 2) * sizeof(char));

  memcpy(return_field, &dbstr[field_start_pos], field_end_pos - field_start_pos + 1);

  return_field[(field_end_pos - field_start_pos) + 1] = 0;

  return return_field;
}

unsigned char getNibbleFromChar(char num) {
  char ret_char = num & 0x0F;
  if (num > '9') {
    ret_char += 9;
  }

  return ret_char;
}

unsigned char getByteFromChars(char msn, char lsn) {
  unsigned char return_char;
  return_char = (getNibbleFromChar(msn) << 4);
  return_char |= getNibbleFromChar(lsn);

  return return_char;
}

// IMPORTANT: The byte array returned from this function MUST
// be passed to free() when ready to be disposed of, in
// order to avoid a memory leak.
unsigned char* strToBytes(const char* bytestr) {
  uint8_t str_length;
  uint8_t byte_length;
  uint8_t str_idx;
  uint8_t byte_idx = 0;
  unsigned char* byte_arr;

  if (bytestr == NULL) {
    return NULL;
  }

  str_length = (uint8_t)strlen(bytestr);

  if (str_length % 2 != 0) {
    return NULL;
  }

  byte_length = str_length / 2;

  byte_arr = (unsigned char*)malloc(byte_length * sizeof(unsigned char));

  for (str_idx = 0; str_idx < str_length && bytestr[str_idx] != 0; str_idx = str_idx + 2) {
    if (!isxdigit(bytestr[str_idx]) || !isxdigit(bytestr[str_idx + 1])) {
      free(byte_arr);
      return NULL;
    }

    byte_arr[byte_idx] = getByteFromChars(bytestr[str_idx], bytestr[str_idx + 1]);
    byte_idx++;
  }

  return byte_arr;
}

uint32_t crc32FromBytes(const unsigned char* bytearr) {
  if (bytearr == NULL) {
    return 0;
  }

  return (uint32_t)(((uint32_t)bytearr[0] << 24) | ((uint32_t)bytearr[1] << 16) | ((uint32_t)bytearr[2] << 8) | (uint32_t)bytearr[3]);
}

uint32_t getPRGSizeFromDatabaseRow(const char* crctest) {
  char* prg_size_str = getDatabaseFieldFromRow(crctest, 0);
  if (prg_size_str == NULL) {
    return 0;
  }

  uint32_t return_size = atoi32_unsigned(prg_size_str);
  free(prg_size_str);

  return return_size;
}

uint32_t getCHRSizeFromDatabaseRow(const char* crctest) {
  char* chr_size_str = getDatabaseFieldFromRow(crctest, 1);
  if (chr_size_str == NULL) {
    return 0;
  }

  uint32_t return_size = atoi32_unsigned(chr_size_str);
  free(chr_size_str);

  return return_size;
}

uint32_t getPRGCRC32FromDatabaseRow(const char* crctest) {
  char* prg_crc32_str = getDatabaseFieldFromRow(crctest, 2);
  if (prg_crc32_str == NULL) {
    return 0;
  }

  unsigned char* prg_crc32_bytes = strToBytes(prg_crc32_str);
  free(prg_crc32_str);

  if (prg_crc32_bytes == NULL) {
    return 0;
  }

  uint32_t prg_crc32 = crc32FromBytes(prg_crc32_bytes);
  free(prg_crc32_bytes);

  return prg_crc32;
}

uint64_t getCHRCRC32FromDatabaseRow(const char* crctest) {
  char* chr_crc32_str = getDatabaseFieldFromRow(crctest, 3);
  if (chr_crc32_str == NULL) {
    return 0;
  }

  unsigned char* chr_crc32_bytes = strToBytes(chr_crc32_str);
  free(chr_crc32_str);

  if (chr_crc32_bytes == NULL) {
    return 0;
  }

  uint32_t chr_crc32 = crc32FromBytes(chr_crc32_bytes);
  free(chr_crc32_bytes);

  return chr_crc32;
}

// IMPORTANT: As with getDatabaseFieldFromRow(), the string
// returned from this function must be passed to free() after
// it's no longer needed in order to avoid a memory leak.
char* getGameTitleFromDatabaseRow(const char* crctest) {
  char* game_title_str = getDatabaseFieldFromRow(crctest, 4);

  return game_title_str;
}

// IMPORTANT: The byte array returned from this function MUST
// be passed to free() when ready to be disposed of, in
// order to avoid a memory leak.
unsigned char* getNES20HeaderBytesFromDatabaseRow(const char* crctest) {
  char* nes_header_str = getDatabaseFieldFromRow(crctest, 5);
  if (nes_header_str == NULL) {
    return NULL;
  }

  unsigned char* nes_header_bytes = strToBytes(nes_header_str);
  free(nes_header_str);

  if (nes_header_bytes == NULL) {
    return NULL;
  }

  return nes_header_bytes;
}
#endif

/******************************************
   Config Functions
 *****************************************/
void setMapper() {
  // OLED
#if defined(enable_OLED)
chooseMapper:
  // Read stored mapper
  EEPROM_readAnything(7, newmapper);
  if (newmapper > 220)
    newmapper = 0;
  // Split into digits
  byte hundreds = newmapper / 100;
  byte tens = newmapper / 10 - hundreds * 10;
  byte units = newmapper - hundreds * 100 - tens * 10;

  // Cycle through al 3 digits
  for (byte digit = 0; digit < 3; digit++) {
    while (1) {
      display_Clear();
      println_Msg(F("Select Mapper:"));
      display.setCursor(23, 20);
      println_Msg(hundreds);
      display.setCursor(43, 20);
      println_Msg(tens);
      display.setCursor(63, 20);
      println_Msg(units);
      println_Msg("");
      println_Msg(F("Press left to change"));
      println_Msg(F("Press right to select"));

      if (digit == 0) {
        display.drawLine(20, 30, 30, 30, WHITE);
        display.drawLine(40, 30, 50, 30, BLACK);
        display.drawLine(60, 30, 70, 30, BLACK);
      }
      else if (digit == 1) {
        display.drawLine(20, 30, 30, 30, BLACK);
        display.drawLine(40, 30, 50, 30, WHITE);
        display.drawLine(60, 30, 70, 30, BLACK);
      }
      else if (digit == 2) {
        display.drawLine(20, 30, 30, 30, BLACK);
        display.drawLine(40, 30, 50, 30, BLACK);
        display.drawLine(60, 30, 70, 30, WHITE);
      }

      /* Check Button
         1 click
         2 doubleClick
         3 hold
         4 longHold */
      int b = checkButton();

      if (b == 1) {
        if (digit == 0) {
          if (hundreds < 2)
            hundreds++;
          else
            hundreds = 0;
        }
        else if (digit == 1) {
          if (hundreds == 2) {
            if (tens < 1)
              tens++;
            else
              tens = 0;
          }
          else {
            if (tens < 9)
              tens++;
            else
              tens = 0;
          }
        }
        else if (digit == 2) {
          if (units < 9)
            units++;
          else
            units = 0;
        }
      }
      else if (b == 2) {
        if (digit == 0) {
          if (hundreds > 0)
            hundreds--;
          else
            hundreds = 2;
        }
        else if (digit == 1) {
          if (hundreds == 2) {
            if (tens > 0)
              tens--;
            else
              tens = 1;
          }
          else {
            if (tens > 0)
              tens--;
            else
              tens = 9;
          }
        }
        else if (digit == 2) {
          if (units > 0)
            units--;
          else
            units = 9;
        }
      }
      else if (b == 3) {
        break;
      }

      display.display();
    }
  }
  display_Clear();

  newmapper = hundreds * 100 + tens * 10 + units;

  // Check if valid
  boolean validMapper = 0;
  byte mapcount = (sizeof(mapsize) / sizeof(mapsize[0])) / 7;
  for (byte currMaplist = 0; currMaplist < mapcount; currMaplist++) {
    if (pgm_read_byte(mapsize + currMaplist * 7) == newmapper)
      validMapper = 1;
  }

  if (!validMapper) {
    errorLvl = 1;
    display.println(F("Mapper not supported"));
    display.display();
    wait();
    goto chooseMapper;
  }

  // LCD
#elif defined(enable_LCD)
  int i = 0;

  display_Clear();
  mapselect = pgm_read_byte(mapsize + i * 7);
  print_Msg(F("Mapper: "));
  println_Msg(mapselect);
  println_Msg(F(""));
  println_Msg(F("Rotate to change"));
  println_Msg(F("Press to select"));
  display_Update();

  while (1) {
    int b = checkButton();

    if (b == 2) { // Previous Mapper
      if (i == 0)
        i = mapcount - 1;
      else
        i--;

      display_Clear();
      mapselect = pgm_read_byte(mapsize + i * 7);
      print_Msg(F("Mapper: "));
      println_Msg(mapselect);
      println_Msg(F(""));
      println_Msg(F("Rotate to change"));
      println_Msg(F("Press to select"));
      display_Update();
    }

    else if (b == 1) { // Next Mapper
      if (i == (mapcount - 1))
        i = 0;
      else
        i++;

      display_Clear();
      mapselect = pgm_read_byte(mapsize + i * 7);
      print_Msg(F("Mapper: "));
      println_Msg(mapselect);
      println_Msg(F(""));
      println_Msg(F("Rotate to change"));
      println_Msg(F("Press to select"));
      display_Update();
    }

    else if (b == 3) { // Long Press - Execute
      newmapper = mapselect;
      break;
    }
  }

  display.setCursor(0, 56 + 8);
  print_Msg(F("MAPPER "));
  print_Msg(newmapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);

  // Serial Monitor
#elif defined(enable_serial)
setmapper:
  String newmap;
  mapfound = false;
  Serial.println(F("SUPPORTED MAPPERS:"));
  for (int i = 0; i < mapcount; i++) {
    int index = i * 7;
    mapselect = pgm_read_byte(mapsize + index);
    Serial.print("[");
    Serial.print(mapselect);
    Serial.print("]");
    if (i < mapcount - 1) {
      if ((i != 0) && ((i + 1) % 10 == 0))
        Serial.println(F(""));
      else
        Serial.print(F("\t"));
    }
    else
      Serial.println(F(""));
  }
  Serial.print(F("Enter Mapper: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newmapper = newmap.toInt();
  for (int i = 0; i < mapcount; i++) {
    int index = i * 7;
    mapselect = pgm_read_byte(mapsize + index);
    if (newmapper == mapselect)
      mapfound = true;
  }
  if (mapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(F(""));
    newmapper = 0;
    goto setmapper;
  }
#endif
  EEPROM_writeAnything(7, newmapper);
  mapper = newmapper;
}

void checkMapperSize() {
  for (int i = 0; i < mapcount; i++) {
    int index = i * 7;
    byte mapcheck = pgm_read_byte(mapsize + index);
    if (mapcheck == mapper) {
      prglo = pgm_read_byte(mapsize + index + 1);
      prghi = pgm_read_byte(mapsize + index + 2);
      chrlo = pgm_read_byte(mapsize + index + 3);
      chrhi = pgm_read_byte(mapsize + index + 4);
      ramlo = pgm_read_byte(mapsize + index + 5);
      ramhi = pgm_read_byte(mapsize + index + 6);
      break;
    }
  }
}

void setPRGSize() {
#if (defined(enable_LCD) || defined(enable_OLED))
  display_Clear();
  if (prglo == prghi)
    newprgsize = prglo;
  else {
    b = 0;
    int i = prglo;

    display_Clear();
    print_Msg(F("PRG Size: "));
    println_Msg(PRG[i]);
    println_Msg(F(""));
#if defined(enable_OLED)
    println_Msg(F("Press left to change"));
    println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
    println_Msg(F("Rotate to change"));
    println_Msg(F("Press to select"));
#endif
    display_Update();

    while (1) {
      b = checkButton();

      if (b == doubleclick) { // Previous
        if (i == prglo)
          i = prghi;
        else
          i--;

        display_Clear();
        print_Msg(F("PRG Size: "));
        println_Msg(PRG[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to change"));
        println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to change"));
        println_Msg(F("Press to select"));
#endif
        display_Update();
      }
      if (b == press) { // Next
        if (i == prghi)
          i = prglo;
        else
          i++;

        display_Clear();
        print_Msg(F("PRG Size: "));
        println_Msg(PRG[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to change"));
        println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to change"));
        println_Msg(F("Press to select"));
#endif
        display_Update();
      }
      if (b == hold) { // Long Press - Execute
        newprgsize = i;
        break;
      }
    }

    display.setCursor(0, 56); // Display selection at bottom
  }
  print_Msg(F("PRG SIZE "));
  print_Msg(PRG[newprgsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);

#elif defined(enable_serial)
  if (prglo == prghi)
    newprgsize = prglo;
  else {
setprg:
    String sizePRG;
    for (int i = 0; i < (prghi - prglo + 1); i++) {
      Serial.print(F("Select PRG Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(PRG[i + prglo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter PRG Size: "));
    while (Serial.available() == 0) {}
    sizePRG = Serial.readStringUntil('\n');
    Serial.println(sizePRG);
    newprgsize = sizePRG.toInt() + prglo;
    if (newprgsize > prghi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setprg;
    }
  }
  Serial.print(F("PRG Size = "));
  Serial.print(PRG[newprgsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newprgsize);
  prgsize = newprgsize;
}

void setCHRSize() {
#if (defined(enable_LCD) || defined(enable_OLED))
  display_Clear();
  if (chrlo == chrhi)
    newchrsize = chrlo;
  else {
    b = 0;
    int i = chrlo;

    display_Clear();
    print_Msg(F("CHR Size: "));
    println_Msg(CHR[i]);
    println_Msg(F(""));
#if defined(enable_OLED)
    println_Msg(F("Press left to change"));
    println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
    println_Msg(F("Rotate to change"));
    println_Msg(F("Press to select"));
#endif
    display_Update();

    while (1) {
      b = checkButton();

      if (b == doubleclick) { // Previous
        if (i == chrlo)
          i = chrhi;
        else
          i--;

        display_Clear();
        print_Msg(F("CHR Size: "));
        println_Msg(CHR[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to change"));
        println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to change"));
        println_Msg(F("Press to select"));
#endif
        display_Update();
      }

      if (b == press) { // Next
        if (i == chrhi)
          i = chrlo;
        else
          i++;

        display_Clear();
        print_Msg(F("CHR Size: "));
        println_Msg(CHR[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to change"));
        println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to change"));
        println_Msg(F("Press to select"));
#endif
        display_Update();
      }

      if (b == hold) { // Long Press - Execute
        newchrsize = i;
        break;
      }
    }
    display.setCursor(0, 56); // Display selection at bottom
  }
  print_Msg(F("CHR SIZE "));
  print_Msg(CHR[newchrsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);

#elif defined(enable_serial)
  if (chrlo == chrhi)
    newchrsize = chrlo;
  else {
setchr:
    String sizeCHR;
    for (int i = 0; i < (chrhi - chrlo + 1); i++) {
      Serial.print(F("Select CHR Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(CHR[i + chrlo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter CHR Size: "));
    while (Serial.available() == 0) {}
    sizeCHR = Serial.readStringUntil('\n');
    Serial.println(sizeCHR);
    newchrsize = sizeCHR.toInt() + chrlo;
    if (newchrsize > chrhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setchr;
    }
  }
  Serial.print(F("CHR Size = "));
  Serial.print(CHR[newchrsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(9, newchrsize);
  chrsize = newchrsize;
}

void setRAMSize() {
#if (defined(enable_LCD) || defined(enable_OLED))
  display_Clear();
  if (ramlo == ramhi)
    newramsize = ramlo;
  else {
    b = 0;
    int i = 0;

    display_Clear();
    print_Msg(F("RAM Size: "));
    if (mapper == 0)
      println_Msg(RAM[i] / 4);
    else if (mapper == 16)
      println_Msg(RAM[i] * 32);
    else if (mapper == 19) {
      if (i == 2)
        println_Msg(F("128"));
      else
        println_Msg(RAM[i]);
    }
    else if ((mapper == 159) || (mapper == 80))
      println_Msg(RAM[i] * 16);
    else if (mapper == 82)
      println_Msg(i * 5);
    else
      println_Msg(RAM[i]);
    println_Msg(F(""));
#if defined(enable_OLED)
    println_Msg(F("Press left to change"));
    println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
    println_Msg(F("Rotate to change"));
    println_Msg(F("Press to select"));
#endif
    display_Update();

    while (1) {
      b = checkButton();

      if (b == doubleclick) { // Previous Mapper
        if (i == 0)
          i = ramhi;
        else
          i--;

        display_Clear();
        print_Msg(F("RAM Size: "));
        if (mapper == 0)
          println_Msg(RAM[i] / 4);
        else if (mapper == 16)
          println_Msg(RAM[i] * 32);
        else if (mapper == 19) {
          if (i == 2)
            println_Msg(F("128"));
          else
            println_Msg(RAM[i]);
        }
        else if ((mapper == 159) || (mapper == 80))
          println_Msg(RAM[i] * 16);
        else if (mapper == 82)
          println_Msg(i * 5);
        else
          println_Msg(RAM[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to change"));
        println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to change"));
        println_Msg(F("Press to select"));
#endif
        display_Update();
      }

      if (b == press) { // Next
        if (i == ramhi)
          i = 0;
        else
          i++;

        display_Clear();
        print_Msg(F("RAM Size: "));
        if (mapper == 0)
          println_Msg(RAM[i] / 4);
        else if (mapper == 16)
          println_Msg(RAM[i] * 32);
        else if (mapper == 19) {
          if (i == 2)
            println_Msg(F("128"));
          else
            println_Msg(RAM[i]);
        }
        else if ((mapper == 159) || (mapper == 80))
          println_Msg(RAM[i] * 16);
        else if (mapper == 82)
          println_Msg(i * 5);
        else
          println_Msg(RAM[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to change"));
        println_Msg(F("Press right to select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to change"));
        println_Msg(F("Press to select"));
#endif
        display_Update();
      }

      if (b == hold) { // Long Press - Execute
        newramsize = i;
        break;
      }
    }

    display.setCursor(0, 56); // Display selection at bottom
  }
  if ((mapper == 16) || (mapper == 159)) {
    int sizeEEP = 0;
    print_Msg(F("EEPROM SIZE "));
    if (mapper == 16)
      sizeEEP = RAM[newramsize] * 32;
    else
      sizeEEP = RAM[newramsize] * 16;
    print_Msg(sizeEEP);
    println_Msg(F("B"));
  }
  else if (mapper == 19) {
    print_Msg(F("RAM SIZE "));
    if (newramsize == 2)
      println_Msg(F("128B"));
    else {
      print_Msg(RAM[newramsize]);
      println_Msg(F("K"));
    }
  }
  else if (mapper == 80) {
    print_Msg(F("RAM SIZE "));
    print_Msg(RAM[newramsize] * 16);
    println_Msg(F("B"));
  }
  else {
    print_Msg(F("RAM SIZE "));
    if (mapper == 0)
      print_Msg(newramsize * 2);
    else if (mapper == 82)
      print_Msg(newramsize * 5);
    else
      print_Msg(RAM[newramsize]);
    println_Msg(F("K"));
  }
  display_Update();
  delay(1000);

#elif defined(enable_serial)
  if (ramlo == ramhi)
    newramsize = ramlo;
  else {
setram:
    String sizeRAM;
    for (int i = 0; i < (ramhi - ramlo + 1); i++) {
      Serial.print(F("Select RAM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      if (mapper == 0) {
        Serial.print(RAM[i] / 4);
        Serial.println(F("K"));
      }
      else if ((mapper == 16) || (mapper == 159)) {
        if (mapper == 16)
          Serial.print(RAM[i + ramlo] * 32);
        else
          Serial.print(RAM[i + ramlo] * 16);
        Serial.println(F("B"));
      }
      else if (mapper == 19) {
        if (i == 2)
          Serial.println(F("128B"));
        else {
          Serial.print(RAM[i + ramlo]);
          Serial.println(F("K"));
        }
      }
      else {
        Serial.print(RAM[i + ramlo]);
        Serial.println(F("K"));
      }
    }
    Serial.print(F("Enter RAM Size: "));
    while (Serial.available() == 0) {}
    sizeRAM = Serial.readStringUntil('\n');
    Serial.println(sizeRAM);
    newramsize = sizeRAM.toInt() + ramlo;
    if (newramsize > ramhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setram;
    }
  }
  if ((mapper == 16) || (mapper == 159)) {
    int sizeEEP = 0;
    Serial.print(F("EEPROM Size = "));
    if (mapper == 16)
      sizeEEP = RAM[newramsize] * 32;
    else
      sizeEEP = RAM[newramsize] * 16;
    Serial.print(sizeEEP);
    Serial.println(F("B"));
    Serial.println(F(""));
  }
  else if (mapper == 19) {
    Serial.print(F("RAM Size =  "));
    if (newramsize == 2)
      Serial.println(F("128B"));
    else {
      Serial.print(RAM[newramsize]);
      Serial.println(F("K"));
    }
    Serial.println(F(""));
  }
  else if (mapper == 80) {
    Serial.print(F("RAM Size = "));
    Serial.print(RAM[newramsize] * 16);
    Serial.println(F("B"));
    Serial.println(F(""));
  }
  else {
    Serial.print(F("RAM Size = "));
    if (mapper == 0)
      Serial.print(newramsize * 2);
    else if (mapper == 82)
      Serial.print(newramsize * 5);
    else
      Serial.print(RAM[newramsize]);
    Serial.println(F("K"));
    Serial.println(F(""));
  }
#endif
  EEPROM_writeAnything(10, newramsize);
  ramsize = newramsize;
}

// MMC6 Detection
// Mapper 4 includes both MMC3 AND MMC6
// RAM is mapped differently between MMC3 and MMC6
void checkMMC6() { // Detect MMC6 Carts - read PRG 0x3E00A ("STARTROPICS")
  write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
  write_prg_byte(0x8001, 0x1F); // 0x3E000
  prgchk0 = read_prg_byte(0x800A);
  prgchk1 = read_prg_byte(0x800B);
  prgchk2 = read_prg_byte(0x800C);
  prgchk3 = read_prg_byte(0x800D);
  if ((prgchk0 == 0x53) && (prgchk1 == 0x54) && (prgchk2 == 0x41) && (prgchk3 == 0x52))
    mmc6 = true; // MMC6 Cart
}

void checkStatus_NES() {
  EEPROM_readAnything(7, mapper);
  EEPROM_readAnything(8, prgsize);
  EEPROM_readAnything(9, chrsize);
  EEPROM_readAnything(10, ramsize);
  prg = (int_pow(2, prgsize)) * 16;
  if (chrsize == 0)
    chr = 0; // 0K
  else
    chr = (int_pow(2, chrsize)) * 4;
  if (ramsize == 0)
    ram = 0; // 0K
  else if (mapper == 82)
    ram = 5; // 5K
  else
    ram = (int_pow(2, ramsize)) * 4;

  // Mapper Variants
  // Identify variant for use across multiple functions
  if (mapper == 4) { // Check for MMC6/MMC3
    checkMMC6();
    if (mmc6)
      ram = 1; // 1K
  }
  else if (mapper == 30) // Check for Flashable/Non-Flashable
    NESmaker_ID(); // Flash ID

  display_Clear();
  println_Msg(F("NES CART READER"));
  println_Msg(F(""));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("MAPPER:   "));
  println_Msg(mapper);
  print_Msg(F("PRG SIZE: "));
  print_Msg(prg);
  println_Msg(F("K"));
  print_Msg(F("CHR SIZE: "));
  print_Msg(chr);
  println_Msg(F("K"));
  print_Msg(F("RAM SIZE: "));
  if (mapper == 0) {
    print_Msg(ram / 4);
    println_Msg(F("K"));
  }
  else if ((mapper == 16) || (mapper == 80) || (mapper == 159)) {
    if (mapper == 16)
      print_Msg(ram * 32);
    else
      print_Msg(ram * 16);
    println_Msg(F("B"));
  }
  else if (mapper == 19) {
    if (ramsize == 2)
      println_Msg(F("128B"));
    else {
      print_Msg(ram);
      println_Msg(F("K"));
    }
  }
  else {
    print_Msg(ram);
    println_Msg(F("K"));
  }
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   ROM Functions
 *****************************************/
void dumpPRG(word base, word address) {
  for (int x = 0; x < 512; x++) {
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  myFile.write(sdBuffer, 512);
}

void dumpCHR(word address) {
  for (int x = 0; x < 512; x++) {
    sdBuffer[x] = read_chr_byte(address + x);
  }
  myFile.write(sdBuffer, 512);
}

void dumpCHR_M2(word address) { // MAPPER 45 - PULSE M2 LO/HI
  for (int x = 0; x < 512; x++) {
    PHI2_LOW;
    sdBuffer[x] = read_chr_byte(address + x);
  }
  myFile.write(sdBuffer, 512);
}

void dumpMMC5RAM(word base, word address) { // MMC5 SRAM DUMP - PULSE M2 LO/HI
  for (int x = 0; x < 512; x++) {
    PHI2_LOW;
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  myFile.write(sdBuffer, 512);
}

void writeMMC5RAM(word base, word address) { // MMC5 SRAM WRITE
  myFile.read(sdBuffer, 512);
  for (int x = 0; x < 512; x++) {
    do {
      write_prg_byte(0x5102, 2); // PRG RAM PROTECT1
      write_prg_byte(0x5103, 1); // PRG RAM PROTECT2
      write_wram_byte(base + address + x, sdBuffer[x]);
      bytecheck = read_prg_byte(base + address + x);
    }
    while (bytecheck != sdBuffer[x]); // CHECK WRITTEN BYTE
  }
  write_prg_byte(0x5102, 0); // PRG RAM PROTECT1
  write_prg_byte(0x5103, 0); // PRG RAM PROTECT2
}

void readPRG(boolean readrom) {
  if (!readrom) {
    display_Clear();
    display_Update();

    LED_BLUE_ON;
    set_address(0);
    _delay_us(1);
    CreatePRGFileInSD();
  }
  else {
    set_address(0);
    _delay_us(1);
  }

  word base = 0x8000;

  if (myFile) {
    switch (mapper) {
      case 0:
      case 3:
      case 13:
      case 87: // 16K/32K
      case 184: // 32K
      case 185: // 16K/32K
        for (word address = 0; address < ((prgsize * 0x4000) + 0x4000); address += 512) { // 16K or 32K
          dumpPRG(base, address);
        }
        break;

      case 1:
      case 155: // 32K/64K/128K/256K/512K
        banks = int_pow(2, prgsize) - 1;
        for (int i = 0; i < banks; i++) { // 16K Banks ($8000-$BFFF)
          write_prg_byte(0x8000, 0x80); // Clear Register
          write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
          if (prgsize > 4) // 512K
            write_mmc1_byte(0xA000, 0x00); // Reset 512K Flag for Lower 256K
          if (i > 15) // Switch Upper 256K
            write_mmc1_byte(0xA000, 0x10); // Set 512K Flag
          write_mmc1_byte(0xE000, i);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        for (word address = 0x4000; address < 0x8000; address += 512) { // Final Bank ($C000-$FFFF)
          dumpPRG(base, address);
        }
        break;

      case 2: // 128K/256K
        for (int i = 0; i < 8; i++) { // 128K/256K
          write_prg_byte(0x8000, i);
          for (word address = 0x0; address < (((prgsize - 3) * 0x4000) + 0x4000); address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 4:
      case 47:
      case 64:
      case 118:
      case 119:
      case 158:
        banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
        if (mapper == 47)
          write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
        for (int i = 0; i < banks; i += 2) { // 32K/64K/128K/256K/512K
          if (mapper == 47) {
            if (i == 0)
              write_prg_byte(0x6000, 0); // Switch to Lower Block
            else if (i == 16)
              write_prg_byte(0x6000, 1); // Switch to Upper Block
          }
          write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7); // PRG Bank 1 ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        if ((mapper == 64) || (mapper == 158)) {
          write_prg_byte(0x8000, 15); // PRG Bank 2 ($C000-$DFFF)
          write_prg_byte(0x8001, banks);
        }
        for (word address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
          dumpPRG(base, address);
        }
        break;

      case 5: // 128K/256K/512K
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x5100, 3); // 8K PRG Banks
        for (int i = 0; i < banks; i += 2) { // 128K/256K/512K
          write_prg_byte(0x5114, i | 0x80);
          write_prg_byte(0x5115, (i + 1) | 0x80);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 7: // 128K/256K
      case 34:
      case 77:
      case 96: // 128K
        banks = int_pow(2, prgsize) / 2;
        for (int i = 0; i < banks; i++) { // 32K Banks
          write_prg_byte(0x8000, i);
          for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 9: // 128K
        for (int i = 0; i < 13; i++) { // 16-3 = 13 = 128K
          write_prg_byte(0xA000, i); // $8000-$9FFF
          for (word address = 0x0; address < 0x2000; address += 512) { // Switch Bank ($8000-$9FFF)
            dumpPRG(base, address);
          }
        }
        for (word address = 0x2000; address < 0x8000; address += 512) { // Final 3 Banks ($A000-$FFFF)
          dumpPRG(base, address);
        }
        break;

      case 10: // 128K/256K
        for (int i = 0; i < (((prgsize - 3) * 8) + 7); i++) {
          write_prg_byte(0xA000, i); // $8000-$BFFF
          for (word address = 0x0; address < 0x4000; address += 512) { // Switch Bank ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        for (word address = 0x4000; address < 0x8000; address += 512) { // Final Bank ($C000-$FFFF)
          dumpPRG(base, address);
        }
        break;

      case 16:
      case 159: // 128K/256K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0x6008, i); // Submapper 4
          write_prg_byte(0x8008, i); // Submapper 5
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 18: // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i & 0xF);
          write_prg_byte(0x8001, (i >> 4) & 0xF);
          write_prg_byte(0x8002, (i + 1) & 0xF);
          write_prg_byte(0x8003, ((i + 1) >> 4) & 0xF);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 19: // 128K/256K
        for (int j = 0; j < 64; j++) { // Init Register
          write_ram_byte(0xE000, 0); // PRG Bank 0 ($8000-$9FFF)
        }
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i++) {
          write_ram_byte(0xE000, i); // PRG Bank 0 ($8000-$9FFF)
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 21:  // 256K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xA000, i);
          for (word address = 0x2000; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 22:
      case 23:
      case 25:
      case 65:
      case 75: // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i);
          write_prg_byte(0xA000, i + 1);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 24:
      case 26: // 256K
      case 78: // 128K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 128K
          write_prg_byte(0x8000, i);
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 30: // 256K/512K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 256K/512K
          if (flashfound)
            write_prg_byte(0xC000+i, i); // Flashable
          else
            write_prg_byte(0x8000+i, i); // Non-Flashable
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 32: // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i++) { // 128K/256K
          write_prg_byte(0x9000, 1); // PRG Mode 0 - Read $A000-$BFFF to avoid difference between Modes 0 and 1
          write_prg_byte(0xA000, i); // PRG Bank
          for (word address = 0x2000; address < 0x4000; address += 512) { // 8K Banks ($A000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 33:
      case 48: // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i); // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i + 1); // PRG Bank 1 ($A000-$BFFF)
          for (word address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 37:
        banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
        write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
        for (int i = 0; i < banks; i += 2) { // 256K
          if (i == 0)
            write_prg_byte(0x6000, 0); // Switch to Lower Block ($0000-$FFFF)
          else if (i == 8)
            write_prg_byte(0x6000, 3); // Switch to 2nd 64K Block ($10000-$1FFFF)
          else if (i == 16)
            write_prg_byte(0x6000, 4); // Switch to 128K Block ($20000-$3FFFF)
          write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7); // PRG Bank 1 ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        for (word address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
          dumpPRG(base, address);
        }
        break;

      case 45: // MMC3 Clone with Outer Registers
        banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
        for (int i = 0; i < banks; i += 2) { // 128K/256K/512K/1024K
          // set outer bank registers
          write_prg_byte(0x6000, 0x00); // CHR-OR
          write_prg_byte(0x6000, (i & 0xC0)); // PRG-OR
          write_prg_byte(0x6000, ((i >> 2) & 0xC0)); // CHR-AND,CHR-OR/PRG-OR
          write_prg_byte(0x6000, 0x80); // PRG-AND
          // set inner bank registers
          write_prg_byte(0x8000, 6); // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i);
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpPRG(base, address);
          }
          // set outer bank registers
          write_prg_byte(0x6000, 0x00); // CHR-OR
          write_prg_byte(0x6000, ((i + 1) & 0xC0)); // PRG-OR
          write_prg_byte(0x6000, (((i + 1) >> 2) & 0xC0)); // CHR-AND,CHR-OR/PRG-OR
          write_prg_byte(0x6000, 0x80); // PRG-AND
          // set inner bank registers
          write_prg_byte(0x8000, 7); // PRG Bank 1 ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1);
          for (word address = 0x2000; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        for (word address = 0x4000; address < 0x8000; address += 512) { // Final 2 Banks ($C000-$FFFF)
          dumpPRG(base, address);
        }
        break;

      case 66: // 64K/128K
        banks = int_pow(2, prgsize) / 2;
        for (int i = 0; i < banks; i++) { // 64K/128K
          write_prg_byte(0x8000, i << 4); // bits 4-5
          for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 67: // 128K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 128K
          write_reg_byte(0xF800, i); // [WRITE RAM SAFE]
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 68:
      case 73: // 128K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 128K
          write_reg_byte(0xF000, i); // [WRITE RAM SAFE]
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 69: // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
        write_prg_byte(0xA000, 0); // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
        for (int i = 0; i < banks; i++) { // 128K/256K
          write_prg_byte(0x8000, 9); // Command Register - PRG Bank 1
          write_prg_byte(0xA000, i); // Parameter Register - ($8000-$9FFF)
          for (word address = 0x0000; address < 0x2000; address += 512) { // 8K Banks ($8000-$9FFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 70:
      case 89:
      case 152: // 64K/128K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 128K
          write_prg_byte(0x8000, i << 4);
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 71: // 64K/128K/256K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0xC000, i);
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 72: // 128K
        banks = int_pow(2, prgsize);
        write_prg_byte(0x8000, 0); // Reset Register
        for (int i = 0; i < banks; i++) { // 128K
          write_prg_byte(0x8000, i | 0x80); // PRG Command + Bank
          write_prg_byte(0x8000, i); // PRG Bank
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 76:
      case 88:
      case 95:
      case 154: // 128K
      case 206: // 32K/64K/128K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, 6); // PRG ROM Command ($8000-$9FFF)
          write_prg_byte(0x8001, i); // PRG Bank
          write_prg_byte(0x8000, 7); // PRG ROM Command ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1); // PRG Bank
          for (word address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 80: // 128K
      case 207: // 256K [CART SOMETIMES NEEDS POWERCYCLE]
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0x7EFA, i); // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x7EFC, i + 1); // PRG Bank 1 ($A000-$BFFF)
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 82: // 128K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0x7EFA, i << 2); // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x7EFB, (i + 1) << 2); // PRG Bank 1 ($A000-$BFFF)
          for (word address = 0x0; address < 0x4000; address += 512) { // 8K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 85: // 128K/512K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0x8000, i); // PRG Bank 0 ($8000-$9FFF)
          for (word address = 0x0; address < 0x2000; address += 512) { // 8K Banks ($8000-$9FFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 86:
      case 140: // 128K
        banks = int_pow(2, prgsize) / 2;
        for (int i = 0; i < banks; i++) { // 128K
          write_prg_byte(0x6000, i << 4); // bits 4-5
          for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 92: // 256K
        banks = int_pow(2, prgsize);
        write_prg_byte(0x8000, 0); // Reset Register
        for (int i = 0; i < banks; i++) { // 256K
          write_prg_byte(0x8000, i | 0x80); // PRG Command + Bank
          write_prg_byte(0x8000, i); // PRG Bank
          for (word address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
            dumpPRG(base, address);
          }
        }
        break;
      
      case 93:
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0x6000, i);
          write_prg_byte(0x8000, i << 4 | 0x01);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;
        
      case 94:
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 128K
          write_prg_byte(0x8000, i << 2);
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;

      case 97: // 256K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 256K
          write_prg_byte(0x8000, i); // PRG Bank
          for (word address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 105: // 256K
        write_mmc1_byte(0xA000, 0x00); // Clear PRG Init/IRQ (Bit 4)
        write_mmc1_byte(0xA000, 0x10); // Set PRG Init/IRQ (Bit 4) to enable bank swapping
        for (int i = 0; i < 4; i++) { // PRG CHIP 1 128K
          write_mmc1_byte(0xA000, i << 1);
          for (word address = 0x0; address < 0x8000; address += 512) { // 32K Banks ($8000-$FFFF)
            dumpPRG(base, address);
          }
        }
        write_mmc1_byte(0x8000, 0x0C); // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
        write_mmc1_byte(0xA000, 0x08); // Select PRG CHIP 2 (Bit 3)
        for (int j = 0; j < 8; j++) { // PRG CHIP 2 128K
          write_mmc1_byte(0xE000, j);
          for (word address = 0x0; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 180: // 128K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) {
          write_prg_byte(0x8000, i);
          for (word address = 0x4000; address < 0x8000; address += 512) { // 16K Banks ($C000-$FFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 153: // 512K
        banks = int_pow(2, prgsize);
        for (int i = 0; i < banks; i++) { // 512K
          write_prg_byte(0x8000, i >> 4); // PRG Outer Bank (Documentation says duplicate over $8000-$8003 registers)
          write_prg_byte(0x8001, i >> 4); // PRG Outer Bank
          write_prg_byte(0x8002, i >> 4); // PRG Outer Bank
          write_prg_byte(0x8003, i >> 4); // PRG Outer Bank
          write_prg_byte(0x8008, i & 0xF); // PRG Inner Bank
          for (word address = 0x0000; address < 0x4000; address += 512) { // 16K Banks ($8000-$BFFF)
            dumpPRG(base, address);
          }
        }
        break;

      case 210: // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (int i = 0; i < banks; i += 2) {
          write_prg_byte(0xE000, i); // PRG Bank 0 ($8000-$9FFF) [WRITE NO RAM]
          write_prg_byte(0xE800, i + 1); // PRG Bank 1 ($A000-$BFFF) [WRITE NO RAM]
          for (word address = 0x0; address < 0x4000; address += 512) {
            dumpPRG(base, address);
          }
        }
        break;
    }
    if (!readrom) {
      myFile.flush();
      myFile.close();

      println_Msg(F("PRG FILE DUMPED!"));
      println_Msg(F(""));
      display_Update();
#ifndef no-intro
      calcCRC(fileName, prg * 1024, &prg_crc32, 0);
#endif
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  LED_BLUE_OFF;
}

void readCHR(boolean readrom) {
  if (!readrom) {
    display_Clear();
    display_Update();
  }

  LED_GREEN_ON;
  set_address(0);
  _delay_us(1);
  if (chrsize == 0) {
    println_Msg(F("CHR SIZE 0K"));
    display_Update();
  }
  else {
    if (!readrom) {
      CreateCHRFileInSD();
    }
    if (myFile) {
      switch (mapper) {
        case 0: // 8K
          for (word address = 0x0; address < 0x2000; address += 512) {
            dumpCHR(address);
          }
          break;

        case 1:
        case 155:
          banks = int_pow(2, chrsize);
          for (int i = 0; i < banks; i += 2) { // 8K/16K/32K/64K/128K (Bank #s are based on 4K Banks)
            write_prg_byte(0x8000, 0x80); // Clear Register
            write_mmc1_byte(0xA000, i);
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 3: // 8K/16K/32K
        case 66: // 16K/32K
        case 70:
        case 152: // 128K
          banks = int_pow(2, chrsize) / 2;
          for (int i = 0; i < banks; i++) { // 8K Banks
            write_prg_byte(0x8000, i); // CHR Bank 0
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 4:
        case 47:
        case 64:
        case 118:
        case 119:
        case 158:
          banks = int_pow(2, chrsize) * 4;
          if (mapper == 47)
            write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
          for (int i = 0; i < banks; i += 4) { // 8K/16K/32K/64K/128K/256K
            if (mapper == 47) {
              if (i == 0)
                write_prg_byte(0x6000, 0); // Switch to Lower Block
              else if (i == 128)
                write_prg_byte(0x6000, 1); // Switch to Upper Block
            }
            write_prg_byte(0x8000, 0); // CHR Bank 0 ($0000-$07FF)
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 1); // CHR Bank 1 ($0800-$0FFF)
            write_prg_byte(0x8001, i + 2);
            for (word address = 0x0; address < 0x1000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 5: // 128K/256K/512K
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x5101, 0); // 8K CHR Banks
          for (int i = 0; i < banks; i++) {
            if (i == 0)
              write_prg_byte(0x5130, 0); // Set Upper 2 bits
            else if (i == 8)
              write_prg_byte(0x5130, 1); // Set Upper 2 bits
            else if (i == 16)
              write_prg_byte(0x5130, 2); // Set Upper 2 bits
            else if (i == 24)
              write_prg_byte(0x5130, 3); // Set Upper 2 bits
            write_prg_byte(0x5127, i);
            for (word address = 0x0; address < 0x2000; address += 512) { // ($0000-$1FFF)
              dumpCHR(address);
            }
          }
          break;

        case 9:
        case 10: // Mapper 9: 128K, Mapper 10: 64K/128K
          if (mapper == 9)
            banks = 32;
          else // Mapper 10
            banks = int_pow(2, chrsize);
          for (int i = 0; i < banks; i++) { // 64K/128K
            write_prg_byte(0xB000, i);
            write_prg_byte(0xC000, i);
            for (word address = 0x0; address < 0x1000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 16:
        case 159: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0x6000, i); // Submapper 4
            write_prg_byte(0x8000, i); // Submapper 5
            for (word address = 0x0; address < 0x400; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 18: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0xA000, i & 0xF); // CHR Bank Lower 4 bits
            write_prg_byte(0xA001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits
            for (word address = 0x0; address < 0x400; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 19: // 128K/256K
          for (int j = 0; j < 64; j++) { // Init Register
            write_ram_byte(0xE800, 0xC0); // CHR RAM High/Low Disable (ROM Enable)
          }
          banks = int_pow(2, chrsize) * 4;
          write_ram_byte(0xE800, 0xC0); // CHR RAM High/Low Disable (ROM Enable)
          for (int i = 0; i < banks; i += 8) {
            write_prg_byte(0x8000, i); // CHR Bank 0
            write_prg_byte(0x8800, i + 1); // CHR Bank 1
            write_prg_byte(0x9000, i + 2); // CHR Bank 2
            write_prg_byte(0x9800, i + 3); // CHR Bank 3
            write_prg_byte(0xA000, i + 4); // CHR Bank 4
            write_prg_byte(0xA800, i + 5); // CHR Bank 5
            write_prg_byte(0xB000, i + 6); // CHR Bank 6
            write_prg_byte(0xB800, i + 7); // CHR Bank 7
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 21: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
            if (chrsize == 5) // Check CHR Size to determine VRC4a (128K) or VRC4c (256K)
              write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4a (Wai Wai World 2)
            else  // banks == 256
              write_prg_byte(0xB040, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4c (Ganbare Goemon Gaiden 2)
            for (word address = 0x0; address < 0x400; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 22: // 128K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0xB000, (i << 1) & 0xF); // CHR Bank Lower 4 bits
            write_prg_byte(0xB002, (i >> 3) & 0xF);  // CHR Bank Upper 4 bits
            for (word address = 0x0; address < 0x400; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 23: // 128K
          // Detect VRC4e Carts - read PRG 0x1FFF6 (DATE)
          // Boku Dracula-kun = 890810, Tiny Toon = 910809
          // Crisis Force = 910701, Parodius Da! = 900916
          write_prg_byte(0x8000, 15);
          prgchk0 = read_prg_byte(0x9FF6);
          if (prgchk0 == 0x30) { // Check for "0" in middle of date
            vrc4e = true; // VRC4e Cart
          }
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
            if (vrc4e == true)
              write_prg_byte(0xB004, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4e
            else
              write_prg_byte(0xB001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC2b/VRC4f
            for (word address = 0x0; address < 0x400; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 24: // 128K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xB003, 0); // PPU Banking Mode 0
          for (int i = 0; i < banks; i += 8) {
            write_prg_byte(0xD000, i); // CHR Bank 0
            write_prg_byte(0xD001, i + 1); // CHR Bank 1
            write_prg_byte(0xD002, i + 2); // CHR Bank 2
            write_prg_byte(0xD003, i + 3); // CHR Bank 3
            write_prg_byte(0xE000, i + 4); // CHR Bank 4 [WRITE NO RAM]
            write_prg_byte(0xE001, i + 5); // CHR Bank 5 [WRITE NO RAM]
            write_prg_byte(0xE002, i + 6); // CHR Bank 6 [WRITE NO RAM]
            write_prg_byte(0xE003, i + 7); // CHR Bank 7 [WRITE NO RAM]
            for (word address = 0x0; address < 0x2000; address += 512) { // 1K Banks
              dumpCHR(address);
            }
          }
          break;

        case 25: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0xB000, i & 0xF); // CHR Bank Lower 4 bits
            if ((ramsize > 0) || (banks == 128)) // VRC2c (Ganbare Goemon Gaiden)/VRC4b (Bio Miracle/Gradius 2/Racer Mini)
              write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC2c/VRC4b
            else
              write_prg_byte(0xB008, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4d (Teenage Mutant Ninja Turtles)
            for (word address = 0x0; address < 0x400; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 26: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xB003, 0x00);
          for (int i = 0; i < banks; i += 4) {
            write_prg_byte(0xD000, i + 0); // CHR Bank 0
            write_prg_byte(0xD002, i + 1); // CHR Bank 1
            write_prg_byte(0xD001, i + 2); // CHR Bank 2
            write_prg_byte(0xD003, i + 3); // CHR Bank 3
            for (word address = 0x0; address < 0x1000; address += 512) { // 1K Banks
              dumpCHR(address);
            }
          }
          break;

        case 32: // 128K
        case 65: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i += 8) {
            write_prg_byte(0xB000, i); // CHR Bank 0
            write_prg_byte(0xB001, i + 1); // CHR Bank 1
            write_prg_byte(0xB002, i + 2); // CHR Bank 2
            write_prg_byte(0xB003, i + 3); // CHR Bank 3
            write_prg_byte(0xB004, i + 4); // CHR Bank 4
            write_prg_byte(0xB005, i + 5); // CHR Bank 5
            write_prg_byte(0xB006, i + 6); // CHR Bank 6
            write_prg_byte(0xB007, i + 7); // CHR Bank 7
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 33: // 128K/256K
        case 48: // 256K
          banks = int_pow(2, chrsize) * 2;
          for (int i = 0; i < banks; i += 2) { // 2K Banks
            write_prg_byte(0x8002, i); // CHR Bank 0
            write_prg_byte(0x8003, i + 1); // CHR Bank 1
            for (word address = 0x0; address < 0x1000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 37:
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xA001, 0x80); // Block Register - PRG RAM Chip Enable, Writable
          for (int i = 0; i < banks; i += 4) { // 256K
            if (i == 0)
              write_prg_byte(0x6000, 0); // Switch to Lower Block ($00000-$1FFFF)
            else if (i == 128)
              write_prg_byte(0x6000, 4); // Switch to Upper Block ($20000-$3FFFF)
            write_prg_byte(0x8000, 0); // CHR Bank 0 ($0000-$07FF)
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 1); // CHR Bank 1 ($0800-$0FFF)
            write_prg_byte(0x8001, i + 2);
            for (word address = 0x0; address < 0x1000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 45: // 128K/256K/512K/1024K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xA001, 0x80); // Unlock Write Protection - not used by some carts
          for (int i = 0; i < banks; i++) {
            // set outer bank registers
            write_prg_byte(0x6000, 0x00); // CHR-OR
            write_prg_byte(0x6000, 0x00); // PRG-OR
            write_prg_byte(0x6000, (((i / 256) << 4) | 0x0F)); // CHR-AND,CHR-OR/PRG-OR
            write_prg_byte(0x6000, 0x80); // PRG-AND
            // set inner bank registers
            write_prg_byte(0x8000, 0x2); // CHR Bank 2 ($1000-$13FF)
            write_prg_byte(0x8001, i);
            for (word address = 0x1000; address < 0x1200; address += 512) {
              dumpCHR_M2(address); // Read CHR with M2 Pulse
            }
            // set outer bank registers
            write_prg_byte(0x6000, 0x00); // CHR-OR
            write_prg_byte(0x6000, 0x00); // PRG-OR
            write_prg_byte(0x6000, (((i / 256) << 4) | 0x0F)); // CHR-AND,CHR-OR/PRG-OR
            write_prg_byte(0x6000, 0x80); // PRG-AND
            // set inner bank registers
            write_prg_byte(0x8000, 0x2); // CHR Bank 2 ($1000-$13FF)
            write_prg_byte(0x8001, i);
            for (word address = 0x1200; address < 0x1400; address += 512) {
              dumpCHR_M2(address); // Read CHR with M2 Pulse
            }
          }
          break;

        case 67: // 128K
          banks = int_pow(2, chrsize) * 2;
          for (int i = 0; i < banks; i += 4) { // 2K Banks
            write_prg_byte(0x8800, i); // CHR Bank 0
            write_prg_byte(0x9800, i + 1); // CHR Bank 1
            write_prg_byte(0xA800, i + 2); // CHR Bank 2
            write_prg_byte(0xB800, i + 3); // CHR Bank 3
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 68: // 128K/256K
          banks = int_pow(2, chrsize) * 2;
          for (int i = 0; i < banks; i += 4) { // 2K Banks
            write_prg_byte(0x8000, i); // CHR Bank 0
            write_prg_byte(0x9000, i + 1); // CHR Bank 1
            write_prg_byte(0xA000, i + 2); // CHR Bank 2
            write_prg_byte(0xB000, i + 3); // CHR Bank 3
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 69: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i++) {
            write_prg_byte(0x8000, 0); // Command Register - CHR Bank 0
            write_prg_byte(0xA000, i); // Parameter Register - ($0000-$03FF)
            for (word address = 0x0; address < 0x400; address += 512) { // 1K Banks
              dumpCHR(address);
            }
          }
          break;

        case 72: // 128K
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x8000, 0); // Reset Register
          for (int i = 0; i < banks; i++) { // 8K Banks
            write_prg_byte(0x8000, i | 0x40); // CHR Command + Bank
            write_prg_byte(0x8000, i); // CHR Bank
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 75: // 128K
          banks = int_pow(2, chrsize);
          for (int i = 0; i < banks; i++) { // 4K Banks
            write_reg_byte(0xE000, i); // CHR Bank Low Bits [WRITE RAM SAFE]
            write_prg_byte(0x9000, (i & 0x10) >> 3); // High Bit
            for (word address = 0x0; address < 0x1000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 76: // 128K
          banks = int_pow(2, chrsize) * 2;
          for (int i = 0; i < banks; i += 2) { // 2K Banks
            write_prg_byte(0x8000, 2); // CHR Command ($0000-$07FF) 2K Bank
            write_prg_byte(0x8001, i); // CHR Bank
            write_prg_byte(0x8000, 3); // CHR Command ($0800-$0FFF) 2K Bank
            write_prg_byte(0x8001, i + 1); // CHR Bank
            for (word address = 0x0000; address < 0x1000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 77: // 32K
          banks = int_pow(2, chrsize) * 2;
          for (int i = 0; i < banks; i++) { // 2K Banks
            write_prg_byte(0x8000, i << 4); // CHR Bank 0
            for (word address = 0x0; address < 0x800; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 78: // 128K
          banks = int_pow(2, chrsize) / 2;
          for (int i = 0; i < banks; i++) { // 8K Banks
            write_prg_byte(0x8000, i << 4); // CHR Bank 0
            for (word address = 0x0; address < 0x2000; address += 512) { // 8K Banks ($0000-$1FFF)
              dumpCHR(address);
            }
          }
          break;

        case 80: // 128K/256K
        case 82: // 128K/256K
        case 207: // 128K [CART SOMETIMES NEEDS POWERCYCLE]
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i += 4) {
            write_prg_byte(0x7EF2, i);  // CHR Bank 2 [REGISTERS 0x7EF0/0x7EF1 WON'T WORK]
            write_prg_byte(0x7EF3, i + 1);  // CHR Bank 3
            write_prg_byte(0x7EF4, i + 2);  // CHR Bank 4
            write_prg_byte(0x7EF5, i + 3);  // CHR Bank 5
            for (word address = 0x1000; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 85: // 128K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i += 8) {
            write_prg_byte(0xA000, i); // CHR Bank 0
            write_prg_byte(0xA008, i + 1); // CHR Bank 1
            write_prg_byte(0xB000, i + 2); // CHR Bank 2
            write_prg_byte(0xB008, i + 3); // CHR Bank 3
            write_prg_byte(0xC000, i + 4); // CHR Bank 4
            write_prg_byte(0xC008, i + 5); // CHR Bank 5
            write_prg_byte(0xD000, i + 6); // CHR Bank 6
            write_prg_byte(0xD008, i + 7); // CHR Bank 7
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 86: // 64K
          banks = int_pow(2, chrsize) / 2;
          for (int i = 0; i < banks; i++) { // 8K Banks
            if (i < 4)
              write_prg_byte(0x6000, i & 0x3);
            else
              write_prg_byte(0x6000, (i | 0x40) & 0x43);
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 87: // 16K/32K
          banks = int_pow(2, chrsize) / 2;
          for (int i = 0; i < banks; i++) { // 16K/32K
            write_prg_byte(0x6000, (((i & 0x1) << 1) | ((i & 0x2) >> 1)));
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 88: // 128K
        case 95: // 32K
        case 154: // 128K
        case 206: // 16K/32K/64K
          banks = int_pow(2, chrsize) * 4;
          for (int i = 0; i < banks; i += 2) { // 1K Banks
            if (i < 64) {
              write_prg_byte(0x8000, 0); // CHR Command ($0000-$07FF) 2K Bank
              write_prg_byte(0x8001, i & 0x3F); // CHR Bank
              for (word address = 0x0; address < 0x800; address += 512) {
                dumpCHR(address);
              }
            }
            else {
              write_prg_byte(0x8000, 2); // CHR Command ($1000-$13FF) 1K Bank
              write_prg_byte(0x8001, i); // CHR Bank
              write_prg_byte(0x8000, 3); // CHR Command ($1400-$17FF) 1K Bank
              write_prg_byte(0x8001, i + 1); // CHR Bank
              for (word address = 0x1000; address < 0x1800; address += 512) {
                dumpCHR(address);
              }
            }
          }
          break;

        case 89: // 128K
          banks = int_pow(2, chrsize) / 2;
          for (int i = 0; i < banks; i++) { // 8K Banks
            if (i < 8)
              write_prg_byte(0x8000, i & 0x7);
            else
              write_prg_byte(0x8000, (i | 0x80) & 0x87);
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 92: // 128K
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x8000, 0); // Reset Register
          for (int i = 0; i < banks; i++) { // 8K Banks
            write_prg_byte(0x8000, i | 0x40); // CHR Command + Bank
            write_prg_byte(0x8000, i); // CHR Bank
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 140: // 32K/128K
          banks = int_pow(2, chrsize) / 2;
          for (int i = 0; i < banks; i++) { // 8K Banks
            write_prg_byte(0x6000, i);
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;

        case 184: // 16K/32K
          banks = int_pow(2, chrsize);
          for (int i = 0; i < banks; i++) { // 4K Banks
            write_prg_byte(0x6000, i); // CHR LOW (Bits 0-2) ($0000-$0FFF)
            for (word address = 0x0; address < 0x1000; address += 512) { // 4K Banks ($0000-$0FFF)
              dumpCHR(address);
            }
          }
          break;

        case 185: // 8K [READ 32K TO OVERRIDE LOCKOUT]
          for (int i = 0; i < 4; i++) { // Read 32K to locate valid 8K
            write_prg_byte(0x8000, i);
            byte chrcheck = read_chr_byte(0);
            for (word address = 0x0; address < 0x2000; address += 512) {
              for (int x = 0; x < 512; x++) {
                sdBuffer[x] = read_chr_byte(address + x);
              }
              if (chrcheck != 0xFF)
                myFile.write(sdBuffer, 512);
            }
          }
          break;

        case 210: // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xE800, 0xC0); // CHR RAM DISABLE (Bit 6 and 7) [WRITE NO RAM]
          for (int i = 0; i < banks; i += 8) {
            write_prg_byte(0x8000, i); // CHR Bank 0
            write_prg_byte(0x8800, i + 1); // CHR Bank 1
            write_prg_byte(0x9000, i + 2); // CHR Bank 2
            write_prg_byte(0x9800, i + 3); // CHR Bank 3
            write_prg_byte(0xA000, i + 4); // CHR Bank 4
            write_prg_byte(0xA800, i + 5); // CHR Bank 5
            write_prg_byte(0xB000, i + 6); // CHR Bank 6
            write_prg_byte(0xB800, i + 7); // CHR Bank 7
            for (word address = 0x0; address < 0x2000; address += 512) {
              dumpCHR(address);
            }
          }
          break;
      }
      if (!readrom) {
        myFile.flush();
        myFile.close();

        println_Msg(F("CHR FILE DUMPED!"));
        println_Msg(F(""));
        display_Update();
#ifndef no-intro
        calcCRC(fileName, chr * 1024, &chr_crc32, 0);
#endif
      }
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  LED_GREEN_OFF;
}

/******************************************
   RAM Functions
 *****************************************/
void readRAM() {
  display_Clear();
  display_Update();

  LED_BLUE_ON;
  LED_GREEN_ON;
  set_address(0);
  _delay_us(1);
  if (ramsize == 0) {

    println_Msg(F("RAM SIZE 0K"));
    display_Update();
  }
  else {
    CreateRAMFileInSD();
    word base = 0x6000;
    if (myFile) {
      switch (mapper) {
        case 0: // 2K/4K
          for (word address = 0x0; address < (0x800 * ramsize); address += 512) { // 2K/4K
            dumpPRG(base, address); // SWITCH MUST BE IN OFF POSITION
          }
          break;

        case 1:
        case 155: // 8K/16K/32K
          banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
          for (int i = 0; i < banks; i++) { // 8K Banks ($6000-$7FFF)
            write_prg_byte(0x8000, 0x80); // Clear Register
            write_mmc1_byte(0x8000, 1 << 3);
            write_mmc1_byte(0xE000, 0);
            if (banks == 4) // 32K
              write_mmc1_byte(0xA000, i << 2);
            else
              write_mmc1_byte(0xA000, i << 3);
            for (word address = 0x0; address < 0x2000; address += 512) { // 8K
              dumpPRG(base, address);
            }
          }
          break;

        case 4: // 1K/8K (MMC6/MMC3)
          if (mmc6) { // MMC6 1K
            write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
            write_prg_byte(0xA001, 0x20); // PRG RAM PROTECT - Enable reading RAM at $7000-$71FF
            for (word address = 0x1000; address < 0x1200; address += 512) { // 512B
              dumpMMC5RAM(base, address);
            }
            write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
            write_prg_byte(0xA001, 0x80); // PRG RAM PROTECT - Enable reading RAM at $7200-$73FF
            for (word address = 0x1200; address < 0x1400; address += 512) { // 512B
              dumpMMC5RAM(base, address);
            }
            write_prg_byte(0x8000, 6); // PRG RAM DISABLE
          }
          else { // MMC3 8K
            write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              dumpPRG(base, address);
            }
          }
          break;

        case 5: // 8K/16K/32K
          write_prg_byte(0x5100, 3); // 8K PRG Banks
          banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
          if (banks == 2) { // 16K - Split SRAM Chips 8K/8K
            for (int i = 0; i < (banks / 2); i++) { // Chip 1
              write_prg_byte(0x5113, i);
              for (word address = 0; address < 0x2000; address += 512) { // 8K
                dumpMMC5RAM(base, address);
              }
            }
            for (int j = 4; j < (banks / 2) + 4; j++) { // Chip 2
              write_prg_byte(0x5113, j);
              for (word address = 0; address < 0x2000; address += 512) { // 8K
                dumpMMC5RAM(base, address);
              }
            }
          }
          else { // 8K/32K Single SRAM Chip
            for (int i = 0; i < banks; i++) { // banks = 1 or 4
              write_prg_byte(0x5113, i);
              for (word address = 0; address < 0x2000; address += 512) { // 8K
                dumpMMC5RAM(base, address);
              }
            }
          }
          break;

        case 16: // 256-byte EEPROM 24C02
        case 159: // 128-byte EEPROM 24C01 [Little Endian]
          if (mapper == 159)
            eepsize = 128;
          else
            eepsize = 256;
          for (word address = 0; address < eepsize; address++) {
            EepromREAD(address);
          }
          myFile.write(sdBuffer, eepsize);
          //          display_Clear(); // TEST PURPOSES - DISPLAY EEPROM DATA
          break;

        case 19:
          if (ramsize == 2) { // PRG RAM 128B
            for (int x = 0; x < 128; x++) {
              write_ram_byte(0xF800, x); // PRG RAM ENABLE
              sdBuffer[x] = read_prg_byte(0x4800); // DATA PORT
            }
            myFile.write(sdBuffer, 128);
          }
          else { // SRAM 8K
            for (int i = 0; i < 64; i++) { // Init Register
              write_ram_byte(0xE000, 0);
            }
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              dumpPRG(base, address);
            }
          }
          break;

        case 80: // 1K
          write_prg_byte(0x7EF8, 0xA3); // PRG RAM ENABLE 0
          write_prg_byte(0x7EF9, 0xA3); // PRG RAM ENABLE 1
          for (int x = 0; x < 128; x++) {  // PRG RAM 1K ($7F00-$7FFF) MIRRORED ONCE
            sdBuffer[x] = read_prg_byte(0x7F00 + x);
          }
          myFile.write(sdBuffer, 128);
          write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 0
          write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 1
          break;

        case 82: // 5K
          write_prg_byte(0x7EF7, 0xCA); // PRG RAM ENABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0x69); // PRG RAM ENABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0x84); // PRG RAM ENABLE 2 ($7000-$73FF)
          for (word address = 0x0; address < 0x1400; address += 512) { // PRG RAM 5K ($6000-$73FF)
            dumpMMC5RAM(base, address);
          }
          write_prg_byte(0x7EF7, 0xFF); // PRG RAM DISABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 2 ($7000-$73FF)
          break;

        default:
          if (mapper == 118) // 8K
            write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          else if (mapper == 19) {
            for (int i = 0; i < 64; i++) { // Init Register
              write_ram_byte(0xE000, 0);
            }
          }
          else if ((mapper == 21) || (mapper == 25)) // 8K
            write_prg_byte(0x8000, 0);
          else if (mapper == 26) // 8K
            write_prg_byte(0xB003, 0x80); // PRG RAM ENABLE
          else if (mapper == 68) // 8K
            write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
          else if (mapper == 69) { // 8K
            write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
            write_prg_byte(0xA000, 0xC0); // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
          }
          else if (mapper == 85) // 8K
            write_ram_byte(0xE000, 0x80); // PRG RAM ENABLE
          else if (mapper == 153) // 8K
            write_prg_byte(0x800D, 0x20); // PRG RAM Chip Enable
          for (word address = 0; address < 0x2000; address += 512) { // 8K
            dumpPRG(base, address);
          }
          if (mapper == 85) // 8K
            write_reg_byte(0xE000, 0); // PRG RAM DISABLE [WRITE RAM SAFE]
          break;
      }
      myFile.flush();
      myFile.close();

      println_Msg(F("RAM FILE DUMPED!"));
      println_Msg(F(""));
      display_Update();

      if ((mapper == 16) || (mapper == 159))
        calcCRC(fileName, eepsize, NULL, 0);
      else
        calcCRC(fileName, ram * 1024, NULL, 0);
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  LED_BLUE_OFF;
  LED_GREEN_OFF;
}

void writeRAM() {
  display_Clear();

  if (ramsize == 0) {
    print_Error(F("RAM SIZE 0K"), false);
  }
  else {
    fileBrowser(F("Select RAM File"));
    word base = 0x6000;

    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);

    display_Clear();
    println_Msg(F("Writing File: "));
    println_Msg(filePath);
    println_Msg(fileName);
    display_Update();

    //open file on sd card
    if (myFile.open(filePath, O_READ)) {
      switch (mapper) {
        case 0: // 2K/4K
          for (word address = 0x0; address < (0x800 * ramsize); address += 512) { // 2K/4K
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]); // SWITCH MUST BE IN OFF POSITION
            }
          }
          break;

        case 1:
        case 155:
          banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
          for (int i = 0; i < banks; i++) { // 8K Banks ($6000-$7FFF)
            write_prg_byte(0x8000, 0x80); // Clear Register
            write_mmc1_byte(0x8000, 1 << 3); // PRG ROM MODE 32K
            write_mmc1_byte(0xE000, 0); // PRG RAM ENABLED
            if (banks == 4) // 32K
              write_mmc1_byte(0xA000, i << 2);
            else
              write_mmc1_byte(0xA000, i << 3);
            for (word address = 0x0; address < 0x2000; address += 512) { // 8K
              myFile.read(sdBuffer, 512);
              for (int x = 0; x < 512; x++) {
                write_prg_byte(base + address + x, sdBuffer[x]);
              }
            }
          }
          break;

        case 4: // 1K/8K (MMC6/MMC3)
          if (mmc6) { // MMC6 1K
            write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
            write_prg_byte(0xA001, 0x30); // PRG RAM PROTECT - Enable reading/writing to RAM at $7000-$71FF
            for (word address = 0x1000; address < 0x1200; address += 512) { // 512B
              myFile.read(sdBuffer, 512);
              for (int x = 0; x < 512; x++) {
                write_wram_byte(base + address + x, sdBuffer[x]);
              }
            }
            write_prg_byte(0x8000, 0x20); // PRG RAM ENABLE
            write_prg_byte(0xA001, 0xC0); // PRG RAM PROTECT - Enable reading/writing to RAM at $7200-$73FF
            for (word address = 0x1200; address < 0x1400; address += 512) { // 512B
              myFile.read(sdBuffer, 512);
              for (int x = 0; x < 512; x++) {
                write_wram_byte(base + address + x, sdBuffer[x]);
              }
            }
            write_prg_byte(0x8000, 0x6); // PRG RAM DISABLE
          }
          else { // MMC3 8K
            write_prg_byte(0xA001, 0x80); // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              myFile.read(sdBuffer, 512);
              for (int x = 0; x < 512; x++) {
                write_prg_byte(base + address + x, sdBuffer[x]);
              }
            }
            write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          }
          break;

        case 5: // 8K/16K/32K
          write_prg_byte(0x5100, 3); // 8K PRG Banks
          banks = int_pow(2, ramsize) / 2; // banks = 1,2,4
          if (banks == 2) { // 16K - Split SRAM Chips 8K/8K [ETROM = 16K (ONLY 1ST 8K BATTERY BACKED)]
            for (int i = 0; i < (banks / 2); i++) { // Chip 1
              write_prg_byte(0x5113, i);
              for (word address = 0; address < 0x2000; address += 512) { // 8K
                writeMMC5RAM(base, address);
              }
            }
            for (int j = 4; j < (banks / 2) + 4; j++) { // Chip 2
              write_prg_byte(0x5113, j);
              for (word address = 0; address < 0x2000; address += 512) { // 8K
                writeMMC5RAM(base, address);
              }
            }
          }
          else { // 8K/32K Single SRAM Chip [EKROM = 8K BATTERY BACKED, EWROM = 32K BATTERY BACKED]
            for (int i = 0; i < banks; i++) { // banks = 1 or 4
              write_prg_byte(0x5113, i);
              for (word address = 0; address < 0x2000; address += 512) { // 8K
                writeMMC5RAM(base, address);
              }
            }
          }
          break;

        case 16: // 256-byte EEPROM 24C02
        case 159: // 128-byte EEPROM 24C01 [Little Endian]
          if (mapper == 159)
            eepsize = 128;
          else
            eepsize = 256;
          myFile.read(sdBuffer, eepsize);
          for (word address = 0; address < eepsize; address++) {
            EepromWRITE(address);
            if ((address % 128) == 0)
              display_Clear();
            print_Msg(F("."));
            display_Update();
          }
          break;

        case 19:
          if (ramsize == 2) { // PRG RAM 128B
            myFile.read(sdBuffer, 128);
            for (int x = 0; x < 128; x++) {
              write_ram_byte(0xF800, x); // PRG RAM ENABLE
              write_prg_byte(0x4800, sdBuffer[x]); // DATA PORT
            }
          }
          else { // SRAM 8K
            for (int i = 0; i < 64; i++) { // Init Register
              write_ram_byte(0xF800, 0x40); // PRG RAM WRITE ENABLE
            }
            write_ram_byte(0xF800, 0x40); // PRG RAM WRITE ENABLE
            for (word address = 0; address < 0x2000; address += 512) { // 8K
              myFile.read(sdBuffer, 512);
              for (int x = 0; x < 512; x++) {
                write_prg_byte(base + address + x, sdBuffer[x]);
              }
            }
            write_ram_byte(0xF800, 0x0F); // PRG RAM WRITE PROTECT
          }
          break;

        case 80: // 1K
          write_prg_byte(0x7EF8, 0xA3); // PRG RAM ENABLE 0
          write_prg_byte(0x7EF9, 0xA3); // PRG RAM ENABLE 1
          for (word address = 0x1F00; address < 0x2000; address += 512) { // PRG RAM 1K ($7F00-$7FFF)
            myFile.read(sdBuffer, 128);
            for (int x = 0; x < 128; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 0
          write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 1
          break;

        case 82: // 5K
          write_prg_byte(0x7EF7, 0xCA); // PRG RAM ENABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0x69); // PRG RAM ENABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0x84); // PRG RAM ENABLE 2 ($7000-$73FF)
          for (word address = 0x0; address < 0x1400; address += 1024) { // PRG RAM 5K ($6000-$73FF)
            myFile.read(sdBuffer, 512);
            firstbyte = sdBuffer[0];
            for (int x = 0; x < 512; x++)
              write_prg_byte(base + address + x, sdBuffer[x]);
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++)
              write_prg_byte(base + address + x + 512, sdBuffer[x]);
            write_prg_byte(base + address, firstbyte); // REWRITE 1ST BYTE
          }
          write_prg_byte(0x7EF7, 0xFF); // PRG RAM DISABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0xFF); // PRG RAM DISABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0xFF); // PRG RAM DISABLE 2 ($7000-$73FF)
          break;

        default:
          if (mapper == 118) // 8K
            write_prg_byte(0xA001, 0x80); // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
          else if ((mapper == 21) || (mapper == 25)) // 8K
            write_prg_byte(0x8000, 0);
          else if (mapper == 26) // 8K
            write_prg_byte(0xB003, 0x80); // PRG RAM ENABLE
          //            else if (mapper == 68) // 8K
          //              write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
          else if (mapper == 69) { // 8K
            write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
            write_prg_byte(0xA000, 0xC0); // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
          }
          else if (mapper == 85) // 8K
            write_ram_byte(0xE000, 0x80); // PRG RAM ENABLE
          else if (mapper == 153) // 8K
            write_prg_byte(0x800D, 0x20); // PRG RAM Chip Enable
          for (word address = 0; address < 0x2000; address += 512) { // 8K
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          if (mapper == 118) // 8K
            write_prg_byte(0xA001, 0xC0); // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          else if (mapper == 26) // 8K
            write_prg_byte(0xB003, 0); // PRG RAM DISABLE
          //            else if (mapper == 68) // 8K
          //              write_reg_byte(0xF000, 0x00); // PRG RAM DISABLE [WRITE RAM SAFE]
          else if (mapper == 69) { // 8K
            write_prg_byte(0x8000, 8); // Command Register - PRG Bank 0
            write_prg_byte(0xA000, 0); // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
          }
          else if (mapper == 85) // 8K
            write_reg_byte(0xE000, 0); // PRG RAM DISABLE [WRITE RAM SAFE]
          break;
      }
      myFile.close();
      LED_GREEN_ON;

      println_Msg(F(""));
      println_Msg(F("RAM FILE WRITTEN!"));
      display_Update();

    }
    else {
      print_Error(F("SD ERROR"), true);
    }
  }

  LED_RED_OFF;
  LED_GREEN_OFF;
  sd.chdir(); // root
  filePath[0] = '\0'; // Reset filePath
}

/******************************************
   Eeprom Functions
 *****************************************/
// EEPROM MAPPING
// 00-01 FOLDER #
// 02-05 SNES/GB READER SETTINGS
// 06 LED - ON/OFF [SNES/GB]
// 07 MAPPER
// 08 PRG SIZE
// 09 CHR SIZE
// 10 RAM SIZE

void resetEEPROM() {
  EEPROM_writeAnything(0, 0); // FOLDER #
  EEPROM_writeAnything(2, 0); // CARTMODE
  EEPROM_writeAnything(3, 0); // RETRY
  EEPROM_writeAnything(4, 0); // STATUS
  EEPROM_writeAnything(5, 0); // UNKNOWNCRC
  EEPROM_writeAnything(6, 1); // LED (RESET TO ON)
  EEPROM_writeAnything(7, 0); // MAPPER
  EEPROM_writeAnything(8, 0); // PRG SIZE
  EEPROM_writeAnything(9, 0); // CHR SIZE
  EEPROM_writeAnything(10, 0); // RAM SIZE
}

void EepromStart_NES() {
  write_prg_byte(0x800D, 0x00); // sda low, scl low
  write_prg_byte(0x800D, 0x60); // sda, scl high
  write_prg_byte(0x800D, 0x20); // sda low, scl high
  write_prg_byte(0x800D, 0x00); // START
}

void EepromStop_NES() {
  write_prg_byte(0x800D, 0x00); // sda, scl low
  write_prg_byte(0x800D, 0x20); // sda low, scl high
  write_prg_byte(0x800D, 0x60); // sda, scl high
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x00); // STOP
}

void EepromSet0_NES() {
  write_prg_byte(0x800D, 0x00); // sda low, scl low
  write_prg_byte(0x800D, 0x20); // sda low, scl high // 0
  write_prg_byte(0x800D, 0x00); // sda low, scl low
}

void EepromSet1_NES() {
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x60); // sda high, scl high // 1
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x00); // sda low, scl low
}

void EepromStatus_NES() { // ACK
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x60); // sda high, scl high
  write_prg_byte(0x800D, 0xE0); // sda high, scl high, read high
  byte eepStatus = 1;
  do {
    eepStatus = (read_prg_byte(0x6000) & 0x10) >> 4;
    delayMicroseconds(4);
  }
  while (eepStatus == 1);
  write_prg_byte(0x800D, 0x40); // sda high, scl low
}

void EepromReadData_NES() {
  // read serial data into buffer
  for (int i = 0; i < 8; i++) {
    write_prg_byte(0x800D, 0x60); // sda high, scl high, read low
    write_prg_byte(0x800D, 0xE0); // sda high, scl high, read high
    eepbit[i] = (read_prg_byte(0x6000) & 0x10) >> 4; // Read 0x6000 with Mask 0x10 (bit 4)
    write_prg_byte(0x800D, 0x40); // sda high, scl low
  }
}

void EepromDevice_NES() { // 24C02 ONLY
  EepromSet1_NES();
  EepromSet0_NES();
  EepromSet1_NES();
  EepromSet0_NES();
  EepromSet0_NES(); // A2
  EepromSet0_NES(); // A1
  EepromSet0_NES(); // A0
}

void EepromReadMode_NES() {
  EepromSet1_NES(); // READ
  EepromStatus_NES(); // ACK
}
void EepromWriteMode_NES() {
  EepromSet0_NES(); // WRITE
  EepromStatus_NES(); // ACK
}

void EepromFinish_NES() {
  write_prg_byte(0x800D, 0x00); // sda low, scl low
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x60); // sda high, scl high
  write_prg_byte(0x800D, 0x40); // sda high, scl low
  write_prg_byte(0x800D, 0x00); // sda low, scl low
}

void EepromSetAddress01(byte address) { // 24C01 [Little Endian]
  for (int i = 0; i < 7; i++) {
    if (address & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    address >>= 1; // rotate to the next bit
  }
}

void EepromSetAddress02(byte address) { // 24C02
  for (int i = 0; i < 8; i++) {
    if ((address >> 7) & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    address <<= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromWriteData01() { // 24C01 [Little Endian]
  for (int i = 0; i < 8; i++) {
    if (eeptemp & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    eeptemp >>= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromWriteData02() { // 24C02
  for (int i = 0; i < 8; i++) {
    if ((eeptemp >> 7) & 0x1) // Bit is HIGH
      EepromSet1_NES();
    else // Bit is LOW
      EepromSet0_NES();
    eeptemp <<= 1; // rotate to the next bit
  }
  EepromStatus_NES(); // ACK
}

void EepromREAD(byte address) {
  EepromStart_NES(); // START
  if (mapper == 159) { // 24C01
    EepromSetAddress01(address); // 24C01 [Little Endian]
    EepromReadMode_NES();
    EepromReadData_NES();
    EepromFinish_NES();
    EepromStop_NES(); // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[7] << 7 | eepbit[6] << 6 | eepbit[5] << 5 | eepbit[4] << 4 | eepbit[3] << 3 | eepbit[2] << 2 | eepbit[1] << 1 | eepbit[0];
  }
  else { // 24C02
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromStart_NES(); // START
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromReadMode_NES();
    EepromReadData_NES();
    EepromFinish_NES();
    EepromStop_NES(); // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }
  sdBuffer[address] = eeptemp;
}

void EepromWRITE(byte address) {
  eeptemp = sdBuffer[address];
  EepromStart_NES(); // START
  if (mapper == 159) { // 24C01
    EepromSetAddress01(address); // 24C01 [Little Endian]
    EepromWriteMode_NES();
    EepromWriteData01(); // 24C01 [Little Endian]
  }
  else { // 24C02
    EepromDevice_NES(); // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromWriteData02();
  }
  EepromStop_NES(); // STOP
}

/******************************************
   NESmaker Flash Cart [SST 39SF40]
 *****************************************/
void NESmaker_ResetFlash() { // Reset Flash
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xFF); // Reset
}

// SST 39SF040 Software ID
void NESmaker_ID() { // Read Flash ID
  NESmaker_ResetFlash();
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x90); // Software ID Entry
  unsigned char ID1 = read_prg_byte(0x8000);
  unsigned char ID2 = read_prg_byte(0x8001);
  sprintf(flashid, "%02X%02X", ID1, ID2);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xF0); // Software ID Exit
  if (strcmp(flashid, "BFB7") == 0) // SST 39SF040
    flashfound = 1;
}

void NESmaker_SectorErase(byte bank, word address) {
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x80);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, bank); // $00-$1F
  write_prg_byte(address, 0x30); // Sector Erase ($8000/$9000/$A000/$B000)
}

void NESmaker_ByteProgram(byte bank, word address, byte data) {
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xA0);
  write_prg_byte(0xC000, bank); // $00-$1F
  write_prg_byte(address, data); // $8000-$BFFF
}

// SST 39SF040 Chip Erase [NOT IMPLEMENTED]
void NESmaker_ChipErase() { // Typical 70ms
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x80);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0x10); // Chip Erase
}

void writeFLASH() {
  display_Clear();
  if (!flashfound) {
    LED_RED_ON;
    println_Msg(F("FLASH NOT DETECTED"));
    display_Update();
  }
  else {
    print_Msg(F("Flash ID: "));
    println_Msg(flashid);
    println_Msg(F(""));
    println_Msg(F("NESmaker Flash Found"));
    println_Msg(F(""));
    display_Update();
    delay(100);

    fileBrowser(F("Select FLASH File"));
    word base = 0x8000;

    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);

    LED_RED_ON;
    display_Clear();
    println_Msg(F("Writing File: "));
    println_Msg(filePath);
    println_Msg(fileName);
    display_Update();

    //open file on sd card
    if (myFile.open(filePath, O_READ)) {
      banks = int_pow(2, prgsize); // 256K/512K
      for (int i = 0; i < banks; i++) { // 16K Banks
        for (word sector = 0; sector < 0x4000; sector += 0x1000) { // 4K Sectors ($8000/$9000/$A000/$B000)
          // Sector Erase
          NESmaker_SectorErase(i, base + sector);
          delay(18); // Typical 18ms
          for (byte j = 0; j < 2; j++) { // Confirm erase twice
            do {
              bytecheck = read_prg_byte(base + sector);
              delay(18);
            }
            while (bytecheck != 0xFF);
          }
          // Program Byte
          for (word addr = 0x0; addr < 0x1000; addr += 512) {
            myFile.read(sdBuffer, 512);
            for (int x = 0; x < 512; x++) {
              word location = base + sector + addr + x;
              NESmaker_ByteProgram(i, base + sector + addr + x, sdBuffer[x]);
              delayMicroseconds(14); // Typical 14us
              for (byte k = 0; k < 2; k++) { // Confirm write twice
                do {
                  bytecheck = read_prg_byte(base + sector + addr + x);
                  delayMicroseconds(14);
                }
                while (bytecheck != sdBuffer[x]);
              }
            }
          }
        }

#if defined(enable_OLED)
        display.print(F("*"));
        display.display();
#elif  defined(enable_LCD)
        display.print(F("*"));
        display.updateDisplay();
#else
        Serial.print(F("*"));
        if ((i != 0) && ((i + 1) % 16 == 0))
          Serial.println(F(""));
#endif
      }
      myFile.close();
      LED_GREEN_ON;

      println_Msg(F(""));
      println_Msg(F("FLASH FILE WRITTEN!"));
      display_Update();
    }
    else {
      LED_RED_ON;
      println_Msg(F("SD ERROR"));
      display_Update();
    }
  }
  display_Clear();
  LED_RED_OFF;
  LED_GREEN_OFF;
  sd.chdir(); // root
  filePath[0] = '\0'; // Reset filePath
}
#endif
//******************************************
// End of File
//******************************************
