//******************************************
// NES MODULE
//******************************************
// mostly copy&pasted from "Famicom Dumper" 2019-08-31 by skaman
// also based on "CoolArduino" by HardWareMan
// Pinout changes: LED and CIRAM_A10

#ifdef ENABLE_NES

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

struct mapper_NES {
  uint16_t mapper;
  uint8_t prglo;
  uint8_t prghi;
  uint8_t chrlo;
  uint8_t chrhi;
  uint8_t ramlo;
  uint8_t ramhi;
};

/******************************************
  Supported Mappers
 *****************************************/
// Supported Mapper Array (iNES Mapper #s)
// Format = {mapper,prglo,prghi,chrlo,chrhi,ramlo,ramhi}
static const struct mapper_NES PROGMEM mapsize[] = {
  { 0, 0, 1, 0, 1, 0, 2 },   // nrom                                                [sram r/w]
  { 1, 1, 5, 0, 5, 0, 3 },   // mmc1                                                [sram r/w]
  { 2, 2, 4, 0, 0, 0, 0 },   // uxrom
  { 3, 0, 1, 0, 3, 0, 0 },   // cnrom
  { 4, 1, 5, 0, 6, 0, 1 },   // mmc3/mmc6                                           [sram/prgram r/w]
  { 5, 3, 5, 5, 7, 0, 3 },   // mmc5                                                [sram r/w]
  { 7, 2, 4, 0, 0, 0, 0 },   // axrom
  { 9, 3, 3, 5, 5, 0, 0 },   // mmc2 (punch out)
  { 10, 3, 4, 4, 5, 1, 1 },  // mmc4                                               [sram r/w]
  { 11, 1, 3, 1, 5, 0, 0 },  // Color Dreams [UNLICENSED]
  { 13, 1, 1, 0, 0, 0, 0 },  // cprom (videomation)
  { 15, 6, 6, 0, 0, 0, 0 },  // K-1029/K-1030P [UNLICENSED]
  { 16, 3, 4, 5, 6, 0, 1 },  // bandai x24c02                                      [eep r/w]
  { 18, 3, 4, 5, 6, 0, 1 },  // jaleco ss8806                                      [sram r/w]
  { 19, 3, 4, 5, 6, 0, 1 },  // namco 106/163                                      [sram/prgram r/w]
  // 20 - bad mapper, not used
  { 21, 4, 4, 5, 6, 0, 1 },  // vrc4a/vrc4c                                        [sram r/w]
  { 22, 3, 3, 5, 5, 0, 0 },  // vrc2a
  { 23, 3, 3, 5, 6, 0, 0 },  // vrc2b/vrc4e
  { 24, 4, 4, 5, 5, 0, 0 },  // vrc6a (akumajou densetsu)
  { 25, 3, 4, 5, 6, 0, 1 },  // vrc2c/vrc4b/vrc4d                                  [sram r/w]
  { 26, 4, 4, 5, 6, 1, 1 },  // vrc6b                                              [sram r/w]
  { 28, 5, 7, 0, 0, 0, 0 },  // Action 53 [UNLICENSED]
  { 30, 4, 5, 0, 0, 0, 0 },  // unrom 512 (NESmaker) [UNLICENSED]
  { 31, 6, 6, 0, 0, 0, 0 },  // NSF music compilations [UNLICENSED]
  { 32, 3, 4, 5, 5, 0, 0 },  // irem g-101
  { 33, 3, 4, 5, 6, 0, 0 },  // taito tc0190
  { 34, 1, 8, 0, 4, 0, 0 },  // BxROM & NINA
  { 35, 0, 7, 1, 8, 0, 0 },  // J.Y. Company ASIC [UNLICENSED]
  { 36, 0, 3, 1, 5, 0, 0 },  // TXC 01-22000-400 Board [UNLICENSED]
  { 37, 4, 4, 6, 6, 0, 0 },  // (super mario bros + tetris + world cup)
  { 38, 1, 3, 0, 3, 0, 0 },  // Crime Busters [UNLICENSED]
  { 42, 0, 3, 0, 5, 0, 0 },  // hacked FDS games converted to cartridge [UNLICENSED]
  { 45, 3, 6, 0, 8, 0, 0 },  // ga23c asic multicart [UNLICENSED]
  { 46, 1, 6, 0, 8, 0, 0 },  // Rumble Station [UNLICENSED]
  { 47, 4, 4, 6, 6, 0, 0 },  // (super spike vball + world cup)
  { 48, 3, 4, 6, 6, 0, 0 },  // taito tc0690
  { 52, 0, 3, 0, 3, 0, 0 },  // Realtec 8213 [UNLICENSED]
  { 56, 0, 7, 0, 6, 0, 0 },  // KS202 [UNLICENSED]
  { 57, 0, 3, 0, 5, 0, 0 },  // BMC-GKA [UNLICENSED]
  { 58, 1, 6, 1, 6, 0, 0 },  // BMC-GKB (C)NROM-based multicarts, duplicate of mapper 213 [UNLICENSED]
  { 59, 0, 3, 0, 4, 0, 0 },  // BMC-T3H53 & BMC-D1038 [UNLICENSED]
  { 60, 2, 2, 3, 3, 0, 0 },  // Reset-based NROM-128 4-in-1 multicarts [UNLICENSED]
  { 62, 7, 7, 8, 8, 0, 0 },  // K-1017P [UNLICENSED]
  { 63, 8, 8, 0, 0, 0, 0 },  // NTDEC "Powerful" multicart, 3072K [UNLICENSED]
  { 64, 2, 3, 4, 5, 0, 0 },  // tengen rambo-1 [UNLICENSED]
  { 65, 3, 4, 5, 6, 0, 0 },  // irem h-3001
  { 66, 2, 3, 2, 3, 0, 0 },  // gxrom/mhrom
  { 67, 3, 3, 5, 5, 0, 0 },  // sunsoft 3
  { 68, 3, 3, 5, 6, 0, 1 },  // sunsoft 4                                          [sram r/w]
  { 69, 3, 4, 5, 6, 0, 1 },  // sunsoft fme-7/5a/5b                                [sram r/w]
  { 70, 3, 3, 5, 5, 0, 0 },  // bandai
  { 71, 2, 4, 0, 0, 0, 0 },  // camerica/codemasters [UNLICENSED]
  { 72, 3, 3, 5, 5, 0, 0 },  // jaleco jf-17
  { 73, 3, 3, 0, 0, 0, 0 },  // vrc3 (salamander)
  { 75, 3, 3, 5, 5, 0, 0 },  // vrc1
  { 76, 3, 3, 5, 5, 0, 0 },  // namco 109 variant (megami tensei: digital devil story)
  { 77, 3, 3, 3, 3, 0, 0 },  // (napoleon senki)
  { 78, 3, 3, 5, 5, 0, 0 },  // irem 74hc161/32
  { 79, 1, 2, 2, 3, 0, 0 },  // NINA-03/06 by AVE [UNLICENSED]
  { 80, 3, 3, 5, 6, 0, 1 },  // taito x1-005                                       [prgram r/w]
  { 82, 3, 3, 5, 6, 0, 1 },  // taito x1-017                                       [prgram r/w]
  // 84 - bad mapper, not used
  { 85, 3, 5, 0, 5, 0, 1 },  // vrc7                                               [sram r/w]
  { 86, 3, 3, 4, 4, 0, 0 },  // jaleco jf-13 (moero pro yakyuu)
  { 87, 0, 1, 2, 3, 0, 0 },  // Jaleco/Konami CNROM (DIS_74X139X74)
  { 88, 3, 3, 5, 5, 0, 0 },  // namco (dxrom variant)
  { 89, 3, 3, 5, 5, 0, 0 },  // sunsoft 2 variant (tenka no goikenban: mito koumon)
  { 90, 0, 7, 1, 8, 0, 0 },  // J.Y. Company ASIC [UNLICENSED]
  { 91, 3, 5, 7, 8, 0, 0 },  // JY830623C/YY840238C boards [UNLICENSED]
  { 92, 4, 4, 5, 5, 0, 0 },  // jaleco jf-19/jf-21
  { 93, 3, 3, 0, 0, 0, 0 },  // sunsoft 2
  { 94, 3, 3, 0, 0, 0, 0 },  // hvc-un1rom (senjou no ookami)
  { 95, 3, 3, 3, 3, 0, 0 },  // namcot-3425 (dragon buster)
  { 96, 3, 3, 0, 0, 0, 0 },  // (oeka kids)
  { 97, 4, 4, 0, 0, 0, 0 },  // irem tam-s1 (kaiketsu yanchamaru)
  // 100 - bad mapper, not used
  // 101 - bad mapper, not used
  { 105, 4, 4, 0, 0, 0, 0 },  // (nintendo world Championships 1990) [UNTESTED]
  { 111, 5, 5, 0, 0, 0, 0 },  // GTROM [UNLICENSED]
  { 113, 1, 4, 0, 5, 0, 0 },  // NINA-03/06 [UNLICENSED]
  { 114, 3, 4, 5, 6, 0, 0 },  // SuperGame MMC3-clone [UNLICENSED]
  { 118, 3, 4, 5, 5, 0, 1 },  // txsrom/mmc3                                       [sram r/w]
  { 119, 3, 3, 4, 4, 0, 0 },  // tqrom/mmc3
  { 126, 1, 8, 0, 8, 0, 0 },  // MMC3-based multicart (PJ-008, AT-207) [UNLICENSED]
  { 134, 1, 8, 0, 8, 0, 0 },  // T4A54A, WX-KB4K, or BS-5652 [UNLICENSED]
  { 140, 3, 3, 3, 5, 0, 0 },  // jaleco jf-11/jf-14
  { 142, 1, 3, 0, 0, 0, 0 },  // UNL-KS7032 [UNLICENSED]
  { 146, 1, 2, 2, 3, 0, 0 },  // Sachen 3015 [UNLICENSED]
  { 148, 1, 2, 0, 4, 0, 0 },  // Sachen SA-0037 & Tengen 800008 [UNLICENSED]
  // 151 - bad mapper, not used
  { 152, 2, 3, 5, 5, 0, 0 },  // BANDAI-74*161/161/32
  { 153, 5, 5, 0, 0, 1, 1 },  // (famicom jump ii)                                 [sram r/w]
  { 154, 3, 3, 5, 5, 0, 0 },  // namcot-3453 (devil man)
  { 155, 3, 3, 3, 5, 0, 1 },  // mmc1 variant                                      [sram r/w]
  { 157, 4, 4, 0, 0, 0, 0 },  // Datach
  { 158, 3, 3, 5, 5, 0, 0 },  // tengen rambo-1 variant (alien syndrome (u)) [UNLICENSED]
  { 159, 3, 4, 5, 6, 1, 1 },  // bandai x24c01                                     [eep r/w]
  { 162, 6, 7, 0, 0, 0, 0 },  // Waixing FS304 [UNLICENSED]
  { 163, 6, 7, 0, 0, 0, 0 },  // Nanjing FC-001 [UNLICENSED]
  { 174, 3, 3, 4, 4, 0, 0 },  // NTDEC 5-in-1 [UNLICENSED]
  { 176, 4, 4, 5, 5, 0, 0 },  // 8025 enhanced MMC3 [UNLICENSED]
  { 177, 1, 7, 0, 0, 0, 0 },  // Henggedianzi Super Rich PCB [UNLICENSED]
  { 178, 5, 5, 0, 0, 0, 0 },  // some Waixing PCBs [UNLICENSED]
  { 180, 3, 3, 0, 0, 0, 0 },  // unrom variant (crazy climber)
  { 184, 1, 1, 2, 3, 0, 0 },  // sunsoft 1
  { 185, 0, 1, 1, 1, 0, 0 },  // cnrom lockout
  // 186 - bad mapper, not used
  { 200, 1, 4, 1, 4, 0, 0 },  // HN-02 multicarts [UNLICENSED]
  { 201, 1, 8, 1, 9, 0, 0 },  // NROM-256 multicarts [UNLICENSED]
  { 202, 0, 3, 1, 4, 0, 0 },  // BMC-150IN1 multicarts [UNLICENSED]
  { 203, 1, 4, 1, 4, 0, 0 },  // various NROM-128 multicarts [UNLICENSED]
  { 206, 1, 3, 2, 4, 0, 0 },  // dxrom
  { 207, 4, 4, 5, 5, 0, 0 },  // taito x1-005 variant (fudou myouou den)
  { 209, 0, 7, 1, 8, 0, 0 },  // J.Y. Company ASIC [UNLICENSED]
  { 210, 3, 5, 5, 6, 0, 0 },  // namco 175/340
  { 211, 0, 7, 1, 8, 0, 0 },  // J.Y. Company ASIC [UNLICENSED]
  { 212, 0, 3, 0, 4, 0, 0 },  // BMC Super HiK 300-in-1 [UNLICENSED]
  { 213, 1, 6, 1, 6, 0, 0 },  // BMC-GKB (C)NROM-based multicarts, duplicate of mapper 58 [UNLICENSED]
  { 214, 0, 3, 0, 4, 0, 0 },  // BMC-SUPERGUN-20IN1, BMC-190IN1 [UNLICENSED]
  { 225, 4, 7, 5, 8, 0, 0 },  // ET-4310 (FC) + K-1010 (NES) [UNLICENSED]
  { 226, 6, 7, 0, 0, 0, 0 },  // BMC-76IN1, BMC-SUPER42IN1, BMC-GHOSTBUSTERS63IN1 [UNLICENSED]
  { 227, 1, 5, 0, 0, 0, 0 },  // 810449-C-A1 / FW-01 [UNLICENSED]
  { 228, 4, 7, 5, 7, 0, 0 },  // Action 52 + Cheetahmen II [UNLICENSED]
  { 229, 5, 5, 6, 6, 0, 0 },  // BMC 31-IN-1 [UNLICENSED]
  { 232, 4, 4, 0, 0, 0, 0 },  // Camerica/Codemasters "Quattro" cartridges [UNLICENSED]
  { 235, 6, 8, 0, 0, 0, 0 },  // "Golden Game" multicarts [UNLICENSED]
  { 236, 0, 6, 0, 5, 0, 0 },  // Realtec 8031, 8099, 8106, 8155 [UNLICENSED]
  { 240, 1, 5, 1, 5, 0, 3 },  // C&E Bootleg Board (Sheng Huo Lie Zhuan, Jing Ke Xin Zhuan) [UNLICENSED]
  { 241, 3, 5, 0, 0, 0, 0 },  // BxROM with WRAM [UNLICENSED]
  { 242, 5, 5, 0, 0, 0, 0 },  // ET-113 [UNLICENSED]
  { 246, 5, 5, 7, 7, 0, 0 },  // C&E Feng Shen Bang [UNLICENSED]
  // 248 - bad mapper, not used
  { 255, 4, 7, 5, 8, 0, 0 },  // 110-in-1 multicart (same as 225) [UNLICENSED]
  { 446, 0, 8, 0, 0, 0, 0 }   // Mindkids SMD172B_FGPA submapper 0 & 1
};

const char _file_name_no_number_fmt[] PROGMEM = "%s.%s";
const char _file_name_with_number_fmt[] PROGMEM = "%s.%02d.%s";

/******************************************
  Defines
 *****************************************/
#define ROMSEL_HI PORTF |= (1 << 1)
#define ROMSEL_LOW PORTF &= ~(1 << 1)
#define PHI2_HI PORTF |= (1 << 0)
#define PHI2_LOW PORTF &= ~(1 << 0)
#define PRG_READ PORTF |= (1 << 7)
#define PRG_WRITE PORTF &= ~(1 << 7)
#define CHR_READ_HI PORTF |= (1 << 5)
#define CHR_READ_LOW PORTF &= ~(1 << 5)
#define CHR_WRITE_HI PORTF |= (1 << 2)
#define CHR_WRITE_LOW PORTF &= ~(1 << 2)

#define MODE_READ \
  { \
    PORTK = 0xFF; \
    DDRK = 0; \
  }
#define MODE_WRITE DDRK = 0xFF

#define press 1
#define doubleclick 2
#define hold 3
#define longhold 4

/******************************************
  Variables
*****************************************/
// Mapper
uint8_t mapcount = (sizeof(mapsize) / sizeof(mapsize[0]));
uint16_t mapselect;

const uint16_t PRG[] PROGMEM = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
uint8_t prglo = 0;   // Lowest Entry
uint8_t prghi = 11;  // Highest Entry

const uint16_t CHR[] PROGMEM = { 0, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
uint8_t chrlo = 0;   // Lowest Entry
uint8_t chrhi = 10;  // Highest Entry

const uint8_t RAM[] PROGMEM = { 0, 8, 16, 32 };
uint8_t ramlo = 0;  // Lowest Entry
uint8_t ramhi = 3;  // Highest Entry

uint16_t prg;
uint16_t chr;
uint8_t ram;
bool mmc6 = false;
bool flashfound = false;  // NESmaker 39SF040 Flash Cart

// Cartridge Config
uint16_t mapper;
uint8_t prgsize;
uint8_t chrsize;
uint8_t ramsize;

/******************************************
  Menus
*****************************************/
// NES start menu
static const char nesMenuItem1[] PROGMEM = "Read iNES Rom";
static const char nesMenuItem2[] PROGMEM = "Read PRG/CHR";
static const char nesMenuItem5[] PROGMEM = "Change Mapper";
static const char nesMenuItem6[] PROGMEM = "Flash Repro";
static const char* const menuOptionsNES[] PROGMEM = { nesMenuItem1, nesMenuItem2, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, nesMenuItem5, nesMenuItem6, FSTRING_RESET };

// NES chips menu
static const char nesChipsMenuItem1[] PROGMEM = "Combined PRG+CHR";
static const char nesChipsMenuItem2[] PROGMEM = "Read only PRG";
static const char nesChipsMenuItem3[] PROGMEM = "Read only CHR";
static const char nesChipsMenuItem4[] PROGMEM = "Back";
static const char* const menuOptionsNESChips[] PROGMEM = { nesChipsMenuItem1, nesChipsMenuItem2, nesChipsMenuItem3, nesChipsMenuItem4 };

#if defined(ENABLE_FLASH)
// Repro Writer Menu
static const char nesFlashMenuItem1[] PROGMEM = "Flash NesMaker";
static const char nesFlashMenuItem2[] PROGMEM = "Flash A29040B-MAPPER0";
static const char nesFlashMenuItem3[] PROGMEM = "Back";
static const char* const menuOptionsNESFlash[] PROGMEM = { nesFlashMenuItem1, nesFlashMenuItem2, nesFlashMenuItem3 };
#endif

// NES start menu
void nesMenu() {
  unsigned char answer;

  // create menu with title "NES CART READER" and 7 options to choose from
  convertPgm(menuOptionsNES, 7);
  answer = question_box(F("NES CART READER"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (answer) {
    // Read Rom
    case 0:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readRom_NES();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
#ifdef ENABLE_GLOBAL_LOG
      save_log();
#endif
      display_Update();
      wait();
      break;

    // Read single chip
    case 1:
      nesChipMenu();
      break;

    // Read RAM
    case 2:
      sd.chdir();
      sprintf(folder, "NES/SAVE");
      sd.mkdir(folder, true);
      sd.chdir(folder);
      readRAM();
      resetROM();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    // Write RAM
    case 3:
      writeRAM();
      resetROM();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    // Change Mapper
    case 4:
      setDefaultRomName();
      setMapper();
      checkMapperSize();
      setPRGSize();
      setCHRSize();
      setRAMSize();
      checkStatus_NES();
      break;

#if defined(ENABLE_FLASH)
    // Write FLASH
    case 5:
      nesFlashMenu();
      break;
#endif

    // Reset
    case 6:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
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
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readRaw_NES();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
#ifdef ENABLE_GLOBAL_LOG
      save_log();
#endif
      display_Update();
      wait();
      break;

    // Read PRG
    case 1:
      CreateROMFolderInSD();
      readPRG(false);
      resetROM();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    // Read CHR
    case 2:
      CreateROMFolderInSD();
      readCHR(false);
      resetROM();
      println_Msg(FS(FSTRING_EMPTY));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
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

#if defined(ENABLE_FLASH)
void nesFlashMenu() {
  // create menu with title "Select NES Flash Repro" and 3 options to choose from
  convertPgm(menuOptionsNESFlash, 3);
  unsigned char answer = question_box(F("Select Flash Writer"), menuOptions, 3, 0);
  switch (answer) {
    case 0:

      if (mapper == 30) {
        writeFLASH();
        resetROM();
      } else {
        display_Clear();
        println_Msg(FS(string_error5));
        println_Msg(F("Can't write to this cartridge"));
        println_Msg(FS(FSTRING_EMPTY));
        // Prints string out of the common strings array either with or without newline
        print_STR(press_button_STR, 1);
        display_Update();
      }
      wait();
      break;
    case 1:
      if (mapper == 0) {
        display_Clear();
        A29040B_writeFLASH();
        display_Update();
        wait();
      } else {
        display_Clear();
        println_Msg(FS(string_error5));
        println_Msg(F("Can't write to this cartridge"));
        println_Msg(mapper);
        // Prints string out of the common strings array either with or without newline
        print_STR(press_button_STR, 1);
        display_Update();
        wait();
      }

      break;
    // Return to Main Menu
    case 2:
      nesMenu();
      wait();
      break;
  }
}
#endif

/******************************************
   Setup
 *****************************************/
void setup_NES() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

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
  rgbLed(black_color);
}

/******************************************
   Get Mapping from SD database
 *****************************************/
uint32_t uppow2(uint32_t n) {
  for (int8_t x = 31; x >= 0; x--)
    if (n & (1u << x)) {
      if ((1u << x) != n)
        return (1u << (x + 1));
      break;
    }
  return n;
}

struct database_entry {
  char filename[128];
  char crc_str[8 + 1 + 8 + 1 + 32 + 1];
  uint32_t crc;
  char* crc512_str;
  uint32_t crc512;
  char* iNES_str;
};

void printPRG(unsigned long myOffset) {
  display_Clear();
  print_Msg(F("Printing PRG at "));
  println_Msg(myOffset);

  char myBuffer[3];

  for (size_t currLine = 0; currLine < 512; currLine += 16) {
    for (uint8_t currByte = 0; currByte < 16; currByte++) {
      itoa(read_prg_byte(myOffset + currLine + currByte), myBuffer, 16);
      for (size_t i = 0; i < 2 - strlen(myBuffer); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(myBuffer);
      print_Msg(" ");
    }
    println_Msg("");
  }
  display_Update();
}

void setDefaultRomName() {
  romName[0] = 'C';
  romName[1] = 'A';
  romName[2] = 'R';
  romName[3] = 'T';
  romName[4] = '\0';
}

void setRomnameFromString(const char* input) {
  uint8_t myLength = 0;
  for (uint8_t i = 0; i < 20 && myLength < 15; i++) {
    // Stop at first "(" to remove "(Country)"
    if (input[i] == '(') {
      break;
    }
    if (
      (input[i] >= '0' && input[i] <= '9') || (input[i] >= 'A' && input[i] <= 'Z') || (input[i] >= 'a' && input[i] <= 'z')) {
      romName[myLength++] = input[i];
    }
  }

  // If name consists out of all japanese characters use CART as name
  if (myLength == 0) {
    setDefaultRomName();
  }
}

void printDataLine_NES(void* entry) {
  struct database_entry* castEntry = (struct database_entry*)entry;
  uint8_t iNES[16];
  uint8_t* output;
  char* input;

  input = castEntry->iNES_str;
  output = iNES;
  for (uint8_t i = 0; i < sizeof(iNES); i++) {
    unsigned int buf;

    sscanf(input, "%2X", &buf);
    *(output++) = buf;
    input += 2;
  }

  mapper = (iNES[6] >> 4) | (iNES[7] & 0xF0) | ((iNES[8] & 0x0F) << 8);

  if ((iNES[9] & 0x0F) != 0x0F) {
    // simple notation
    prgsize = (iNES[4] | ((iNES[9] & 0x0F) << 8));  //*16
  } else {
    // exponent-multiplier notation
    prgsize = (((1 << (iNES[4] >> 2)) * ((iNES[4] & 0b11) * 2 + 1)) >> 14);  //*16
  }
  if (prgsize != 0)
    prgsize = (int(log(prgsize) / log(2)));

  if ((iNES[9] & 0xF0) != 0xF0) {
    // simple notation
    chrsize = (uppow2(iNES[5] | ((iNES[9] & 0xF0) << 4))) * 2;  //*4
  } else {
    // exponent-multiplier notation
    chrsize = (((1 << (iNES[5] >> 2)) * ((iNES[5] & 0b11) * 2 + 1)) >> 13) * 2;  //*4
  }
  if (chrsize != 0)
    chrsize = (int(log(chrsize) / log(2)));

  ramsize = ((iNES[10] & 0xF0) ? (64 << ((iNES[10] & 0xF0) >> 4)) : 0) / 4096;  //*4
  if (ramsize != 0)
    ramsize = (int(log(ramsize) / log(2)));

  prg = (int_pow(2, prgsize)) * 16;
  if (chrsize == 0)
    chr = 0;  // 0K
  else
    chr = (int_pow(2, chrsize)) * 4;
  if (ramsize == 0)
    ram = 0;  // 0K
  else if (mapper == 82)
    ram = 5;  // 5K
  else
    ram = (int_pow(2, ramsize)) * 4;

  // Mapper Variants
  // Identify variant for use across multiple functions
  if (mapper == 4) {  // Check for MMC6/MMC3
    checkMMC6();
    if (mmc6)
      ram = 1;  // 1K
  }
  printNESSettings();
}

void getMapping() {
  FsFile database;
  uint32_t oldcrc32 = 0xFFFFFFFF;
  uint32_t oldcrc32MMC3 = 0xFFFFFFFF;
  char crcStr[9];

  display_Clear();

  sd.chdir();
  if (!database.open("nes.txt", O_READ)) {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
    // never reached
  }

  // Read first 512 bytes of first and last block of PRG ROM and compute CRC32
  // MMC3 maps the last 8KB block of PRG ROM to 0xE000 while 0x8000 can contain random data after bootup
  for (size_t c = 0; c < 512; c++) {
    UPDATE_CRC(oldcrc32, read_prg_byte(0x8000 + c));
    UPDATE_CRC(oldcrc32MMC3, read_prg_byte(0xE000 + c));
  }
  oldcrc32 = ~oldcrc32;
  oldcrc32MMC3 = ~oldcrc32MMC3;
  bool browseDatabase;

  // Filter out all 0xFF checksums at 0x8000 and 0xE000
  if (oldcrc32 == 0xBD7BC39F && oldcrc32MMC3 == 0xBD7BC39F) {
    println_Msg(F("No data found."));
    println_Msg(F("Using manual selection"));
    display_Update();
    delay(500);
    setDefaultRomName();
    browseDatabase = selectMapping(database);
  } else {
    println_Msg(F("Searching database"));
    print_Msg(F("for "));
    sprintf(crcStr, "%08lX", oldcrc32);
    print_Msg(crcStr);
    if (oldcrc32 != oldcrc32MMC3) {
      print_Msg(F(" or "));
      sprintf(crcStr, "%08lX", oldcrc32MMC3);
      print_Msg(crcStr);
    }
    println_Msg(F("..."));
    display_Update();
    while (database.available()) {
      struct database_entry entry;

      readDatabaseEntry(database, &entry);
      //if checksum search was successful set mapper and end search, also filter out 0xFF checksum
      if (
        entry.crc512 != 0xBD7BC39F && (entry.crc512 == oldcrc32 || entry.crc512 == oldcrc32MMC3)) {
        // Rewind to start of entry
        rewind_line(database, 3);
        break;
      }
    }
    if (database.available()) {
      browseDatabase = true;
    } else {
      // File searched until end but nothing found
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("CRC not found in database"));
      println_Msg(F("Using manual selection"));
      display_Update();
      delay(500);
      // Print content of PRG for debugging
      //printPRG(0x8000);
      //printPRG(0xE000);

      // Change ROM name to CART
      setDefaultRomName();
      browseDatabase = selectMapping(database);
    }
  }
  if (browseDatabase) {
    struct database_entry entry;

    if (checkCartSelection(database, &readDataLine_NES, &entry, &printDataLine_NES, &setRomnameFromString)) {
      // anything else: select current record
      // Save Mapper
      EEPROM_writeAnything(7, mapper);
      EEPROM_writeAnything(9, prgsize);
      EEPROM_writeAnything(10, chrsize);
      EEPROM_writeAnything(11, ramsize);
    }
  }
  database.close();
}

static void readDatabaseEntry(FsFile& database, struct database_entry* entry) {
  get_line(entry->filename, &database, sizeof(entry->filename));
  readDataLine_NES(database, entry);
  skip_line(&database);
}

void readDataLine_NES(FsFile& database, void* e) {
  struct database_entry* entry = (database_entry*)e;
  get_line(entry->crc_str, &database, sizeof(entry->crc_str));

  entry->crc_str[8] = 0;
  entry->crc512_str = &entry->crc_str[8 + 1];
  entry->crc512_str[8] = 0;
  entry->iNES_str = &entry->crc_str[8 + 1 + 8 + 1];

  // Convert "4E4553" to (0x4E, 0x45, 0x53)
  unsigned int iNES_BUF;
  for (uint8_t j = 0; j < 16; j++) {
    sscanf(entry->iNES_str + j * 2, "%2X", &iNES_BUF);
    iNES_HEADER[j] = iNES_BUF;
  }

  entry->crc = strtoul(entry->crc_str, NULL, 16);
  entry->crc512 = strtoul(entry->crc512_str, NULL, 16);
}

bool selectMapping(FsFile& database) {
  // Select starting letter
  byte myLetter = starting_letter();

  if (myLetter == 27) {
    // Change Mapper
    setMapper();
    checkMapperSize();
    setPRGSize();
    setCHRSize();
    setRAMSize();
    return 0;
  } else {
    seek_first_letter_in_database(database, myLetter);
  }
  return 1;
}

void read_NES(const char* fileSuffix, const byte* header, const uint8_t headersize, const boolean renamerom) {
  // Get name, add extension and convert to char array for sd lib
  createFolderAndOpenFile("NES", "ROM", romName, fileSuffix);

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(headersize + prgsize * 16 * 1024 + chrsize * 4 * 1024);
  draw_progressbar(0, totalProgressBar);

  //Write header
  if (headersize > 0) {
    myFile.write(header, headersize);

    // update progress bar
    processedProgressBar += headersize;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }

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
  compareCRC("nes.txt", 0, renamerom, headersize);
}

void readRom_NES() {
  read_NES("nes", iNES_HEADER, 16, true);
}

void readRaw_NES() {
  read_NES("bin", NULL, 0, false);
}

/******************************************
   Low Level Functions
 *****************************************/
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

  set_address(address);  // PHI2 low, ROMSEL always HIGH
  //  _delay_us(1);
  PHI2_HI;
  //_delay_us(10);
  set_romsel(address);  // ROMSEL is low if need, PHI2 high
  _delay_us(1);         // WRITING
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

#if defined(ENABLE_FLASH)
static void write_chr_byte(unsigned int address, uint8_t data) {
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PORTK = data;
  set_address(address);  // PHI2 low, ROMSEL always HIGH
  _delay_us(1);

  CHR_WRITE_LOW;

  _delay_us(1);  // WRITING

  CHR_WRITE_HI;

  _delay_us(1);

  MODE_READ;
  set_address(0);
  PHI2_HI;

  //_delay_us(1);
}
#endif

void resetROM() {
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
}

void write_mmc1_byte(unsigned int address, uint8_t data) {  // write loop for 5 bit register
  if (address >= 0xE000) {
    for (uint8_t i = 0; i < 5; i++) {
      write_reg_byte(address, data >> i);  // shift 1 bit into temp register [WRITE RAM SAFE]
    }
  } else {
    for (uint8_t j = 0; j < 5; j++) {
      write_prg_byte(address, data >> j);  // shift 1 bit into temp register
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
static void write_reg_byte(unsigned int address, uint8_t data) {  // FIX FOR MMC1 RAM CORRUPTION
  PHI2_LOW;
  ROMSEL_HI;  // A15 HI = E000
  MODE_WRITE;
  PRG_WRITE;  // CPU R/W LO
  PORTK = data;

  set_address(address);  // PHI2 low, ROMSEL always HIGH
  // DIRECT PIN TO PREVENT RAM CORRUPTION
  // DIFFERENCE BETWEEN M2 LO AND ROMSEL HI MUST BE AROUND 33ns
  // IF TIME IS GREATER THAN 33ns THEN WRITES TO 0xE000/0xF000 WILL CORRUPT RAM AT 0x6000/0x7000
  PORTF = 0b01111101;  // ROMSEL LO/M2 HI
  PORTF = 0b01111110;  // ROMSEL HI/M2 LO
  _delay_us(1);
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
}

static void write_ram_byte(unsigned int address, uint8_t data) {  // Mapper 19 (Namco 106/163) WRITE RAM SAFE ($E000-$FFFF)
  PHI2_LOW;
  ROMSEL_HI;
  MODE_WRITE;
  PRG_WRITE;
  PORTK = data;

  set_address(address);  // PHI2 low, ROMSEL always HIGH
  PHI2_HI;
  ROMSEL_LOW;    // SET /ROMSEL LOW OTHERWISE CORRUPTS RAM
  _delay_us(1);  // WRITING
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

static void write_wram_byte(unsigned int address, uint8_t data) {  // Mapper 5 (MMC5) RAM
  PHI2_LOW;
  ROMSEL_HI;
  set_address(address);
  PORTK = data;

  _delay_us(1);
  MODE_WRITE;
  PRG_WRITE;
  PHI2_HI;
  _delay_us(1);  // WRITING
  PHI2_LOW;
  ROMSEL_HI;
  // Back to read mode
  PRG_READ;
  MODE_READ;
  set_address(0);
  // Set phi2 to high state to keep cartridge unreseted
  PHI2_HI;
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

FsFile createNewFile(const char* prefix, const char* extension) {
  char filename[FILENAME_LENGTH];
  snprintf_P(filename, sizeof(filename), _file_name_no_number_fmt, prefix, extension);
  for (uint8_t i = 0; i < 100; i++) {
    if (!sd.exists(filename)) {
      return sd.open(fileName, O_RDWR | O_CREAT);
    }
    snprintf_P(filename, sizeof(filename), _file_name_with_number_fmt, prefix, i, extension);
  }
  // Could not find an available name, recompose the original name and error out.
  snprintf_P(filename, sizeof(filename), _file_name_no_number_fmt, prefix, extension);

  rgbLed(red_color);

  display_Clear();
  print_Msg(filename);
  println_Msg(F(": no available name"));
  display_Update();
  print_FatalError(sd_error_STR);

  rgbLed(black_color);
}

void CreatePRGFileInSD() {
  myFile = createNewFile("PRG", "bin");
}

void CreateCHRFileInSD() {
  myFile = createNewFile("CHR", "bin");
}

//createNewFile fails to dump RAM if ROM isn't dumped first
//void CreateRAMFileInSD() {
//myFile = createNewFile("RAM", "bin");
//}
//Temporary fix
void CreateRAMFileInSD() {
  char fileCount[3];
  strcpy(fileName, "RAM");
  strcat(fileName, ".bin");
  for (uint8_t i = 0; i < 100; i++) {
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
    rgbLed(red_color);

    display_Clear();
    println_Msg(F("RAM FILE FAILED!"));
    display_Update();
    //print_Error(F("SD Error"), true);

    rgbLed(black_color);
  }
}

/******************************************
   Config Functions
 *****************************************/

#if defined(ENABLE_LCD)
void printMapperSelection_NES(int index) {
  display_Clear();
  mapselect = pgm_read_word(mapsize + index);
  print_Msg(FS(FSTRING_MAPPER));
  println_Msg(mapselect);
}
#endif

void setMapper() {
  uint16_t newmapper;
#ifdef ENABLE_GLOBAL_LOG
  // Disable log to prevent unnecessary logging
  println_Log(F("Set Mapper manually"));
  dont_log = true;
#endif

  // OLED
#if defined(ENABLE_OLED)
chooseMapper:
  // Read stored mapper
  EEPROM_readAnything(7, newmapper);
  if (newmapper > 220)
    newmapper = 0;
  // Split into digits
  uint8_t hundreds = newmapper / 100;
  uint8_t tens = newmapper / 10 - hundreds * 10;
  uint8_t units = newmapper - hundreds * 100 - tens * 10;

  // Cycle through all 3 digits
  uint8_t digit = 0;
  while (digit < 3) {
    display_Clear();
    println_Msg(F("Select Mapper:"));
    display.setCursor(23, 20);
    println_Msg(hundreds);
    display.setCursor(43, 20);
    println_Msg(tens);
    display.setCursor(63, 20);
    println_Msg(units);
    println_Msg("");
    println_Msg("");
    println_Msg("");
    print_STR(press_to_change_STR, 1);
    println_Msg(F("Press right to select"));

    if (digit == 0) {
      display.setDrawColor(1);
      display.drawLine(20, 30, 30, 30);
      display.setDrawColor(0);
      display.drawLine(40, 30, 50, 30);
      display.drawLine(60, 30, 70, 30);
      display.setDrawColor(1);
    } else if (digit == 1) {
      display.setDrawColor(0);
      display.drawLine(20, 30, 30, 30);
      display.setDrawColor(1);
      display.drawLine(40, 30, 50, 30);
      display.setDrawColor(0);
      display.drawLine(60, 30, 70, 30);
      display.setDrawColor(1);
    } else if (digit == 2) {
      display.setDrawColor(0);
      display.drawLine(20, 30, 30, 30);
      display.drawLine(40, 30, 50, 30);
      display.setDrawColor(1);
      display.drawLine(60, 30, 70, 30);
    }
    display.updateDisplay();

    while (1) {
      /* Check Button
         1 click
         2 doubleClick
         3 hold
         4 longHold */
      uint8_t b = checkButton();

      if (b == 1) {
        if (digit == 0) {
          if (hundreds < 2)
            hundreds++;
          else
            hundreds = 0;
        } else if (digit == 1) {
          if (hundreds == 2) {
            if (tens < 1)
              tens++;
            else
              tens = 0;
          } else {
            if (tens < 9)
              tens++;
            else
              tens = 0;
          }
        } else if (digit == 2) {
          if (units < 9)
            units++;
          else
            units = 0;
        }
        break;
      } else if (b == 2) {
        if (digit == 0) {
          if (hundreds > 0)
            hundreds--;
          else
            hundreds = 2;
        } else if (digit == 1) {
          if (hundreds == 2) {
            if (tens > 0)
              tens--;
            else
              tens = 1;
          } else {
            if (tens > 0)
              tens--;
            else
              tens = 9;
          }
        } else if (digit == 2) {
          if (units > 0)
            units--;
          else
            units = 9;
        }
        break;
      } else if (b == 3) {
        digit++;
        break;
      }
    }
  }
  display_Clear();

  newmapper = hundreds * 100 + tens * 10 + units;

  // Check if valid
  bool validMapper = 0;
  for (uint8_t currMaplist = 0; currMaplist < mapcount; currMaplist++) {
    if (pgm_read_word(mapsize + currMaplist) == newmapper)
      validMapper = 1;
  }

  if (!validMapper) {
    errorLvl = 1;
    display.println(F("Mapper not supported"));
    display.updateDisplay();
    wait();
    goto chooseMapper;
  }

  // LCD
#elif defined(ENABLE_LCD)
  navigateMenu(0, mapcount - 1, &printMapperSelection_NES);
  newmapper = mapselect;

  display.setCursor(0, 56 + 8);
  print_Msg(F("MAPPER "));
  print_Msg(newmapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(500);

  // Serial Monitor
#elif defined(ENABLE_SERIAL)
setmapper:
  String newmap;
  bool mapfound = false;
  Serial.println(F("SUPPORTED MAPPERS:"));
  for (int i = 0; i < mapcount; i++) {
    mapselect = pgm_read_word(mapsize + i);
    Serial.print("[");
    Serial.print(mapselect);
    Serial.print("]");
    if (i < mapcount - 1) {
      if ((i != 0) && ((i + 1) % 10 == 0))
        Serial.println(FS(FSTRING_EMPTY));
      else
        Serial.print(F("\t"));
    } else
      Serial.println(FS(FSTRING_EMPTY));
  }
  Serial.print(F("Enter Mapper: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  newmapper = newmap.toInt();
  for (uint8_t i = 0; i < mapcount; i++) {
    mapselect = pgm_read_word(mapsize + i);
    if (newmapper == mapselect)
      mapfound = true;
  }
  if (mapfound == false) {
    Serial.println(F("MAPPER NOT SUPPORTED!"));
    Serial.println(FS(FSTRING_EMPTY));
    newmapper = 0;
    goto setmapper;
  }
#endif

  EEPROM_writeAnything(7, newmapper);
  mapper = newmapper;

#ifdef ENABLE_GLOBAL_LOG
  // Enable log again
  dont_log = false;
#endif
}

void checkMapperSize() {
  mapper_NES v;
  for (uint8_t i = 0; i < mapcount; i++) {
    memcpy_P(&v, mapsize + i, sizeof(v));
    if (mapper == v.mapper) {
      prglo = v.prglo;
      prghi = v.prghi;
      chrlo = v.chrlo;
      chrhi = v.chrhi;
      ramlo = v.ramlo;
      ramhi = v.ramhi;
      break;
    }
  }
}

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
void printPrgSize_NES(int index) {
  display_Clear();
  print_Msg(F("PRG Size: "));
  println_Msg(pgm_read_word(&(PRG[index])));
}
#endif

void setPRGSize() {
  uint8_t newprgsize;

#ifdef ENABLE_GLOBAL_LOG
  // Disable log to prevent unnecessary logging
  println_Log(F("Set PRG Size"));
  dont_log = true;
#endif

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
  display_Clear();
  if (prglo == prghi)
    newprgsize = prglo;
  else {
    newprgsize = navigateMenu(prglo, prghi, &printPrgSize_NES);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("PRG SIZE "));
  print_Msg(pgm_read_word(&(PRG[newprgsize])));
  println_Msg(F("K"));
  display_Update();
  delay(500);

#elif defined(ENABLE_SERIAL)
  if (prglo == prghi)
    newprgsize = prglo;
  else {
setprg:
    String sizePRG;
    for (int i = 0; i < (prghi - prglo + 1); i++) {
      Serial.print(F("Select PRG Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(pgm_read_word(&(PRG[i + prglo])));
      Serial.println(F("K"));
    }
    Serial.print(F("Enter PRG Size: "));
    while (Serial.available() == 0) {}
    sizePRG = Serial.readStringUntil('\n');
    Serial.println(sizePRG);
    newprgsize = sizePRG.toInt() + prglo;
    if (newprgsize > prghi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setprg;
    }
  }
  Serial.print(F("PRG Size = "));
  Serial.print(pgm_read_word(&(PRG[newprgsize])));
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(9, newprgsize);
  prgsize = newprgsize;

#ifdef ENABLE_GLOBAL_LOG
  // Enable log again
  dont_log = false;
#endif
}

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
void printChrSize_NES(int index) {
  display_Clear();
  print_Msg(F("CHR Size: "));
  println_Msg(pgm_read_word(&(CHR[index])));
}
#endif

void setCHRSize() {
  uint8_t newchrsize;
#ifdef ENABLE_GLOBAL_LOG
  // Disable log to prevent unnecessary logging
  println_Log(F("Set CHR Size"));
  dont_log = true;
#endif

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
  display_Clear();
  if (chrlo == chrhi)
    newchrsize = chrlo;
  else {
    newchrsize = navigateMenu(chrlo, chrhi, &printChrSize_NES);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("CHR SIZE "));
  print_Msg(pgm_read_word(&(CHR[newchrsize])));
  println_Msg(F("K"));
  display_Update();
  delay(500);

#elif defined(ENABLE_SERIAL)
  if (chrlo == chrhi)
    newchrsize = chrlo;
  else {
setchr:
    String sizeCHR;
    for (int i = 0; i < (chrhi - chrlo + 1); i++) {
      Serial.print(F("Select CHR Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(pgm_read_word(&(CHR[i + chrlo])));
      Serial.println(F("K"));
    }
    Serial.print(F("Enter CHR Size: "));
    while (Serial.available() == 0) {}
    sizeCHR = Serial.readStringUntil('\n');
    Serial.println(sizeCHR);
    newchrsize = sizeCHR.toInt() + chrlo;
    if (newchrsize > chrhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setchr;
    }
  }
  Serial.print(F("CHR Size = "));
  Serial.print(pgm_read_word(&(CHR[newchrsize])));
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(10, newchrsize);
  chrsize = newchrsize;

#ifdef ENABLE_GLOBAL_LOG
  // Enable log again
  dont_log = false;
#endif
}

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
void printRamSize_NES(int index) {
  display_Clear();
  print_Msg(F("RAM Size: "));
  if (mapper == 0)
    println_Msg(pgm_read_byte(&(RAM[index])) / 4);
  else if (mapper == 16)
    println_Msg(pgm_read_byte(&(RAM[index])) * 32);
  else if (mapper == 19) {
    if (index == 2)
      println_Msg(F("128"));
    else
      println_Msg(pgm_read_byte(&(RAM[index])));
  } else if ((mapper == 159) || (mapper == 80))
    println_Msg(pgm_read_byte(&(RAM[index])) * 16);
  else if (mapper == 82)
    println_Msg(index * 5);
  else
    println_Msg(pgm_read_byte(&(RAM[index])));
}
#endif

void setRAMSize() {
  uint8_t newramsize;
#ifdef ENABLE_GLOBAL_LOG
  // Disable log to prevent unnecessary logging
  println_Log(F("Set RAM Size"));
  dont_log = true;
#endif

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
  display_Clear();
  if (ramlo == ramhi)
    newramsize = ramlo;
  else {
    newramsize = navigateMenu(0, ramhi, &printRamSize_NES);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  if ((mapper == 16) || (mapper == 159)) {
    int sizeEEP = 0;
    print_Msg(F("EEPROM SIZE "));
    if (mapper == 16)
      sizeEEP = pgm_read_byte(&(RAM[newramsize])) * 32;
    else
      sizeEEP = pgm_read_byte(&(RAM[newramsize])) * 16;
    print_Msg(sizeEEP);
    println_Msg(F("B"));
  } else if (mapper == 19) {
    print_Msg(F("RAM SIZE "));
    if (newramsize == 2)
      println_Msg(F("128B"));
    else {
      print_Msg(pgm_read_byte(&(RAM[newramsize])));
      println_Msg(F("K"));
    }
  } else if (mapper == 80) {
    print_Msg(F("RAM SIZE "));
    print_Msg(pgm_read_byte(&(RAM[newramsize])) * 16);
    println_Msg(F("B"));
  } else {
    print_Msg(F("RAM SIZE "));
    if (mapper == 0)
      print_Msg(newramsize * 2);
    else if (mapper == 82)
      print_Msg(newramsize * 5);
    else
      print_Msg(pgm_read_byte(&(RAM[newramsize])));
    println_Msg(F("K"));
  }
  display_Update();
  delay(500);

#elif defined(ENABLE_SERIAL)
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
        Serial.print(pgm_read_byte(&(RAM[i])) / 4);
        Serial.println(F("K"));
      } else if ((mapper == 16) || (mapper == 159)) {
        if (mapper == 16)
          Serial.print(pgm_read_byte(&(RAM[i + ramlo])) * 32);
        else
          Serial.print(pgm_read_byte(&(RAM[i + ramlo])) * 16);
        Serial.println(F("B"));
      } else if (mapper == 19) {
        if (i == 2)
          Serial.println(F("128B"));
        else {
          Serial.print(pgm_read_byte(&(RAM[i + ramlo])));
          Serial.println(F("K"));
        }
      } else {
        Serial.print(pgm_read_byte(&(RAM[i + ramlo])));
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
      Serial.println(FS(FSTRING_EMPTY));
      goto setram;
    }
  }
  if ((mapper == 16) || (mapper == 159)) {
    int sizeEEP = 0;
    Serial.print(F("EEPROM Size = "));
    if (mapper == 16)
      sizeEEP = pgm_read_byte(&(RAM[newramsize])) * 32;
    else
      sizeEEP = pgm_read_byte(&(RAM[newramsize])) * 16;
    Serial.print(sizeEEP);
    Serial.println(F("B"));
    Serial.println(FS(FSTRING_EMPTY));
  } else if (mapper == 19) {
    Serial.print(F("RAM Size =  "));
    if (newramsize == 2)
      Serial.println(F("128B"));
    else {
      Serial.print(pgm_read_byte(&(RAM[newramsize])));
      Serial.println(F("K"));
    }
    Serial.println(FS(FSTRING_EMPTY));
  } else if (mapper == 80) {
    Serial.print(F("RAM Size = "));
    Serial.print(pgm_read_byte(&(RAM[newramsize])) * 16);
    Serial.println(F("B"));
    Serial.println(FS(FSTRING_EMPTY));
  } else {
    Serial.print(F("RAM Size = "));
    if (mapper == 0)
      Serial.print(newramsize * 2);
    else if (mapper == 82)
      Serial.print(newramsize * 5);
    else
      Serial.print(pgm_read_byte(&(RAM[newramsize])));
    Serial.println(F("K"));
    Serial.println(FS(FSTRING_EMPTY));
  }
#endif
  EEPROM_writeAnything(11, newramsize);
  ramsize = newramsize;

#ifdef ENABLE_GLOBAL_LOG
  // Enable log again
  dont_log = false;
#endif
}

// MMC6 Detection
// Mapper 4 includes both MMC3 AND MMC6
// RAM is mapped differently between MMC3 and MMC6
void checkMMC6() {               // Detect MMC6 Carts - read PRG 0x3E00A ("STARTROPICS")
  write_prg_byte(0x8000, 6);     // PRG Bank 0 ($8000-$9FFF)
  write_prg_byte(0x8001, 0x1F);  // 0x3E000
  uint8_t prgchk0 = read_prg_byte(0x800A);
  uint8_t prgchk1 = read_prg_byte(0x800B);
  uint8_t prgchk2 = read_prg_byte(0x800C);
  uint8_t prgchk3 = read_prg_byte(0x800D);
  if ((prgchk0 == 0x53) && (prgchk1 == 0x54) && (prgchk2 == 0x41) && (prgchk3 == 0x52))
    mmc6 = true;  // MMC6 Cart
}

void checkStatus_NES() {
  EEPROM_readAnything(7, mapper);
  EEPROM_readAnything(9, prgsize);
  EEPROM_readAnything(10, chrsize);
  EEPROM_readAnything(11, ramsize);
  prg = (int_pow(2, prgsize)) * 16;
  if (chrsize == 0)
    chr = 0;  // 0K
  else
    chr = (int_pow(2, chrsize)) * 4;
  if (ramsize == 0)
    ram = 0;  // 0K
  else if (mapper == 82)
    ram = 5;  // 5K
  else
    ram = (int_pow(2, ramsize)) * 4;

  // Mapper Variants
  // Identify variant for use across multiple functions
  if (mapper == 4) {  // Check for MMC6/MMC3
    checkMMC6();
    if (mmc6)
      ram = 1;                // 1K
  } else if (mapper == 30) {  // Check for Flashable/Non-Flashable
#if defined(ENABLE_FLASH)
    NESmaker_ID();  // Flash ID
#endif
  }

  display_Clear();
  println_Msg(F("NES CART READER"));
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  printNESSettings();
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

static void printNESSettings(void) {
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
  } else if ((mapper == 16) || (mapper == 80) || (mapper == 159)) {
    if (mapper == 16)
      print_Msg(ram * 32);
    else
      print_Msg(ram * 16);
    println_Msg(F("B"));
  } else if (mapper == 19) {
    if (ramsize == 2)
      println_Msg(F("128B"));
    else {
      print_Msg(ram);
      println_Msg(F("K"));
    }
  } else {
    print_Msg(ram);
    println_Msg(F("K"));
  }
}

/******************************************
   ROM Functions
 *****************************************/
void dumpPRG(word base, word address) {
  for (size_t x = 0; x < 512; x++) {
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  myFile.write(sdBuffer, 512);
}

void dumpCHR(word address) {
  for (size_t x = 0; x < 512; x++) {
    sdBuffer[x] = read_chr_byte(address + x);
  }
  myFile.write(sdBuffer, 512);
}

void dumpCHR_M2(word address) {  // MAPPER 45 - PULSE M2 LO/HI
  for (size_t x = 0; x < 512; x++) {
    PHI2_LOW;
    sdBuffer[x] = read_chr_byte(address + x);
  }
  myFile.write(sdBuffer, 512);
}

void dumpMMC5RAM(word base, word address) {  // MMC5 SRAM DUMP - PULSE M2 LO/HI
  for (size_t x = 0; x < 512; x++) {
    PHI2_LOW;
    sdBuffer[x] = read_prg_byte(base + address + x);
  }
  myFile.write(sdBuffer, 512);
}

void writeMMC5RAM(word base, word address) {  // MMC5 SRAM WRITE
  uint8_t bytecheck;
  myFile.read(sdBuffer, 512);
  for (size_t x = 0; x < 512; x++) {
    do {
      write_prg_byte(0x5102, 2);  // PRG RAM PROTECT1
      write_prg_byte(0x5103, 1);  // PRG RAM PROTECT2
      write_wram_byte(base + address + x, sdBuffer[x]);
      bytecheck = read_prg_byte(base + address + x);
    } while (bytecheck != sdBuffer[x]);  // CHECK WRITTEN BYTE
  }
  write_prg_byte(0x5102, 0);  // PRG RAM PROTECT1
  write_prg_byte(0x5103, 0);  // PRG RAM PROTECT2
}

void dumpBankPRG(const size_t from, const size_t to, const size_t base) {
  for (size_t address = from; address < to; address += 512) {
    dumpPRG(base, address);
  }
}

void dumpBankCHR(const size_t from, const size_t to) {
  for (size_t address = from; address < to; address += 512) {
    dumpCHR(address);
  }
}

void readPRG(bool readrom) {
  if (!readrom) {
    display_Clear();
    display_Update();

    rgbLed(blue_color);
    set_address(0);
    _delay_us(1);
    CreatePRGFileInSD();
  } else {
    set_address(0);
    _delay_us(1);
  }

  word base = 0x8000;
  bool busConflict = false;
  uint16_t banks;

  if (myFile) {
    switch (mapper) {
      case 0:
      case 3:
      case 13:
      case 87:                                                      // 16K/32K
      case 184:                                                     // 32K
      case 185:                                                     // 16K/32K
        dumpBankPRG(0, (((word)prgsize) * 0x4000) + 0x4000, base);  // 16K or 32K
        break;

      case 1:
      case 155:  // 32K/64K/128K/256K/512K
        banks = int_pow(2, prgsize) - 1;
        for (size_t i = 0; i < banks; i++) {  // 16K Banks ($8000-$BFFF)
          write_prg_byte(0x8000, 0x80);       // Clear Register
          write_mmc1_byte(0x8000, 0x0C);      // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
          if (prgsize > 4)                    // 512K
            write_mmc1_byte(0xA000, 0x00);    // Reset 512K Flag for Lower 256K
          if (i > 15)                         // Switch Upper 256K
            write_mmc1_byte(0xA000, 0x10);    // Set 512K Flag
          write_mmc1_byte(0xE000, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);  // Final Bank ($C000-$FFFF)
        break;

      case 2:   // bus conflicts - fixed last bank
      case 30:  // bus conflicts in non-flashable configuration
        banks = int_pow(2, prgsize);
        busConflict = true;
        for (size_t i = 0; i < banks - 1; i++) {
          for (size_t x = 0; x < 0x4000; x++) {
            if (read_prg_byte(0xC000 + x) == i) {
              write_prg_byte(0xC000 + x, i);
              busConflict = false;
              break;
            }
          }
          if (busConflict) {
            write_prg_byte(0xC000 + i, i);
          }
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);
        break;

      case 4:
      case 47:
      case 64:
      case 118:
      case 119:
      case 158:
        banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
        if (mapper == 47)
          write_prg_byte(0xA001, 0x80);          // Block Register - PRG RAM Chip Enable, Writable
        for (size_t i = 0; i < banks; i += 2) {  // 32K/64K/128K/256K/512K
          if (mapper == 47) {
            if (i == 0)
              write_prg_byte(0x6000, 0);  // Switch to Lower Block
            else if (i == 16)
              write_prg_byte(0x6000, 1);  // Switch to Upper Block
          }
          write_prg_byte(0x8000, 6);  // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7);  // PRG Bank 1 ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        if ((mapper == 64) || (mapper == 158)) {
          write_prg_byte(0x8000, 15);  // PRG Bank 2 ($C000-$DFFF)
          write_prg_byte(0x8001, banks);
        }
        dumpBankPRG(0x4000, 0x8000, base);  // Final 2 Banks ($C000-$FFFF)
        break;

      case 5:  // 128K/256K/512K
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x5100, 3);               // 8K PRG Banks
        for (size_t i = 0; i < banks; i += 2) {  // 128K/256K/512K
          write_prg_byte(0x5114, i | 0x80);
          write_prg_byte(0x5115, (i + 1) | 0x80);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 7:  // 128K/256K
      case 77:
      case 96:   // 128K
      case 177:  // up to 1024K
      case 241:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {  // 32K Banks
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x8000, base);  // 32K Banks ($8000-$FFFF)
        }
        break;

      case 9:                              // 128K
        for (size_t i = 0; i < 13; i++) {  // 16-3 = 13 = 128K
          write_prg_byte(0xA000, i);       // $8000-$9FFF
          dumpBankPRG(0x0, 0x2000, base);  // Switch Bank ($8000-$9FFF)
        }
        dumpBankPRG(0x2000, 0x8000, base);  // Final 3 Banks ($A000-$FFFF)
        break;

      case 10:  // 128K/256K
        for (size_t i = 0; i < (unsigned)(((prgsize - 3) * 8) + 7); i++) {
          write_prg_byte(0xA000, i);       // $8000-$BFFF
          dumpBankPRG(0x0, 0x4000, base);  // Switch Bank ($8000-$BFFF)
        }
        dumpBankPRG(0x4000, 0x8000, base);  // Final Bank ($C000-$FFFF)
        break;

      case 11:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xFFB0 + i, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 15:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 16:
      case 159:  // 128K/256K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x6008, i);       // Submapper 4
          write_prg_byte(0x8008, i);       // Submapper 5
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 18:  // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i & 0xF);
          write_prg_byte(0x8001, (i >> 4) & 0xF);
          write_prg_byte(0x8002, (i + 1) & 0xF);
          write_prg_byte(0x8003, ((i + 1) >> 4) & 0xF);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 19:                             // 128K/256K
        for (size_t j = 0; j < 64; j++) {  // Init Register
          write_ram_byte(0xE000, 0);       // PRG Bank 0 ($8000-$9FFF)
        }
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i++) {
          write_ram_byte(0xE000, i);  // PRG Bank 0 ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 21:  // 256K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xA000, i);
          dumpBankPRG(0x2000, 0x4000, base);
        }
        break;

      case 22:
      case 25:
      case 65:
      case 75:  // 128K/256K
        banks = int_pow(2, prgsize) * 2;

        // set vrc4 swap setting for TMNT2
        if (mapper == 25)
          write_prg_byte(0x9005, 0x00);

        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i);
          write_prg_byte(0xA000, i + 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 23:
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x9002, 0);
        write_prg_byte(0x9008, 0);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 24:
      case 26:  // 256K
      case 78:  // 128K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {  // 128K
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x2000, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 28:  // using 32k mode for inner and outer banks, switching only with outer
        banks = int_pow(2, prgsize) / 2;
        write_prg_byte(0x5000, 0x81);
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x5000, 0x80);
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x5000, 0x01);
        write_prg_byte(0x8000, 0);
        write_prg_byte(0x5000, 0x00);
        write_prg_byte(0x8000, 0);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5000, 0x81);
          write_prg_byte(0x8000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 31:
        banks = int_pow(2, prgsize) * 4;
        for (size_t i = 0; i < banks; i += 8) {
          write_prg_byte(0x5FF8, i);
          write_prg_byte(0x5FF9, i + 1);
          write_prg_byte(0x5FFA, i + 2);
          write_prg_byte(0x5FFB, i + 3);
          write_prg_byte(0x5FFC, i + 4);
          write_prg_byte(0x5FFD, i + 5);
          write_prg_byte(0x5FFE, i + 6);
          write_prg_byte(0x5FFF, i + 7);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 32:  // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i++) {  // 128K/256K
          write_prg_byte(0x9000, 1);          // PRG Mode 0 - Read $A000-$BFFF to avoid difference between Modes 0 and 1
          write_prg_byte(0xA000, i);          // PRG Bank
          dumpBankPRG(0x2000, 0x4000, base);  // 8K Banks ($A000-$BFFF)
        }
        break;

      case 33:
      case 48:  // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000, i);       // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i + 1);   // PRG Bank 1 ($A000-$BFFF)
          dumpBankPRG(0x0, 0x4000, base);  // 8K Banks ($A000-$BFFF)
        }
        break;

      case 34:  // BxROM/NINA
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x7FFD, i);       // NINA Bank select
          write_prg_byte(0x8000, i);       // BxROM bank select
          delay(200);                      // NINA seems slow to switch banks
          dumpBankPRG(0x0, 0x8000, base);  // 32K Banks ($8000-$FFFF)
        }
        break;

      case 35:
      case 90:
      case 209:
      case 211:
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0xD000, 0x02);

        for (uint8_t i = 0; i < banks; i++) {
          write_prg_byte(0xD003, (((i >> 5) & 0x06) | 0x20));
          write_prg_byte(0x8000, (i & 0x3f));
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 36:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xFFA0 + i, (i << 4));
          write_prg_byte(0x4101, 0);
          write_prg_byte(0x4102, (i << 4));
          write_prg_byte(0x4103, 0);
          write_prg_byte(0x4100, 0);
          write_prg_byte(0x4103, 0xFF);
          write_prg_byte(0xFFFF, 0xFF);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 37:
        banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
        write_prg_byte(0xA001, 0x80);             // Block Register - PRG RAM Chip Enable, Writable
        for (size_t i = 0; i < banks; i += 2) {   // 256K
          if (i == 0)
            write_prg_byte(0x6000, 0);  // Switch to Lower Block ($0000-$FFFF)
          else if (i == 8)
            write_prg_byte(0x6000, 3);  // Switch to 2nd 64K Block ($10000-$1FFFF)
          else if (i == 16)
            write_prg_byte(0x6000, 4);  // Switch to 128K Block ($20000-$3FFFF)
          write_prg_byte(0x8000, 6);    // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7);  // PRG Bank 1 ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);  // Final 2 Banks ($C000-$FFFF)
        break;

      case 38:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x7000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 42:
        banks = int_pow(2, prgsize) * 2;
        base = 0x6000;  // 8k switchable PRG ROM bank at $6000-$7FFF
        for (size_t i = 0; i < banks - 4; i++) {
          write_prg_byte(0xE000, i & 0x0F);
          dumpBankPRG(0x0, 0x2000, base);
        }
        base = 0x8000;  // last 32k fixed to $8000-$FFFF
        dumpBankPRG(0x0, 0x8000, base);
        break;

      case 45:                                    // MMC3 Clone with Outer Registers
        banks = ((int_pow(2, prgsize) * 2)) - 2;  // Set Number of Banks
        for (size_t i = 0; i < banks; i += 2) {   // 128K/256K/512K/1024K
          // set outer bank registers
          write_prg_byte(0x6000, 0x00);               // CHR-OR
          write_prg_byte(0x6000, (i & 0xC0));         // PRG-OR
          write_prg_byte(0x6000, ((i >> 2) & 0xC0));  // CHR-AND,CHR-OR/PRG-OR
          write_prg_byte(0x6000, 0x80);               // PRG-AND
          // set inner bank registers
          write_prg_byte(0x8000, 6);  // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x8001, i);
          dumpBankPRG(0x0, 0x2000, base);
          // set outer bank registers
          write_prg_byte(0x6000, 0x00);                     // CHR-OR
          write_prg_byte(0x6000, ((i + 1) & 0xC0));         // PRG-OR
          write_prg_byte(0x6000, (((i + 1) >> 2) & 0xC0));  // CHR-AND,CHR-OR/PRG-OR
          write_prg_byte(0x6000, 0x80);                     // PRG-AND
          // set inner bank registers
          write_prg_byte(0x8000, 7);  // PRG Bank 1 ($A000-$BFFF)
          write_prg_byte(0x8001, i + 1);
          dumpBankPRG(0x2000, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);  // Final 2 Banks ($C000-$FFFF)
        break;

      case 46:
        banks = int_pow(2, prgsize) / 2;  // 32k banks
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x6000, (i & 0x1E) >> 1);  // high bits
          write_prg_byte(0x8000, i & 0x01);         // low bit
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 52:
        banks = int_pow(2, prgsize);
        write_prg_byte(0xA001, 0x80);  // enable WRAM write
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x6000, (i & 0x07) | 0x08);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 56:
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xE000, 1);
          write_prg_byte(0xF000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 57:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8800, (i & 0x07) << 5);
          write_prg_byte(0x8000, 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 58:
      case 213:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + (i & 0x07), 0x00);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 59:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte((0x8000 + (i & 0x07)) << 4 | 0x80, 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 60:
        dumpBankPRG(0x0, 0x4000, base);
        for (size_t i = 0; i < 3; i++) {
          write_prg_byte(0x8D8D, i);
          delay(500);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 62:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + (i * 512) + ((i & 32) << 1), 0x00);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 66:  // 64K/128K
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {  // 64K/128K
          write_prg_byte(0x8000, i << 4);     // bits 4-5
          dumpBankPRG(0x0, 0x8000, base);     // 32K Banks ($8000-$FFFF)
        }
        break;

      case 63:  // 3072K total
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < 192; i++) {
          write_prg_byte(0x8000 + (i << 2), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 67:  // 128K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {  // 128K
          write_reg_byte(0xF800, i);          // [WRITE RAM SAFE]
          dumpBankPRG(0x0, 0x4000, base);     // 16K Banks ($8000-$BFFF)
        }
        break;

      case 68:
      case 73:  // 128K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {  // 128K
          write_prg_byte(0xF000, i);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 69:  // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x8000, 8);            // Command Register - PRG Bank 0
        write_prg_byte(0xA000, 0);            // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
        for (size_t i = 0; i < banks; i++) {  // 128K/256K
          write_prg_byte(0x8000, 9);          // Command Register - PRG Bank 1
          write_prg_byte(0xA000, i);          // Parameter Register - ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);     // 8K Banks ($8000-$9FFF)
        }
        break;

      case 70:
      case 89:
      case 152:  // 64K/128K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {  // 128K
          write_prg_byte(0x8000, i << 4);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 71:  // 64K/128K/256K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xC000, i);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 72:  // 128K
        banks = int_pow(2, prgsize);
        write_prg_byte(0x8000, 0);            // Reset Register
        for (size_t i = 0; i < banks; i++) {  // 128K
          write_prg_byte(0x8000, i | 0x80);   // PRG Command + Bank
          write_prg_byte(0x8000, i);          // PRG Bank
          dumpBankPRG(0x0, 0x4000, base);     // 16K Banks ($8000-$BFFF)
        }
        break;

      case 76:
      case 88:
      case 95:
      case 154:  // 128K
      case 206:  // 32/64/128K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks - 2; i += 2) {
          write_prg_byte(0x8000, 6);
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7);
          write_prg_byte(0x8001, i | 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);
        break;

      case 79:
      case 146:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x4100, i << 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 80:   // 128K
      case 207:  // 256K [CART SOMETIMES NEEDS POWERCYCLE]
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x7EFA, i);      // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x7EFC, i + 1);  // PRG Bank 1 ($A000-$BFFF)
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 82:  // 128K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x7EFA, i << 2);        // PRG Bank 0 ($8000-$9FFF)
          write_prg_byte(0x7EFB, (i + 1) << 2);  // PRG Bank 1 ($A000-$BFFF)
          dumpBankPRG(0x0, 0x4000, base);        // 8K Banks ($8000-$BFFF)
        }
        break;

      case 85:  // 128K/512K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, i);       // PRG Bank 0 ($8000-$9FFF)
          dumpBankPRG(0x0, 0x2000, base);  // 8K Banks ($8000-$9FFF)
        }
        break;

      case 86:
      case 140:  // 128K
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {  // 128K
          write_prg_byte(0x6000, i << 4);     // bits 4-5
          dumpBankPRG(0x0, 0x8000, base);     // 32K Banks ($8000-$FFFF)
        }
        break;

      case 91:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < (banks - 2); i += 2) {
          write_prg_byte(0x7000, (i | 0));
          write_prg_byte(0x7001, (i | 1));
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);
        break;

      case 92:  // 256K
        banks = int_pow(2, prgsize);
        write_prg_byte(0x8000, 0);            // Reset Register
        for (size_t i = 0; i < banks; i++) {  // 256K
          write_prg_byte(0x8000, i | 0x80);   // PRG Command + Bank
          write_prg_byte(0x8000, i);          // PRG Bank
          dumpBankPRG(0x4000, 0x8000, base);  // 16K Banks ($C000-$FFFF)
        }
        break;

      case 93:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x6000, i);
          write_prg_byte(0x8000, i << 4 | 0x01);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 94:  // bus conflicts - fixed last bank
        banks = int_pow(2, prgsize);
        busConflict = true;
        for (size_t i = 0; i < banks - 1; i++) {
          for (size_t x = 0; x < 0x4000; x++) {
            if (read_prg_byte(0xC000 + x) == (i << 2)) {
              write_prg_byte(0xC000 + x, i << 2);
              busConflict = false;
              break;
            }
          }
          if (busConflict) {
            write_prg_byte(0x8000 + i, i << 2);
          }
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);
        break;

      case 97:   // fixed first bank
      case 180:  // bus conflicts - fixed fist bank
        banks = int_pow(2, prgsize);
        busConflict = true;
        dumpBankPRG(0x0, 0x4000, base);
        for (size_t i = 1; i < banks; i++) {
          for (size_t x = 0; x < 0x4000; x++) {
            if (read_prg_byte(0x8000 + x) == i) {
              write_prg_byte(0x8000 + x, i);
              busConflict = false;
              break;
            }
          }
          if (busConflict) {
            write_prg_byte(0x8000 + i, i);
          }
          dumpBankPRG(0x4000, 0x8000, base);
        }
        break;

      case 105:                           // 256K
        write_mmc1_byte(0xA000, 0x00);    // Clear PRG Init/IRQ (Bit 4)
        write_mmc1_byte(0xA000, 0x10);    // Set PRG Init/IRQ (Bit 4) to enable bank swapping
        for (size_t i = 0; i < 4; i++) {  // PRG CHIP 1 128K
          write_mmc1_byte(0xA000, i << 1);
          dumpBankPRG(0x0, 0x8000, base);  // 32K Banks ($8000-$FFFF)
        }
        write_mmc1_byte(0x8000, 0x0C);    // Switch 16K Bank ($8000-$BFFF) + Fixed Last Bank ($C000-$FFFF)
        write_mmc1_byte(0xA000, 0x08);    // Select PRG CHIP 2 (Bit 3)
        for (size_t j = 0; j < 8; j++) {  // PRG CHIP 2 128K
          write_mmc1_byte(0xE000, j);
          dumpBankPRG(0x0, 0x4000, base);  // 16K Banks ($8000-$BFFF)
        }
        break;

      case 111:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5000, i);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 113:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x4100, (i & 0x07) << 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 114:  // Submapper 0
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x6000, 0);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0xA000, 4);
          write_prg_byte(0xC000, i);
          dumpBankPRG(0x0, 0x2000, base);
        }
        break;

      case 126:
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0xA001, 0x80);  // enable WRAM
        write_prg_byte(0x6003, 0x00);  // set MMC3 banking mode
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x6000, (i & 0x180) >> 3 | (i & 0x70) >> 4);  // select outer bank
          write_prg_byte(0x8000, 6);                                   // 8k bank 0 at $8000
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7);  // 8k bank 1 at $A000
          write_prg_byte(0x8001, i + 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 134:
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x6000, 0x00);  // set MMC3 banking mode
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x6001, (i & 0x30) >> 4);  // select outer bank
          write_prg_byte(0x8000, 6);                // 8k bank 0 at $8000
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7);  // 8k bank 1 at $A000
          write_prg_byte(0x8001, i + 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 142:
        banks = int_pow(2, prgsize) * 2;
        base = 0x6000;  // 4x 8k switchable PRG ROM banks at $6000-$DFFF
        for (size_t i = 0; i < banks; i += 4) {
          write_prg_byte(0xE000, 4);  // Select 8 KB PRG bank at CPU $6000-$7FFF
          write_prg_byte(0xF000, i);
          write_prg_byte(0xE000, 1);  // Select 8 KB PRG bank at CPU $8000-$9FFF
          write_prg_byte(0xF000, i + 1);
          write_prg_byte(0xE000, 2);  // Select 8 KB PRG bank at CPU $A000-$BFFF
          write_prg_byte(0xF000, i + 2);
          write_prg_byte(0xE000, 3);  // Select 8 KB PRG bank at CPU $C000-$DFFF
          write_prg_byte(0xF000, i + 3);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 148:  // Sachen SA-008-A and Tengen 800008 -- Bus conflicts
        banks = int_pow(2, prgsize) / 2;
        busConflict = true;
        for (size_t i = 0; i < banks; i++) {
          for (size_t x = 0; x < 0x8000; x++) {
            if (read_prg_byte(0x8000 + x) == i) {
              write_prg_byte(0x8000 + x, i << 3);
              busConflict = false;
              break;
            }
          }
          if (busConflict) {
            write_prg_byte(0x8000 + i, i << 3);
          }
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 153:  // 512K
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {  // 512K
          write_prg_byte(0x8000, i >> 4);     // PRG Outer Bank (Documentation says duplicate over $8000-$8003 registers)
          write_prg_byte(0x8001, i >> 4);     // PRG Outer Bank
          write_prg_byte(0x8002, i >> 4);     // PRG Outer Bank
          write_prg_byte(0x8003, i >> 4);     // PRG Outer Bank
          write_prg_byte(0x8008, i & 0xF);    // PRG Inner Bank
          dumpBankPRG(0x0, 0x4000, base);     // 16K Banks ($8000-$BFFF)
        }
        break;

      case 157:
        for (size_t i = 0; i < 15; i++) {
          write_prg_byte(0x8008, i);  // select 16k bank at $8000-$BFFF
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);  // last 16k bank fixed at $C000-$FFFF
        break;

      case 162:
        banks = int_pow(2, prgsize) / 2;
        write_prg_byte(0x5300, 0x07);  // A16-A15 controlled by $5000
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5200, (i & 0x30) >> 4);  // A20-A19
          write_prg_byte(0x5000, i & 0x0F);         // A18-A15
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 163:
        banks = int_pow(2, prgsize) / 2;
        write_prg_byte(0x5300, 0x04);  // disable bit swap on writes to $5000-$5200
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5200, (i & 0x30) >> 4);  // A20-A19
          write_prg_byte(0x5000, i & 0x0F);         // A18-A15
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 174:  // 128k
        for (size_t i = 0; i < 8; i++) {
          write_prg_byte(0xFF00 + (i << 4), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 176:
        banks = int_pow(2, prgsize) * 2;
        write_prg_byte(0x5FF3, 0);  // extended MMC3 mode: disabled
        write_prg_byte(0x5FF0, 1);  // 256K outer bank mode
        for (size_t i = 0; i < banks - 3; i += 2) {
          write_prg_byte(0x5FF1, (i & 0xE0) >> 1);  // outer bank select
          write_prg_byte(0x8000, 6);
          write_prg_byte(0x8001, i);
          write_prg_byte(0x8000, 7);
          write_prg_byte(0x8001, i + 1);
          dumpBankPRG(0x0, 0x4000, base);
        }
        dumpBankPRG(0x4000, 0x8000, base);
        break;

      case 178:
        banks = int_pow(2, prgsize);
        write_prg_byte(0x4800, 0);  // NROM-256 mode
        write_prg_byte(0x4803, 0);  // set PRG-RAM
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x4802, i >> 3);    // high PRG (up to 8 bits?!)
          write_prg_byte(0x4801, i & 0x07);  // low PRG (3 bits)
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 200:
      case 212:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + (i & 0x07), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 201:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 + (i & 0xFF), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 202:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 | (i << 1), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 203:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000, (i & 0x1F) << 2);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 210:  // 128K/256K
        banks = int_pow(2, prgsize) * 2;
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0xE000, i);      // PRG Bank 0 ($8000-$9FFF) [WRITE NO RAM]
          write_prg_byte(0xE800, i + 1);  // PRG Bank 1 ($A000-$BFFF) [WRITE NO RAM]
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 214:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 | (i << 2), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 225:
      case 255:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x8000 + (((i & 0x40) << 8) | ((i & 0x3F) << 6)), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 226:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i += 2) {
          write_prg_byte(0x8001, (i & 0x40) >> 6);
          write_prg_byte(0x8000, ((i & 0x20) << 2) | (i & 0x1F));
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 227:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8083 + ((i & 0xF) << 3), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 228:
        banks = int_pow(2, prgsize);
        write_prg_byte(0x8000, 0);
        for (size_t i = 0; i < banks; i += 2) {  // up to 1024k PRG
          write_prg_byte(0x8000 + ((i & 0x3F) << 6), 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        if (prgsize > 5) {  // reading the 3rd 512k PRG chip (Action 52)
          for (size_t i = 0; i < 32; i += 2) {
            write_prg_byte(0x9800 + ((i & 0x1F) << 6), 0);
            dumpBankPRG(0x0, 0x8000, base);
          }
        }
        break;

      case 229:
        write_prg_byte(0x8000, 0);
        dumpBankPRG(0x0, 0x8000, base);
        for (size_t i = 2; i < 32; i++) {
          write_prg_byte(0x8000 + i, i);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 232:
        banks = int_pow(2, prgsize) / 4;
        for (size_t outerbank = 0; outerbank < 4; outerbank++) {
          write_prg_byte(0x8000, outerbank << 3);
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xC000, i);
            dumpBankPRG(0x0, 0x4000, base);
          }
        }
        break;

      case 235:
        for (size_t i = 0; i < 32; i++) {
          write_prg_byte(0x8000 + i, 0);
          dumpBankPRG(0x0, 0x8000, base);
        }
        if (prgsize > 6) {
          for (size_t i = 32; i < 64; i++) {
            write_prg_byte(0x80E0 + i, 0);
            dumpBankPRG(0x0, 0x8000, base);
          }
          if (prgsize > 7) {
            for (size_t i = 64; i < 96; i++) {
              write_prg_byte(0x81E0 + i, 0);
              dumpBankPRG(0x0, 0x8000, base);
            }
            for (size_t i = 96; i < 128; i++) {
              write_prg_byte(0x82E0 + i, 0);
              dumpBankPRG(0x0, 0x8000, base);
            }
          }
        }
        break;

      case 236:
        banks = int_pow(2, prgsize);
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x8000 | ((i & 0x38) >> 3), 0);  // A19-A17
          write_prg_byte(0xC030 | (i & 0x0F), 0);         // A17-A14
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 240:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i++) {
          write_prg_byte(0x5FFF, (i & 0xF) << 4);
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 242:                            // total size is 640k THIS IS NORMAL
        for (size_t i = 0; i < 32; i++) {  // dump 1st chip of 512k
          write_prg_byte(0x8400 + (i * 4), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        for (size_t i = 0; i < 8; i++) {  // dump 2nd chip of 128k
          write_prg_byte(0x8000 + (i * 4), 0);
          dumpBankPRG(0x0, 0x4000, base);
        }
        break;

      case 246:
        banks = int_pow(2, prgsize) / 2;
        for (size_t i = 0; i < banks; i += 4) {
          write_prg_byte(0x6000, (i | 0));
          write_prg_byte(0x6001, (i | 1));
          write_prg_byte(0x6002, (i | 2));
          write_prg_byte(0x6003, (i | 3));
          dumpBankPRG(0x0, 0x8000, base);
        }
        break;

      case 446:
        {
          banks = int_pow(2, prgsize) * 2;
          write_prg_byte(0x5003, 0);
          write_prg_byte(0x5005, 0);
          for (uint8_t i = 0; i < banks; i++) {  // 8192 for 64MiB
            write_prg_byte(0x5002, i >> 8);      // outer bank LSB
            write_prg_byte(0x5001, i);           // outer bank MSB
            write_prg_byte(0x8000, 0);
            dumpBankPRG(0x0, 0x2000, base);
          }
          break;
        }
    }
    if (!readrom) {
      myFile.flush();
      myFile.close();

      println_Msg(F("PRG FILE DUMPED!"));
      println_Msg(FS(FSTRING_EMPTY));
      display_Update();
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  rgbLed(black_color);
}

void readCHR(bool readrom) {
  if (!readrom) {
    display_Clear();
    display_Update();
  }

  uint16_t banks;

  rgbLed(green_color);
  set_address(0);
  _delay_us(1);
  if (chrsize == 0) {
    println_Msg(F("CHR SIZE 0K"));
    display_Update();
  } else {
    if (!readrom) {
      CreateCHRFileInSD();
    }
    if (myFile) {
      switch (mapper) {
        case 0:  // 8K
          dumpBankCHR(0x0, 0x2000);
          break;

        case 1:
        case 155:
          banks = int_pow(2, chrsize);
          for (size_t i = 0; i < banks; i += 2) {  // 8K/16K/32K/64K/128K (Bank #s are based on 4K Banks)
            write_prg_byte(0x8000, 0x80);          // Clear Register
            write_mmc1_byte(0xA000, i);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 3:    // 8K/16K/32K - bus conflicts
        case 148:  // Sachen SA-008-A and Tengen 800008 - Bus conflicts
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            for (size_t x = 0; x < 0x8000; x++) {
              if (read_prg_byte(0x8000 + x) == i) {
                write_prg_byte(0x8000 + x, i);
                break;
              }
            }
            dumpBankCHR(0x0, 0x2000);
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
            write_prg_byte(0xA001, 0x80);          // Block Register - PRG RAM Chip Enable, Writable
          for (size_t i = 0; i < banks; i += 4) {  // 8K/16K/32K/64K/128K/256K
            if (mapper == 47) {
              if (i == 0)
                write_prg_byte(0x6000, 0);  // Switch to Lower Block
              else if (i == 128)
                write_prg_byte(0x6000, 1);  // Switch to Upper Block
            }
            write_prg_byte(0x8000, 0);  // CHR Bank 0 ($0000-$07FF)
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 1);  // CHR Bank 1 ($0800-$0FFF)
            write_prg_byte(0x8001, i + 2);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 5:  // 128K/256K/512K
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x5101, 0);  // 8K CHR Banks
          for (size_t i = 0; i < banks; i++) {
            if (i == 0)
              write_prg_byte(0x5130, 0);  // Set Upper 2 bits
            else if (i == 8)
              write_prg_byte(0x5130, 1);  // Set Upper 2 bits
            else if (i == 16)
              write_prg_byte(0x5130, 2);  // Set Upper 2 bits
            else if (i == 24)
              write_prg_byte(0x5130, 3);  // Set Upper 2 bits
            write_prg_byte(0x5127, i);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 9:
        case 10:  // Mapper 9: 128K, Mapper 10: 64K/128K
          if (mapper == 9)
            banks = 32;
          else  // Mapper 10
            banks = int_pow(2, chrsize);
          for (size_t i = 0; i < banks; i++) {  // 64K/128K
            write_prg_byte(0xB000, i);
            write_prg_byte(0xC000, i);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 11:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xFFB0 + i, i << 4);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 16:
        case 159:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x6000, i);  // Submapper 4
            write_prg_byte(0x8000, i);  // Submapper 5
            dumpBankCHR(0x0, 0x400);
          }
          break;

        case 18:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xA000, i & 0xF);         // CHR Bank Lower 4 bits
            write_prg_byte(0xA001, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits
            dumpBankCHR(0x0, 0x400);
          }
          break;

        case 19:                             // 128K/256K
          for (size_t j = 0; j < 64; j++) {  // Init Register
            write_ram_byte(0xE800, 0xC0);    // CHR RAM High/Low Disable (ROM Enable)
          }
          banks = int_pow(2, chrsize) * 4;
          write_ram_byte(0xE800, 0xC0);  // CHR RAM High/Low Disable (ROM Enable)
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0x8000, i);      // CHR Bank 0
            write_prg_byte(0x8800, i + 1);  // CHR Bank 1
            write_prg_byte(0x9000, i + 2);  // CHR Bank 2
            write_prg_byte(0x9800, i + 3);  // CHR Bank 3
            write_prg_byte(0xA000, i + 4);  // CHR Bank 4
            write_prg_byte(0xA800, i + 5);  // CHR Bank 5
            write_prg_byte(0xB000, i + 6);  // CHR Bank 6
            write_prg_byte(0xB800, i + 7);  // CHR Bank 7
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 21:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xB000, i & 0xF);           // CHR Bank Lower 4 bits
            if (chrsize == 5)                          // Check CHR Size to determine VRC4a (128K) or VRC4c (256K)
              write_prg_byte(0xB002, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4a (Wai Wai World 2)
            else                                       // banks == 256
              write_prg_byte(0xB040, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4c (Ganbare Goemon Gaiden 2)
            dumpBankCHR(0x0, 0x400);
          }
          break;

        case 22:  // 128K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xB000, (i << 1) & 0xF);  // CHR Bank Lower 4 bits
            write_prg_byte(0xB002, (i >> 3) & 0xF);  // CHR Bank Upper 4 bits
            dumpBankCHR(0x0, 0x400);
          }
          break;

        case 23:
          {  // 128K
            banks = int_pow(2, chrsize) * 4;

            // Detect VRC4e Carts - read PRG 0x1FFF6 (DATE)
            // Boku Dracula-kun = 890810, Tiny Toon = 910809
            // Crisis Force = 910701, Parodius Da! = 900916
            write_prg_byte(0x8000, 15);
            uint8_t prgchk0 = read_prg_byte(0x9FF6);
            if (prgchk0 == 0x30) {  // Check for "0" in middle of date. If true, assume VRC4e Cart
              for (size_t i = 0; i < banks; i++) {
                write_prg_byte(0xB000, i & 0xF);         // CHR Bank Lower 4 bits
                write_prg_byte(0xB004, (i >> 4) & 0xF);  // CHR Bank Upper 4 bits VRC4e
                dumpBankCHR(0x0, 0x400);
              }

              break;
            }

            // VRC2b/VRC4f - See https://www.nesdev.org/wiki/VRC2_and_VRC4
            for (size_t i = 0; i < banks; i += 8) {
              write_prg_byte(0xB000, i & 0xF);               // CHR Bank 0: Lower 4 bits
              write_prg_byte(0xB001, (i >> 4) & 0xF);        // CHR Bank 0: Upper 4 bits
              write_prg_byte(0xB002, (i + 1) & 0xF);         // CHR Bank 1: Lower 4 bits
              write_prg_byte(0xB003, ((i + 1) >> 4) & 0xF);  // CHR Bank 1: Upper 4 bits
              write_prg_byte(0xC000, (i + 2) & 0xF);         // CHR Bank 2: Lower 4 bits
              write_prg_byte(0xC001, ((i + 2) >> 4) & 0xF);  // CHR Bank 2: Upper 4 bits
              write_prg_byte(0xC002, (i + 3) & 0xF);         // CHR Bank 3: Lower 4 bits
              write_prg_byte(0xC003, ((i + 3) >> 4) & 0xF);  // CHR Bank 3: Upper 4 bits
              write_prg_byte(0xD000, (i + 4) & 0xF);         // CHR Bank 4: Lower 4 bits
              write_prg_byte(0xD001, ((i + 4) >> 4) & 0xF);  // CHR Bank 4: Upper 4 bits
              write_prg_byte(0xD002, (i + 5) & 0xF);         // CHR Bank 5: Lower 4 bits
              write_prg_byte(0xD003, ((i + 5) >> 4) & 0xF);  // CHR Bank 5: Upper 4 bits
              write_prg_byte(0xE000, (i + 6) & 0xF);         // CHR Bank 6: Lower 4 bits
              write_prg_byte(0xE001, ((i + 6) >> 4) & 0xF);  // CHR Bank 6: Upper 4 bits
              write_prg_byte(0xE002, (i + 7) & 0xF);         // CHR Bank 7: Lower 4 bits
              write_prg_byte(0xE003, ((i + 7) >> 4) & 0xF);  // CHR Bank 7: Upper 4 bits
              dumpBankCHR(0x0, 0x2000);                      // 8 Banks for a total of 8 KiB
            }
            break;
          }
        case 24:  // 128K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xB003, 0);  // PPU Banking Mode 0
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0xD000, i);      // CHR Bank 0
            write_prg_byte(0xD001, i + 1);  // CHR Bank 1
            write_prg_byte(0xD002, i + 2);  // CHR Bank 2
            write_prg_byte(0xD003, i + 3);  // CHR Bank 3
            write_prg_byte(0xE000, i + 4);  // CHR Bank 4 [WRITE NO RAM]
            write_prg_byte(0xE001, i + 5);  // CHR Bank 5 [WRITE NO RAM]
            write_prg_byte(0xE002, i + 6);  // CHR Bank 6 [WRITE NO RAM]
            write_prg_byte(0xE003, i + 7);  // CHR Bank 7 [WRITE NO RAM]
            dumpBankCHR(0x0, 0x2000);       // 1K Banks
          }
          break;

        case 25:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xB000, i & 0xF);         // CHR Bank Lower 4 bits
            write_prg_byte(0xB00A, (i >> 4) & 0xF);  // Combine VRC2c and VRC4b, VRC4d reg
            dumpBankCHR(0x0, 0x400);
          }
          break;

        case 26:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xB003, 0x00);
          for (size_t i = 0; i < banks; i += 4) {
            write_prg_byte(0xD000, i + 0);  // CHR Bank 0
            write_prg_byte(0xD002, i + 1);  // CHR Bank 1
            write_prg_byte(0xD001, i + 2);  // CHR Bank 2
            write_prg_byte(0xD003, i + 3);  // CHR Bank 3
            dumpBankCHR(0x0, 0x1000);       // 1K Banks
          }
          break;

        case 32:  // 128K
        case 65:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0xB000, i);      // CHR Bank 0
            write_prg_byte(0xB001, i + 1);  // CHR Bank 1
            write_prg_byte(0xB002, i + 2);  // CHR Bank 2
            write_prg_byte(0xB003, i + 3);  // CHR Bank 3
            write_prg_byte(0xB004, i + 4);  // CHR Bank 4
            write_prg_byte(0xB005, i + 5);  // CHR Bank 5
            write_prg_byte(0xB006, i + 6);  // CHR Bank 6
            write_prg_byte(0xB007, i + 7);  // CHR Bank 7
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 33:  // 128K/256K
        case 48:  // 256K
          banks = int_pow(2, chrsize) * 2;
          for (size_t i = 0; i < banks; i += 2) {  // 2K Banks
            write_prg_byte(0x8002, i);             // CHR Bank 0
            write_prg_byte(0x8003, i + 1);         // CHR Bank 1
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 34:  // NINA
          banks = int_pow(2, chrsize);
          for (size_t i = 0; i < banks; i += 2) {
            write_prg_byte(0x7FFE, i);      // Select 4 KB CHR bank at $0000
            write_prg_byte(0x7FFF, i + 1);  // Select 4 KB CHR bank at $1000
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 35:
        case 90:
        case 209:
        case 211:
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0xD000, 0x02);

          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xD003, (((i >> 3) & 0x18) | 0x20));
            write_prg_byte(0x9000, (i & 0x3f));
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 36:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x4200, i);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 37:
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xA001, 0x80);            // Block Register - PRG RAM Chip Enable, Writable
          for (size_t i = 0; i < banks; i += 4) {  // 256K
            if (i == 0)
              write_prg_byte(0x6000, 0);  // Switch to Lower Block ($00000-$1FFFF)
            else if (i == 128)
              write_prg_byte(0x6000, 4);  // Switch to Upper Block ($20000-$3FFFF)
            write_prg_byte(0x8000, 0);    // CHR Bank 0 ($0000-$07FF)
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 1);  // CHR Bank 1 ($0800-$0FFF)
            write_prg_byte(0x8001, i + 2);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 38:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x7000, i << 2);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 42:
          banks = int_pow(2, chrsize);
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000, i & 0x0F);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 45:  // 128K/256K/512K/1024K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xA001, 0x80);  // Unlock Write Protection - not used by some carts
          for (size_t i = 0; i < banks; i++) {
            // set outer bank registers
            write_prg_byte(0x6000, 0x00);                       // CHR-OR
            write_prg_byte(0x6000, 0x00);                       // PRG-OR
            write_prg_byte(0x6000, (((i / 256) << 4) | 0x0F));  // CHR-AND,CHR-OR/PRG-OR
            write_prg_byte(0x6000, 0x80);                       // PRG-AND
            // set inner bank registers
            write_prg_byte(0x8000, 0x2);  // CHR Bank 2 ($1000-$13FF)
            write_prg_byte(0x8001, i);
            for (size_t address = 0x1000; address < 0x1200; address += 512) {
              dumpCHR_M2(address);  // Read CHR with M2 Pulse
            }
            // set outer bank registers
            write_prg_byte(0x6000, 0x00);                       // CHR-OR
            write_prg_byte(0x6000, 0x00);                       // PRG-OR
            write_prg_byte(0x6000, (((i / 256) << 4) | 0x0F));  // CHR-AND,CHR-OR/PRG-OR
            write_prg_byte(0x6000, 0x80);                       // PRG-AND
            // set inner bank registers
            write_prg_byte(0x8000, 0x2);  // CHR Bank 2 ($1000-$13FF)
            write_prg_byte(0x8001, i);
            for (size_t address = 0x1200; address < 0x1400; address += 512) {
              dumpCHR_M2(address);  // Read CHR with M2 Pulse
            }
          }
          break;

        case 46:
          banks = int_pow(2, chrsize);  // 8k banks
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x6000, (i & 0x78) << 1);  // high bits
            write_prg_byte(0x8000, (i & 0x07) << 4);  // low bits
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 52:
          banks = int_pow(2, chrsize);
          write_prg_byte(0xA001, 0x80);  // enable WRAM write
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x6000, (i & 0x04) << 2 | (i & 0x03) << 4 | 0x40);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 56:
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0xFC00, i);
            dumpBankCHR(0x0, 0x400);
          }
          break;

        case 57:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8800, i & 0x07);                  // A15-A13
            write_prg_byte(0x8000, 0x80 | ((i & 0x08) << 3));  // A16
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 58:
        case 213:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + ((i & 0x07) << 3), 0x00);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 59:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + (i & 0x07), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 60:
          for (size_t i = 0; i < 4; i++) {
            write_prg_byte(0x8D8D, i);
            delay(500);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 62:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + (i / 4), i & 3);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 66:  // 16K/32K
        case 70:
        case 152:  // 128K
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            write_prg_byte(0x8000, i);          // CHR Bank 0
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 67:  // 128K
          banks = int_pow(2, chrsize) * 2;
          for (size_t i = 0; i < banks; i += 4) {  // 2K Banks
            write_prg_byte(0x8800, i);             // CHR Bank 0
            write_prg_byte(0x9800, i + 1);         // CHR Bank 1
            write_prg_byte(0xA800, i + 2);         // CHR Bank 2
            write_prg_byte(0xB800, i + 3);         // CHR Bank 3
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 68:  // 128K/256K
          banks = int_pow(2, chrsize) * 2;
          for (size_t i = 0; i < banks; i += 4) {  // 2K Banks
            write_prg_byte(0x8000, i);             // CHR Bank 0
            write_prg_byte(0x9000, i + 1);         // CHR Bank 1
            write_prg_byte(0xA000, i + 2);         // CHR Bank 2
            write_prg_byte(0xB000, i + 3);         // CHR Bank 3
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 69:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000, 0);  // Command Register - CHR Bank 0
            write_prg_byte(0xA000, i);  // Parameter Register - ($0000-$03FF)
            dumpBankCHR(0x0, 0x400);    // 1K Banks
          }
          break;

        case 72:  // 128K
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x8000, 0);            // Reset Register
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            write_prg_byte(0x8000, i | 0x40);   // CHR Command + Bank
            write_prg_byte(0x8000, i);          // CHR Bank
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 75:  // 128K
          banks = int_pow(2, chrsize);
          for (size_t i = 0; i < banks; i++) {        // 4K Banks
            write_reg_byte(0xE000, i);                // CHR Bank Low Bits [WRITE RAM SAFE]
            write_prg_byte(0x9000, (i & 0x10) >> 3);  // High Bit
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 76:  // 128K
          banks = int_pow(2, chrsize) * 2;
          for (size_t i = 0; i < banks; i += 2) {  // 2K Banks
            write_prg_byte(0x8000, 2);             // CHR Command ($0000-$07FF) 2K Bank
            write_prg_byte(0x8001, i);             // CHR Bank
            write_prg_byte(0x8000, 3);             // CHR Command ($0800-$0FFF) 2K Bank
            write_prg_byte(0x8001, i + 1);         // CHR Bank
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 77:  // 32K
          banks = int_pow(2, chrsize) * 2;
          for (size_t i = 0; i < banks; i++) {  // 2K Banks
            write_prg_byte(0x8000, i << 4);     // CHR Bank 0
            dumpBankCHR(0x0, 0x800);
          }
          break;

        case 78:  // 128K
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            write_prg_byte(0x8000, i << 4);     // CHR Bank 0
            dumpBankCHR(0x0, 0x2000);           // 8K Banks ($0000-$1FFF)
          }
          break;

        case 79:
        case 146:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x4100, i);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 80:   // 128K/256K
        case 82:   // 128K/256K
        case 207:  // 128K [CART SOMETIMES NEEDS POWERCYCLE]
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i += 4) {
            write_prg_byte(0x7EF2, i);      // CHR Bank 2 [REGISTERS 0x7EF0/0x7EF1 WON'T WORK]
            write_prg_byte(0x7EF3, i + 1);  // CHR Bank 3
            write_prg_byte(0x7EF4, i + 2);  // CHR Bank 4
            write_prg_byte(0x7EF5, i + 3);  // CHR Bank 5
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 85:  // 128K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0xA000, i);      // CHR Bank 0
            write_prg_byte(0xA008, i + 1);  // CHR Bank 1
            write_prg_byte(0xB000, i + 2);  // CHR Bank 2
            write_prg_byte(0xB008, i + 3);  // CHR Bank 3
            write_prg_byte(0xC000, i + 4);  // CHR Bank 4
            write_prg_byte(0xC008, i + 5);  // CHR Bank 5
            write_prg_byte(0xD000, i + 6);  // CHR Bank 6
            write_prg_byte(0xD008, i + 7);  // CHR Bank 7
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 86:  // 64K
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            if (i < 4)
              write_prg_byte(0x6000, i & 0x3);
            else
              write_prg_byte(0x6000, (i | 0x40) & 0x43);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 87:  // 16K/32K
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {  // 16K/32K
            write_prg_byte(0x6000, (((i & 0x1) << 1) | ((i & 0x2) >> 1)));
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 88:   // 128K
        case 95:   // 32K
        case 154:  // 128K
        case 206:  // 16K/32K/64K
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i += 2) {  // 1K Banks
            if (i < 64) {
              write_prg_byte(0x8000, 0);         // CHR Command ($0000-$07FF) 2K Bank
              write_prg_byte(0x8001, i & 0x3F);  // CHR Bank
              dumpBankCHR(0x0, 0x800);
            } else {
              write_prg_byte(0x8000, 2);      // CHR Command ($1000-$13FF) 1K Bank
              write_prg_byte(0x8001, i);      // CHR Bank
              write_prg_byte(0x8000, 3);      // CHR Command ($1400-$17FF) 1K Bank
              write_prg_byte(0x8001, i + 1);  // CHR Bank
              dumpBankCHR(0x1000, 0x1800);
            }
          }
          break;

        case 89:  // 128K
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            if (i < 8)
              write_prg_byte(0x8000, i & 0x7);
            else
              write_prg_byte(0x8000, (i | 0x80) & 0x87);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 91:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0x6000, (i / 2) | 0);
            write_prg_byte(0x6001, (i / 2) | 1);
            write_prg_byte(0x6002, (i / 2) | 2);
            write_prg_byte(0x6003, (i / 2) | 3);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 92:  // 128K
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x8000, 0);            // Reset Register
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            write_prg_byte(0x8000, i | 0x40);   // CHR Command + Bank
            write_prg_byte(0x8000, i);          // CHR Bank
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 113:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x4100, (i & 0x08) << 3 | (i & 0x07));
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 114:  // Submapper 0
          banks = int_pow(2, chrsize) * 4;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x6000, (i & 0x80) >> 7);
            write_prg_byte(0xA000, 6);
            write_prg_byte(0xC000, i);
            dumpBankCHR(0x1000, 0x1400);
          }
          break;

        case 126:
          banks = int_pow(2, chrsize) * 2;
          write_prg_byte(0xA001, 0x80);  // enable WRAM
          write_prg_byte(0x6003, 0x00);  // set MMC3 banking mode
          for (size_t i = 0; i < banks; i += 2) {
            write_prg_byte(0x6000, (i & 0x200) >> 5 | (i & 0x100) >> 3);  // select outer bank
            write_prg_byte(0x8000, 0);                                    // 2k bank 0 at $0000
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 1);  // 2k bank 1 at $0800
            write_prg_byte(0x8001, i + 2);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 134:
          banks = int_pow(2, chrsize) * 2;
          write_prg_byte(0x6000, 0x00);  // set MMC3 banking mode
          for (size_t i = 0; i < banks; i += 2) {
            write_prg_byte(0x6001, (i & 0x180) >> 3);  // select outer bank
            write_prg_byte(0x8000, 0);                 // 2k bank 0 at $0000
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 1);  // 2k bank 1 at $0800
            write_prg_byte(0x8001, i + 2);
            dumpBankCHR(0x0, 0x1000);
          }
          break;

        case 140:  // 32K/128K
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {  // 8K Banks
            write_prg_byte(0x6000, i);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 174:  // 64k
          for (size_t i = 0; i < 8; i++) {
            write_prg_byte(0xFF00 + (i << 1), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 176:
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0x5FF3, 0);  // extended MMC3 mode: disabled
          write_prg_byte(0x5FF0, 1);  // 256K outer bank mode
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0x5FF2, (i & 0x700) >> 3);  // outer 256k bank
            write_prg_byte(0x8000, 0);
            write_prg_byte(0x8001, i);
            write_prg_byte(0x8000, 0x0A);
            write_prg_byte(0x8001, i + 1);
            write_prg_byte(0x8000, 1);
            write_prg_byte(0x8001, i + 2);
            write_prg_byte(0x8000, 0x0B);
            write_prg_byte(0x8001, i + 3);
            write_prg_byte(0x8000, 2);
            write_prg_byte(0x8001, i + 4);
            write_prg_byte(0x8000, 3);
            write_prg_byte(0x8001, i + 5);
            write_prg_byte(0x8000, 4);
            write_prg_byte(0x8001, i + 6);
            write_prg_byte(0x8000, 5);
            write_prg_byte(0x8001, i + 7);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 184:  // 16K/32K
          banks = int_pow(2, chrsize);
          for (size_t i = 0; i < banks; i++) {  // 4K Banks
            write_prg_byte(0x6000, i);          // CHR LOW (Bits 0-2) ($0000-$0FFF)
            dumpBankCHR(0x0, 0x1000);           // 4K Banks ($0000-$0FFF)
          }
          break;

        case 185:                           // 8K [READ 32K TO OVERRIDE LOCKOUT]
          for (size_t i = 0; i < 4; i++) {  // Read 32K to locate valid 8K
            write_prg_byte(0x8000, i);
            uint8_t chrcheck = read_chr_byte(0);
            for (size_t address = 0x0; address < 0x2000; address += 512) {
              for (size_t x = 0; x < 512; x++) {
                sdBuffer[x] = read_chr_byte(address + x);
              }
              if (chrcheck != 0xFF)
                myFile.write(sdBuffer, 512);
            }
          }
          break;

        case 200:
        case 212:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + (i & 0x07), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 201:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + (i & 0xFF), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 202:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 | (i << 1), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 203:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000, (i & 0x03));
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 210:  // 128K/256K
          banks = int_pow(2, chrsize) * 4;
          write_prg_byte(0xE800, 0xC0);  // CHR RAM DISABLE (Bit 6 and 7) [WRITE NO RAM]
          for (size_t i = 0; i < banks; i += 8) {
            write_prg_byte(0x8000, i);      // CHR Bank 0
            write_prg_byte(0x8800, i + 1);  // CHR Bank 1
            write_prg_byte(0x9000, i + 2);  // CHR Bank 2
            write_prg_byte(0x9800, i + 3);  // CHR Bank 3
            write_prg_byte(0xA000, i + 4);  // CHR Bank 4
            write_prg_byte(0xA800, i + 5);  // CHR Bank 5
            write_prg_byte(0xB000, i + 6);  // CHR Bank 6
            write_prg_byte(0xB800, i + 7);  // CHR Bank 7
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 214:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 | (i << 2), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 225:
        case 255:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + (((i & 0x40) << 8) | (i & 0x3F)), 0);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 228:
          banks = int_pow(2, chrsize) / 2;
          write_prg_byte(0x8000, 0);
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 + ((i & 0x3C) >> 2), i & 0x03);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 229:
          for (size_t i = 0; i < 32; i++) {
            write_prg_byte(0x8000 + i, i);
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 236:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x8000 | (i & 0x0F), 0);  // A16-A13
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 240:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i++) {
            write_prg_byte(0x5FFF, (i & 0xF));
            dumpBankCHR(0x0, 0x2000);
          }
          break;

        case 246:
          banks = int_pow(2, chrsize) / 2;
          for (size_t i = 0; i < banks; i += 4) {
            write_prg_byte(0x6004, (i | 0));
            write_prg_byte(0x6005, (i | 1));
            write_prg_byte(0x6006, (i | 2));
            write_prg_byte(0x6007, (i | 3));
            dumpBankCHR(0x0, 0x2000);
          }
          break;
      }
      if (!readrom) {
        myFile.flush();
        myFile.close();

        println_Msg(F("CHR FILE DUMPED!"));
        println_Msg(FS(FSTRING_EMPTY));
        display_Update();
      }
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  rgbLed(black_color);
}

/******************************************
   RAM Functions
 *****************************************/
void readRAM() {
  display_Clear();
  display_Update();

  uint16_t banks;

  rgbLed(turquoise_color);
  set_address(0);
  _delay_us(1);
  if (ramsize == 0) {

    println_Msg(F("RAM SIZE 0K"));
    display_Update();
  } else {
    CreateRAMFileInSD();
    word base = 0x6000;
    if (myFile) {
      switch (mapper) {
        case 0:                                       // 2K/4K
          dumpBankPRG(0x0, (0x800 * ramsize), base);  // 2K/4K
          break;                                      // SWITCH MUST BE IN OFF POSITION

        case 1:
        case 155:                               // 8K/16K/32K
          banks = int_pow(2, ramsize) / 2;      // banks = 1,2,4
          for (size_t i = 0; i < banks; i++) {  // 8K Banks ($6000-$7FFF)
            write_prg_byte(0x8000, 0x80);       // Clear Register
            write_mmc1_byte(0x8000, 1 << 3);
            write_mmc1_byte(0xE000, 0);
            if (banks == 4)  // 32K
              write_mmc1_byte(0xA000, i << 2);
            else
              write_mmc1_byte(0xA000, i << 3);
            dumpBankPRG(0x0, 0x2000, base);  // 8K
          }
          break;

        case 4:                                                                // 1K/8K (MMC6/MMC3)
          if (mmc6) {                                                          // MMC6 1K
            write_prg_byte(0x8000, 0x20);                                      // PRG RAM ENABLE
            write_prg_byte(0xA001, 0x20);                                      // PRG RAM PROTECT - Enable reading RAM at $7000-$71FF
            for (size_t address = 0x1000; address < 0x1200; address += 512) {  // 512B
              dumpMMC5RAM(base, address);
            }
            write_prg_byte(0x8000, 0x20);                                      // PRG RAM ENABLE
            write_prg_byte(0xA001, 0x80);                                      // PRG RAM PROTECT - Enable reading RAM at $7200-$73FF
            for (size_t address = 0x1200; address < 0x1400; address += 512) {  // 512B
              dumpMMC5RAM(base, address);
            }
            write_prg_byte(0x8000, 6);       // PRG RAM DISABLE
          } else {                           // MMC3 8K
            write_prg_byte(0xA001, 0xC0);    // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
            dumpBankPRG(0x0, 0x2000, base);  // 8K
          }
          break;

        case 5:                                         // 8K/16K/32K
          write_prg_byte(0x5100, 3);                    // 8K PRG Banks
          banks = int_pow(2, ramsize) / 2;              // banks = 1,2,4
          if (banks == 2) {                             // 16K - Split SRAM Chips 8K/8K
            for (size_t i = 0; i < (banks / 2); i++) {  // Chip 1
              write_prg_byte(0x5113, i);
              for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                dumpMMC5RAM(base, address);
              }
            }
            for (size_t j = 4; j < (banks / 2) + 4; j++) {  // Chip 2
              write_prg_byte(0x5113, j);
              for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                dumpMMC5RAM(base, address);
              }
            }
          } else {                                // 8K/32K Single SRAM Chip
            for (size_t i = 0; i < banks; i++) {  // banks = 1 or 4
              write_prg_byte(0x5113, i);
              for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                dumpMMC5RAM(base, address);
              }
            }
          }
          break;

        case 16:  // 256-byte EEPROM 24C02
        case 159:
          {  // 128-byte EEPROM 24C01 [Little Endian]
            size_t eepsize;
            if (mapper == 159)
              eepsize = 128;
            else
              eepsize = 256;
            for (size_t address = 0; address < eepsize; address++) {
              EepromREAD(address);
            }
            myFile.write(sdBuffer, eepsize);
            //          display_Clear(); // TEST PURPOSES - DISPLAY EEPROM DATA
            break;
          }
        case 19:
          if (ramsize == 2) {  // PRG RAM 128B
            for (size_t x = 0; x < 128; x++) {
              write_ram_byte(0xF800, x);            // PRG RAM ENABLE
              sdBuffer[x] = read_prg_byte(0x4800);  // DATA PORT
            }
            myFile.write(sdBuffer, 128);
          } else {                             // SRAM 8K
            for (size_t i = 0; i < 64; i++) {  // Init Register
              write_ram_byte(0xE000, 0);
            }
            dumpBankPRG(0x0, 0x2000, base);  // 8K
          }
          break;

        case 80:                              // 1K
          write_prg_byte(0x7EF8, 0xA3);       // PRG RAM ENABLE 0
          write_prg_byte(0x7EF9, 0xA3);       // PRG RAM ENABLE 1
          for (size_t x = 0; x < 128; x++) {  // PRG RAM 1K ($7F00-$7FFF) MIRRORED ONCE
            sdBuffer[x] = read_prg_byte(0x7F00 + x);
          }
          myFile.write(sdBuffer, 128);
          write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 0
          write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 1
          break;

        case 82:                                                          // 5K
          write_prg_byte(0x7EF7, 0xCA);                                   // PRG RAM ENABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0x69);                                   // PRG RAM ENABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0x84);                                   // PRG RAM ENABLE 2 ($7000-$73FF)
          for (size_t address = 0x0; address < 0x1400; address += 512) {  // PRG RAM 5K ($6000-$73FF)
            dumpMMC5RAM(base, address);
          }
          write_prg_byte(0x7EF7, 0xFF);  // PRG RAM DISABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 2 ($7000-$73FF)
          break;

        default:
          if (mapper == 118)               // 8K
            write_prg_byte(0xA001, 0xC0);  // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          else if (mapper == 19) {
            for (size_t i = 0; i < 64; i++) {  // Init Register
              write_ram_byte(0xE000, 0);
            }
          } else if ((mapper == 21) || (mapper == 25))  // 8K
            write_prg_byte(0x8000, 0);
          else if (mapper == 26)           // 8K
            write_prg_byte(0xB003, 0x80);  // PRG RAM ENABLE
          else if (mapper == 68)           // 8K
            write_reg_byte(0xF000, 0x10);  // PRG RAM ENABLE [WRITE RAM SAFE]
          else if (mapper == 69) {         // 8K
            write_prg_byte(0x8000, 8);     // Command Register - PRG Bank 0
            write_prg_byte(0xA000, 0xC0);  // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
          } else if (mapper == 85)         // 8K
            write_ram_byte(0xE000, 0x80);  // PRG RAM ENABLE
          else if (mapper == 153)          // 8K
            write_prg_byte(0x800D, 0x20);  // PRG RAM Chip Enable
          dumpBankPRG(0x0, 0x2000, base);  // 8K
          if (mapper == 85)                // 8K
            write_reg_byte(0xE000, 0);     // PRG RAM DISABLE [WRITE RAM SAFE]
          break;
      }
      myFile.flush();
      myFile.close();

      println_Msg(F("RAM FILE DUMPED!"));
      println_Msg(FS(FSTRING_EMPTY));
      display_Update();

      if ((mapper == 16) || (mapper == 159))
        printCRC(fileName, NULL, 0);
      else
        printCRC(fileName, NULL, 0);
    }
  }
  set_address(0);
  PHI2_HI;
  ROMSEL_HI;
  rgbLed(black_color);
}

void writeBankPRG(const size_t from, const size_t to, const size_t base) {
  for (size_t address = from; address < to; address += 512) {
    myFile.read(sdBuffer, 512);
    for (size_t x = 0; x < 512; x++) {
      write_prg_byte(base + address + x, sdBuffer[x]);
    }
  }
}

void writeBankWRAM(const size_t from, const size_t to, const size_t base) {
  for (size_t address = from; address < to; address += 512) {
    myFile.read(sdBuffer, 512);
    for (size_t x = 0; x < 512; x++) {
      write_wram_byte(base + address + x, sdBuffer[x]);
    }
  }
}

void writeRAM() {
  display_Clear();

  if (ramsize == 0) {
    print_Error(F("RAM SIZE 0K"));
  } else {
    fileBrowser(F("Select RAM File"));
    word base = 0x6000;
    uint16_t banks;

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
        case 0:                                        // 2K/4K
          writeBankPRG(0x0, (0x800 * ramsize), base);  // 2K/4K
          break;                                       // SWITCH MUST BE IN OFF POSITION

        case 1:
        case 155:
          banks = int_pow(2, ramsize) / 2;      // banks = 1,2,4
          for (size_t i = 0; i < banks; i++) {  // 8K Banks ($6000-$7FFF)
            write_prg_byte(0x8000, 0x80);       // Clear Register
            write_mmc1_byte(0x8000, 1 << 3);    // PRG ROM MODE 32K
            write_mmc1_byte(0xE000, 0);         // PRG RAM ENABLED
            if (banks == 4)                     // 32K
              write_mmc1_byte(0xA000, i << 2);
            else
              write_mmc1_byte(0xA000, i << 3);
            writeBankPRG(0x0, 0x2000, base);  // 8K
          }
          break;

        case 4:                                   // 1K/8K (MMC6/MMC3)
          if (mmc6) {                             // MMC6 1K
            write_prg_byte(0x8000, 0x20);         // PRG RAM ENABLE
            write_prg_byte(0xA001, 0x30);         // PRG RAM PROTECT - Enable reading/writing to RAM at $7000-$71FF
            writeBankWRAM(0x1000, 0x1200, base);  // 512B

            write_prg_byte(0x8000, 0x20);         // PRG RAM ENABLE
            write_prg_byte(0xA001, 0xC0);         // PRG RAM PROTECT - Enable reading/writing to RAM at $7200-$73FF
            writeBankWRAM(0x1200, 0x1400, base);  // 512B

            write_prg_byte(0x8000, 0x6);      // PRG RAM DISABLE
          } else {                            // MMC3 8K
            write_prg_byte(0xA001, 0x80);     // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
            writeBankPRG(0x0, 0x2000, base);  // 8K
            write_prg_byte(0xA001, 0xC0);     // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          }
          break;

        case 5:                                         // 8K/16K/32K
          write_prg_byte(0x5100, 3);                    // 8K PRG Banks
          banks = int_pow(2, ramsize) / 2;              // banks = 1,2,4
          if (banks == 2) {                             // 16K - Split SRAM Chips 8K/8K [ETROM = 16K (ONLY 1ST 8K BATTERY BACKED)]
            for (size_t i = 0; i < (banks / 2); i++) {  // Chip 1
              write_prg_byte(0x5113, i);
              for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                writeMMC5RAM(base, address);
              }
            }
            for (size_t j = 4; j < (banks / 2) + 4; j++) {  // Chip 2
              write_prg_byte(0x5113, j);
              for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                writeMMC5RAM(base, address);
              }
            }
          } else {                                // 8K/32K Single SRAM Chip [EKROM = 8K BATTERY BACKED, EWROM = 32K BATTERY BACKED]
            for (size_t i = 0; i < banks; i++) {  // banks = 1 or 4
              write_prg_byte(0x5113, i);
              for (size_t address = 0; address < 0x2000; address += 512) {  // 8K
                writeMMC5RAM(base, address);
              }
            }
          }
          break;

        case 16:  // 256-byte EEPROM 24C02
        case 159:
          {  // 128-byte EEPROM 24C01 [Little Endian]
            size_t eepsize;
            if (mapper == 159)
              eepsize = 128;
            else
              eepsize = 256;
            myFile.read(sdBuffer, eepsize);
            for (size_t address = 0; address < eepsize; address++) {
              EepromWRITE(address);
              if ((address % 128) == 0)
                display_Clear();
              print_Msg(F("."));
              display_Update();
            }
            break;
          }
        case 19:
          if (ramsize == 2) {  // PRG RAM 128B
            myFile.read(sdBuffer, 128);
            for (size_t x = 0; x < 128; x++) {
              write_ram_byte(0xF800, x);            // PRG RAM ENABLE
              write_prg_byte(0x4800, sdBuffer[x]);  // DATA PORT
            }
          } else {                             // SRAM 8K
            for (size_t i = 0; i < 64; i++) {  // Init Register
              write_ram_byte(0xF800, 0x40);    // PRG RAM WRITE ENABLE
            }
            write_ram_byte(0xF800, 0x40);     // PRG RAM WRITE ENABLE
            writeBankPRG(0x0, 0x2000, base);  // 8K
            write_ram_byte(0xF800, 0x0F);     // PRG RAM WRITE PROTECT
          }
          break;

        case 80:                                                             // 1K
          write_prg_byte(0x7EF8, 0xA3);                                      // PRG RAM ENABLE 0
          write_prg_byte(0x7EF9, 0xA3);                                      // PRG RAM ENABLE 1
          for (size_t address = 0x1F00; address < 0x2000; address += 512) {  // PRG RAM 1K ($7F00-$7FFF)
            myFile.read(sdBuffer, 128);
            for (size_t x = 0; x < 128; x++) {
              write_prg_byte(base + address + x, sdBuffer[x]);
            }
          }
          write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 0
          write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 1
          break;

        case 82:                                                           // 5K
          write_prg_byte(0x7EF7, 0xCA);                                    // PRG RAM ENABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0x69);                                    // PRG RAM ENABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0x84);                                    // PRG RAM ENABLE 2 ($7000-$73FF)
          for (size_t address = 0x0; address < 0x1400; address += 1024) {  // PRG RAM 5K ($6000-$73FF)
            myFile.read(sdBuffer, 512);
            uint8_t firstbyte = sdBuffer[0];
            for (size_t x = 0; x < 512; x++)
              write_prg_byte(base + address + x, sdBuffer[x]);
            myFile.read(sdBuffer, 512);
            for (size_t x = 0; x < 512; x++)
              write_prg_byte(base + address + x + 512, sdBuffer[x]);
            write_prg_byte(base + address, firstbyte);  // REWRITE 1ST BYTE
          }
          write_prg_byte(0x7EF7, 0xFF);  // PRG RAM DISABLE 0 ($6000-$67FF)
          write_prg_byte(0x7EF8, 0xFF);  // PRG RAM DISABLE 1 ($6800-$6FFF)
          write_prg_byte(0x7EF9, 0xFF);  // PRG RAM DISABLE 2 ($7000-$73FF)
          break;

        default:
          if (mapper == 118)                          // 8K
            write_prg_byte(0xA001, 0x80);             // PRG RAM CHIP ENABLE - Chip Enable, Allow Writes
          else if ((mapper == 21) || (mapper == 25))  // 8K
            write_prg_byte(0x8000, 0);
          else if (mapper == 26)           // 8K
            write_prg_byte(0xB003, 0x80);  // PRG RAM ENABLE
          //            else if (mapper == 68) // 8K
          //              write_reg_byte(0xF000, 0x10); // PRG RAM ENABLE [WRITE RAM SAFE]
          else if (mapper == 69) {         // 8K
            write_prg_byte(0x8000, 8);     // Command Register - PRG Bank 0
            write_prg_byte(0xA000, 0xC0);  // Parameter Register - PRG RAM Enabled, PRG RAM, Bank 0 to $6000-$7FFF
          } else if (mapper == 85)         // 8K
            write_ram_byte(0xE000, 0x80);  // PRG RAM ENABLE
          else if (mapper == 153)          // 8K
            write_prg_byte(0x800D, 0x20);  // PRG RAM Chip Enable
          writeBankPRG(0x0, 0x2000, base);

          if (mapper == 118)               // 8K
            write_prg_byte(0xA001, 0xC0);  // PRG RAM CHIP ENABLE - Chip Enable, Write Protect
          else if (mapper == 26)           // 8K
            write_prg_byte(0xB003, 0);     // PRG RAM DISABLE
          //            else if (mapper == 68) // 8K
          //              write_reg_byte(0xF000, 0x00); // PRG RAM DISABLE [WRITE RAM SAFE]
          else if (mapper == 69) {      // 8K
            write_prg_byte(0x8000, 8);  // Command Register - PRG Bank 0
            write_prg_byte(0xA000, 0);  // Parameter Register - PRG RAM Disabled, PRG ROM, Bank 0 to $6000-$7FFF
          } else if (mapper == 85)      // 8K
            write_reg_byte(0xE000, 0);  // PRG RAM DISABLE [WRITE RAM SAFE]
          break;
      }
      myFile.close();
      rgbLed(green_color);

      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("RAM FILE WRITTEN!"));
      display_Update();

    } else {
      print_FatalError(sd_error_STR);
    }
  }

  rgbLed(black_color);
  sd.chdir();          // root
  filePath[0] = '\0';  // Reset filePath
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
  EEPROM_writeAnything(0, 0);   // FOLDER #
  EEPROM_writeAnything(2, 0);   // CARTMODE
  EEPROM_writeAnything(3, 0);   // RETRY
  EEPROM_writeAnything(4, 0);   // STATUS
  EEPROM_writeAnything(5, 0);   // UNKNOWNCRC
  EEPROM_writeAnything(6, 1);   // LED (RESET TO ON)
  EEPROM_writeAnything(7, 0);   // MAPPER
  EEPROM_writeAnything(9, 0);   // PRG SIZE
  EEPROM_writeAnything(10, 0);  // CHR SIZE
  EEPROM_writeAnything(11, 0);  // RAM SIZE
}

void EepromStart_NES() {
  write_prg_byte(0x800D, 0x00);  // sda low, scl low
  write_prg_byte(0x800D, 0x60);  // sda, scl high
  write_prg_byte(0x800D, 0x20);  // sda low, scl high
  write_prg_byte(0x800D, 0x00);  // START
}

void EepromStop_NES() {
  write_prg_byte(0x800D, 0x00);  // sda, scl low
  write_prg_byte(0x800D, 0x20);  // sda low, scl high
  write_prg_byte(0x800D, 0x60);  // sda, scl high
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
  write_prg_byte(0x800D, 0x00);  // STOP
}

void EepromSet0_NES() {
  write_prg_byte(0x800D, 0x00);  // sda low, scl low
  write_prg_byte(0x800D, 0x20);  // sda low, scl high // 0
  write_prg_byte(0x800D, 0x00);  // sda low, scl low
}

void EepromSet1_NES() {
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
  write_prg_byte(0x800D, 0x60);  // sda high, scl high // 1
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
  write_prg_byte(0x800D, 0x00);  // sda low, scl low
}

void EepromStatus_NES() {        // ACK
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
  write_prg_byte(0x800D, 0x60);  // sda high, scl high
  write_prg_byte(0x800D, 0xE0);  // sda high, scl high, read high
  uint8_t eepStatus = 1;
  do {
    eepStatus = (read_prg_byte(0x6000) & 0x10) >> 4;
    delayMicroseconds(4);
  } while (eepStatus == 1);
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
}

void EepromReadData_NES() {
  // read serial data into buffer
  for (uint8_t i = 0; i < 8; i++) {
    write_prg_byte(0x800D, 0x60);                     // sda high, scl high, read low
    write_prg_byte(0x800D, 0xE0);                     // sda high, scl high, read high
    eepbit[i] = (read_prg_byte(0x6000) & 0x10) >> 4;  // Read 0x6000 with Mask 0x10 (bit 4)
    write_prg_byte(0x800D, 0x40);                     // sda high, scl low
  }
}

void EepromDevice_NES() {  // 24C02 ONLY
  EepromSet1_NES();
  EepromSet0_NES();
  EepromSet1_NES();
  EepromSet0_NES();
  EepromSet0_NES();  // A2
  EepromSet0_NES();  // A1
  EepromSet0_NES();  // A0
}

void EepromReadMode_NES() {
  EepromSet1_NES();    // READ
  EepromStatus_NES();  // ACK
}
void EepromWriteMode_NES() {
  EepromSet0_NES();    // WRITE
  EepromStatus_NES();  // ACK
}

void EepromFinish_NES() {
  write_prg_byte(0x800D, 0x00);  // sda low, scl low
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
  write_prg_byte(0x800D, 0x60);  // sda high, scl high
  write_prg_byte(0x800D, 0x40);  // sda high, scl low
  write_prg_byte(0x800D, 0x00);  // sda low, scl low
}

void EepromSetAddress01(uint8_t address) {  // 24C01 [Little Endian]
  for (uint8_t i = 0; i < 7; i++) {
    if (address & 0x1)  // Bit is HIGH
      EepromSet1_NES();
    else  // Bit is LOW
      EepromSet0_NES();
    address >>= 1;  // rotate to the next bit
  }
}

void EepromSetAddress02(uint8_t address) {  // 24C02
  for (uint8_t i = 0; i < 8; i++) {
    if ((address >> 7) & 0x1)  // Bit is HIGH
      EepromSet1_NES();
    else  // Bit is LOW
      EepromSet0_NES();
    address <<= 1;  // rotate to the next bit
  }
  EepromStatus_NES();  // ACK
}

void EepromWriteData01() {  // 24C01 [Little Endian]
  for (uint8_t i = 0; i < 8; i++) {
    if (eeptemp & 0x1)  // Bit is HIGH
      EepromSet1_NES();
    else  // Bit is LOW
      EepromSet0_NES();
    eeptemp >>= 1;  // rotate to the next bit
  }
  EepromStatus_NES();  // ACK
}

void EepromWriteData02() {  // 24C02
  for (uint8_t i = 0; i < 8; i++) {
    if ((eeptemp >> 7) & 0x1)  // Bit is HIGH
      EepromSet1_NES();
    else  // Bit is LOW
      EepromSet0_NES();
    eeptemp <<= 1;  // rotate to the next bit
  }
  EepromStatus_NES();  // ACK
}

void EepromREAD(uint8_t address) {
  EepromStart_NES();              // START
  if (mapper == 159) {            // 24C01
    EepromSetAddress01(address);  // 24C01 [Little Endian]
    EepromReadMode_NES();
    EepromReadData_NES();
    EepromFinish_NES();
    EepromStop_NES();  // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[7] << 7 | eepbit[6] << 6 | eepbit[5] << 5 | eepbit[4] << 4 | eepbit[3] << 3 | eepbit[2] << 2 | eepbit[1] << 1 | eepbit[0];
  } else {               // 24C02
    EepromDevice_NES();  // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromStart_NES();   // START
    EepromDevice_NES();  // DEVICE [1010] + ADDR [A2-A0]
    EepromReadMode_NES();
    EepromReadData_NES();
    EepromFinish_NES();
    EepromStop_NES();  // STOP
    // OR 8 bits into byte
    eeptemp = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  }
  sdBuffer[address] = eeptemp;
}

void EepromWRITE(uint8_t address) {
  eeptemp = sdBuffer[address];
  EepromStart_NES();              // START
  if (mapper == 159) {            // 24C01
    EepromSetAddress01(address);  // 24C01 [Little Endian]
    EepromWriteMode_NES();
    EepromWriteData01();  // 24C01 [Little Endian]
  } else {                // 24C02
    EepromDevice_NES();   // DEVICE [1010] + ADDR [A2-A0]
    EepromWriteMode_NES();
    EepromSetAddress02(address);
    EepromWriteData02();
  }
  EepromStop_NES();  // STOP
}

#if defined(ENABLE_FLASH)
/******************************************
   NESmaker Flash Cart [SST 39SF40]
 *****************************************/
void NESmaker_Cmd(byte cmd) {
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, cmd);
}

// SST 39SF040 Software ID
void NESmaker_ID() {   // Read Flash ID
  NESmaker_Cmd(0xFF);  // Reset
  NESmaker_Cmd(0x90);  // Software ID Entry
  flashid = read_prg_byte(0x8000) << 8;
  flashid |= read_prg_byte(0x8001);
  sprintf(flashid_str, "%04X", flashid);
  NESmaker_Cmd(0xF0);     // Software ID Exit
  if (flashid == 0xBFB7)  // SST 39SF040
    flashfound = 1;
}

void NESmaker_SectorErase(uint8_t bank, word address) {
  NESmaker_Cmd(0x80);
  write_prg_byte(0xC000, 0x01);
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xC000, 0x00);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0xC000, bank);   // $00-$1F
  write_prg_byte(address, 0x30);  // Sector Erase ($8000/$9000/$A000/$B000)
}

void NESmaker_ByteProgram(uint8_t bank, word address, uint8_t data) {
  NESmaker_Cmd(0xA0);
  write_prg_byte(0xC000, bank);   // $00-$1F
  write_prg_byte(address, data);  // $8000-$BFFF
}

// SST 39SF040 Chip Erase [NOT IMPLEMENTED]
void NESmaker_ChipErase() {  // Typical 70ms
  NESmaker_Cmd(0x80);
  NESmaker_Cmd(0x10);  // Chip Erase
}

void writeFLASH() {
  display_Clear();
  if (!flashfound) {
    rgbLed(red_color);
    println_Msg(F("FLASH NOT DETECTED"));
    display_Update();
  } else {
    print_Msg(F("Flash ID: "));
    println_Msg(flashid_str);
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("NESmaker Flash Found"));
    println_Msg(FS(FSTRING_EMPTY));
    display_Update();
    delay(100);

    fileBrowser(F("Select FLASH File"));
    word base = 0x8000;

    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);

    rgbLed(red_color);
    display_Clear();
    println_Msg(F("Writing File: "));
    println_Msg(filePath);
    println_Msg(fileName);
    display_Update();

    uint8_t bytecheck;
    uint16_t banks;

    //open file on sd card
    if (myFile.open(filePath, O_READ)) {
      banks = int_pow(2, prgsize);                                    // 256K/512K
      for (size_t i = 0; i < banks; i++) {                            // 16K Banks
        for (size_t sector = 0; sector < 0x4000; sector += 0x1000) {  // 4K Sectors ($8000/$9000/$A000/$B000)
          // Sector Erase
          NESmaker_SectorErase(i, base + sector);
          delay(18);                         // Typical 18ms
          for (uint8_t j = 0; j < 2; j++) {  // Confirm erase twice
            do {
              bytecheck = read_prg_byte(base + sector);
              delay(18);
            } while (bytecheck != 0xFF);
          }
          // Program Byte
          for (size_t addr = 0x0; addr < 0x1000; addr += 512) {
            myFile.read(sdBuffer, 512);
            for (size_t x = 0; x < 512; x++) {
              word location = base + sector + addr + x;
              NESmaker_ByteProgram(i, location, sdBuffer[x]);
              delayMicroseconds(14);             // Typical 14us
              for (uint8_t k = 0; k < 2; k++) {  // Confirm write twice
                do {
                  bytecheck = read_prg_byte(location);
                  delayMicroseconds(14);
                } while (bytecheck != sdBuffer[x]);
              }
            }
          }
        }

#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
        display.print(F("*"));
        display.updateDisplay();
#else
        Serial.print(F("*"));
        if ((i != 0) && ((i + 1) % 16 == 0))
          Serial.println(FS(FSTRING_EMPTY));
#endif
      }
      myFile.close();
      rgbLed(green_color);

      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("FLASH FILE WRITTEN!"));
      display_Update();
    } else {
      rgbLed(red_color);
      println_Msg(F("SD ERROR"));
      display_Update();
    }
  }
  display_Clear();
  rgbLed(black_color);
  sd.chdir();          // root
  filePath[0] = '\0';  // Reset filePath
}


/******************************************
   A29040B Flash Cart [A29040B]
 *****************************************/

// A29040B Software ID
void A29040B_ID() {  // Read Flash ID
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0x9555, 0x90);

  flashid = read_prg_byte(0x8000) << 8;
  flashid |= read_prg_byte(0x8001);
  sprintf(flashid_str, "%04X", flashid);
  if (flashid == 0x3786)  // A29040B
    flashfound = 1;

  A29040B_PRG_ResetFlash();
}

void A29040B_PRG_ResetFlash() {  // Reset Flash
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0x9555, 0xF0);  // Reset
  delayMicroseconds(14);         // Typical 14us
}


void A29040B_PRG_Write(uint16_t address, uint8_t data) {
  write_prg_byte(0x9555, 0xAA);
  write_prg_byte(0xAAAA, 0x55);
  write_prg_byte(0x9555, 0xA0);
  write_prg_byte(address, data);  // $8000-$BFFF
  delayMicroseconds(20);          // Typical 14us
}

void A29040B_PRG_SectorErase(uint16_t sec) {
  if (flashfound) {
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0x9555, 0x80);  //->setup
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(sec, 0x30);  //->erase
    delay(1000);                // WAIT MORE
  } else {
    println_Msg(F("FLASH NOT DETECTED OR SECTOR PROTECTED"));
  }
}

void A29040B_PRG_ChipErase() {
  if (flashfound) {
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0x9555, 0x80);  //->setup
    write_prg_byte(0x9555, 0xAA);
    write_prg_byte(0xAAAA, 0x55);
    write_prg_byte(0x9555, 0x10);  //->erase
    delay(8000);                   // WAIT MORE
  } else {
    println_Msg(F("FLASH NOT DETECTED OR SECTOR PROTECTED"));
  }
}

// CHR ================================================

void A29040B_CHR_ResetFlash() {  // Reset Flash
  write_chr_byte(0x0555, 0xAA);  // Original address for CHR
  write_chr_byte(0x02AA, 0x55);  // Original address for CHR
  write_chr_byte(0x0555, 0xF0);  // Reset command with original address
  delayMicroseconds(14);         // Typical 14us
}

void A29040B_CHR_Write(uint16_t address, uint8_t data) {
  write_chr_byte(0x0555, 0xAA);   // Original address for CHR
  write_chr_byte(0x02AA, 0x55);   // Original address for CHR
  write_chr_byte(0x0555, 0xA0);   // Program command with original address
  write_chr_byte(address, data);  // CHR address range (0x0000 - 0x1FFF)
  delayMicroseconds(20);          // Typical 14us
}


void A29040B_CHR_SectorErase(uint16_t sec) {
  if (flashfound) {
    write_chr_byte(0x0555, 0xAA);  // Original address for CHR
    write_chr_byte(0x02AA, 0x55);  // Original address for CHR
    write_chr_byte(0x0555, 0x80);  // Erase Setup with original address
    write_chr_byte(0x0555, 0xAA);  // Original address for CHR
    write_chr_byte(0x02AA, 0x55);  // Original address for CHR
    write_chr_byte(sec, 0x30);     // Sector Erase Command with sector address
    delay(1000);                   // WAIT MORE
  } else {
    println_Msg(F("FLASH NOT DETECTED OR SECTOR PROTECTED"));
  }
}

void A29040B_CHR_ChipErase() {
  if (flashfound) {
    write_chr_byte(0x0555, 0xAA);  // Original address for CHR
    write_chr_byte(0x02AA, 0x55);  // Original address for CHR
    write_chr_byte(0x0555, 0x80);  // Erase Setup with original address
    write_chr_byte(0x0555, 0xAA);  // Original address for CHR
    write_chr_byte(0x02AA, 0x55);  // Original address for CHR
    write_chr_byte(0x0555, 0x10);  // Chip Erase Command with original address
    delay(8000);                   // WAIT MORE
  } else {
    println_Msg(F("FLASH NOT DETECTED OR SECTOR PROTECTED"));
  }
}
#define A29040B_TITLE "FLASH A29040B MAPPER 0"
void A29040B_writeFLASH() {

  display_Clear();
  A29040B_ID();

  char data_str[10];
  uint32_t prgSize = 0;
  uint32_t chrSize = 0;

  if (!flashfound) {
    rgbLed(red_color);
    println_Msg(F(A29040B_TITLE));
    println_Msg(FS(FSTRING_EMPTY));
    print_Msg(F("Flash ID: "));
    println_Msg(flashid_str);
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("FLASH NOT FOUND"));
    display_Update();
    wait();
  } else {
    println_Msg(F(A29040B_TITLE));
    println_Msg(FS(FSTRING_EMPTY));
    print_Msg(F("Flash ID: "));
    println_Msg(flashid_str);
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(F("Flash Found"));
    println_Msg(FS(FSTRING_EMPTY));
    display_Update();
    delay(3000);
    fileBrowser(F("Select FLASH File"));
    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);

    if (myFile.open(filePath, O_READ)) {
      // Step 1: Read the header and extract PRG and CHR sizes
      uint8_t header[16];
      myFile.read(header, 16);  // Read the 16-byte header
      uint32_t prgAddress = 0x8000;

      prgSize = (uint32_t)header[4] * 16384;  // PRG size in bytes (header[4] gives size in 16 KB units)
      chrSize = (uint32_t)header[5] * 8192;   // CHR size in bytes (header[5] gives size in 8 KB units)

      // Output the sizes for verification
      display_Clear();
      println_Msg(F(A29040B_TITLE));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("PRG Size:"));
      sprintf(data_str, "%lu", prgSize);
      println_Msg(data_str);

      println_Msg(F("CHR Size:"));
      sprintf(data_str, "%lu", chrSize);
      println_Msg(data_str);
      display_Update();
      delay(3000);

      // Step 2: Erase the entire PRG space
      rgbLed(red_color);
      display_Clear();
      println_Msg(F(A29040B_TITLE));
      println_Msg(FS(FSTRING_EMPTY));
      A29040B_PRG_ResetFlash();
      println_Msg(F("ERASING PRG..."));
      display_Update();
      A29040B_PRG_ChipErase();


      uint8_t readByte = read_prg_byte(prgAddress);
      if (readByte != 0xFF) {
        println_Msg(F("Erase Error!"));
      } else {
        println_Msg(F("Erase OK!"));
      }

      display_Update();

      // Verify that the first byte has been erased
      uint8_t erase_check = read_prg_byte(0x8000);
      if (erase_check != 0xFF) {
        println_Msg(F("SECTOR NOT ERASED"));
        sprintf(data_str, "%02X", erase_check);
        println_Msg(data_str);
        return;
      }

      delay(18);  // Adjust delay as needed

      rgbLed(red_color);
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("Writing PRG Data..."));
      display_Update();

      A29040B_PRG_ResetFlash();

      // Step 3: Write PRG data
      uint32_t bytesProcessed = 0;
      uint8_t buffer[512];
      myFile.seek(16);  // Skip header to start of PRG data

      while (bytesProcessed < prgSize) {
        int bytesRead = myFile.read(buffer, sizeof(buffer));
        if (bytesRead <= 0) break;

        for (int i = 0; i < bytesRead; i++) {
          A29040B_PRG_Write(prgAddress++, buffer[i]);
          delayMicroseconds(14);  // Typical 14us

          uint8_t readByte = read_prg_byte(prgAddress - 1);
          delayMicroseconds(14);  // Typical 14us
          if (readByte != buffer[i]) {
            println_Msg(F("Write Error!"));
            sprintf(data_str, "%02X", readByte);
            println_Msg(data_str);
            myFile.close();
            break;
          }
        }
        bytesProcessed += bytesRead;
      }

      // Step 4: Erase and Write CHR data
      A29040B_CHR_ResetFlash();
      display_Clear();
      println_Msg(F(A29040B_TITLE));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("ERASING CHR..."));
      display_Update();
      A29040B_CHR_ChipErase();
      delay(20);
      display_Clear();

      uint32_t chrAddress = 0x0000;
      bytesProcessed = 0;
      myFile.seek(16 + prgSize);  // Seek to the start of CHR data

      readByte = read_chr_byte(chrAddress);
      if (readByte != 0xFF) {
        println_Msg(F("Erase Error!"));
      } else {
        println_Msg(F("Erase OK!"));
      }
      display_Update();

      println_Msg(F("Writing CHR Data..."));
      display_Update();


      while (bytesProcessed < chrSize) {
        int bytesRead = myFile.read(buffer, sizeof(buffer));
        if (bytesRead <= 0) break;

        for (int i = 0; i < bytesRead; i++) {
          A29040B_CHR_Write(chrAddress++, buffer[i]);
          delayMicroseconds(14);  // Typical 14us

          uint8_t readByte = read_chr_byte(chrAddress - 1);
          delayMicroseconds(14);  // Typical 14us
          if (readByte != buffer[i]) {
            println_Msg(F("Write Error!"));
            sprintf(data_str, "%02X", readByte);
            println_Msg(data_str);
            myFile.close();
            break;
          }
        }
        bytesProcessed += bytesRead;
      }
      delay(3000);
      myFile.close();
      rgbLed(green_color);
      display_Clear();
      println_Msg(F(A29040B_TITLE));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(F("FLASH FILE WRITTEN!"));
      display_Update();
    } else {
      rgbLed(red_color);
      println_Msg(F("SD ERROR"));
      display_Update();
    }

    display_Update();
  }
}
#endif

// avoid warnings
#undef MODE_READ
#undef MODE_WRITE

#endif
//******************************************
// End of File
//******************************************