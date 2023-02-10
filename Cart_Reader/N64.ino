//******************************************
// NINTENDO 64 MODULE
//******************************************
#ifdef enable_N64

/******************************************
  Defines
 *****************************************/
// These two macros toggle the eepDataPin/ControllerDataPin between input and output
// External 1K pull-up resistor from eepDataPin to VCC required
// 0x10 = 00010000 -> Port H Pin 4
#define N64_HIGH DDRH &= ~0x10
#define N64_LOW DDRH |= 0x10
// Read the current state(0/1) of the eepDataPin
#define N64_QUERY (PINH & 0x10)

/******************************************
   Variables
 *****************************************/
// Received N64 Eeprom data bits, 1 page
int eepPages;

// N64 Controller
struct {
  char stick_x;
  char stick_y;
} N64_status;
//stings that hold the buttons
String button = "N/A";
String lastbutton = "N/A";

// Rom base address
unsigned long romBase = 0x10000000;

// Flashram type
byte flashramType = 1;
boolean MN63F81MPN = false;

//ControllerTest
bool quit = 1;

#ifdef savesummarytotxt
String CRC1 = "";
String CRC2 = "";
#endif

static const char N64_EEP_FILENAME_FMT[] PROGMEM = "%s.eep";
static const char N64_SAVE_DIRNAME_FMT[] PROGMEM = "N64/SAVE/%s/%d";

/******************************************
  Menu
*****************************************/
// N64 start menu
static const char n64MenuItem1[] PROGMEM = "Game Cartridge";
static const char n64MenuItem2[] PROGMEM = "Controller";
static const char n64MenuItem3[] PROGMEM = "Flash Repro";
static const char n64MenuItem4[] PROGMEM = "Flash Gameshark";
//static const char n64MenuItem5[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsN64[] PROGMEM = { n64MenuItem1, n64MenuItem2, n64MenuItem3, n64MenuItem4, string_reset2 };

// N64 controller menu items
static const char N64ContMenuItem1[] PROGMEM = "Test Controller";
static const char N64ContMenuItem2[] PROGMEM = "Read ControllerPak";
static const char N64ContMenuItem3[] PROGMEM = "Write ControllerPak";
//static const char N64ContMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsN64Controller[] PROGMEM = { N64ContMenuItem1, N64ContMenuItem2, N64ContMenuItem3, string_reset2 };

// N64 cart menu items
static const char N64CartMenuItem1[] PROGMEM = "Read ROM";
static const char N64CartMenuItem2[] PROGMEM = "Read Save";
static const char N64CartMenuItem3[] PROGMEM = "Write Save";
static const char N64CartMenuItem4[] PROGMEM = "Force Savetype";
//static const char N64CartMenuItem5[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsN64Cart[] PROGMEM = { N64CartMenuItem1, N64CartMenuItem2, N64CartMenuItem3, N64CartMenuItem4, string_reset2 };

// N64 CRC32 error menu items
static const char N64CRCMenuItem1[] PROGMEM = "No";
static const char N64CRCMenuItem2[] PROGMEM = "Yes and keep old";
static const char N64CRCMenuItem3[] PROGMEM = "Yes and delete old";
//static const char N64CRCMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsN64CRC[] PROGMEM = { N64CRCMenuItem1, N64CRCMenuItem2, N64CRCMenuItem3, string_reset2 };

// Rom menu
static const char N64RomItem1[] PROGMEM = "4 MB";
static const char N64RomItem2[] PROGMEM = "8 MB";
static const char N64RomItem3[] PROGMEM = "12 MB";
static const char N64RomItem4[] PROGMEM = "16 MB";
static const char N64RomItem5[] PROGMEM = "32 MB";
static const char N64RomItem6[] PROGMEM = "64 MB";
static const char* const romOptionsN64[] PROGMEM = { N64RomItem1, N64RomItem2, N64RomItem3, N64RomItem4, N64RomItem5, N64RomItem6 };

// Save menu
static const char N64SaveItem1[] PROGMEM = "None";
static const char N64SaveItem2[] PROGMEM = "4K EEPROM";
static const char N64SaveItem3[] PROGMEM = "16K EEPROM";
static const char N64SaveItem4[] PROGMEM = "SRAM";
static const char N64SaveItem5[] PROGMEM = "FLASH";
static const char* const saveOptionsN64[] PROGMEM = { N64SaveItem1, N64SaveItem2, N64SaveItem3, N64SaveItem4, N64SaveItem5 };

// Repro write buffer menu
static const char N64BufferItem1[] PROGMEM = "No buffer";
static const char N64BufferItem2[] PROGMEM = "32 Byte";
static const char N64BufferItem3[] PROGMEM = "64 Byte";
static const char N64BufferItem4[] PROGMEM = "128 Byte";
static const char* const bufferOptionsN64[] PROGMEM = { N64BufferItem1, N64BufferItem2, N64BufferItem3, N64BufferItem4 };

// Repro sector size menu
static const char N64SectorItem1[] PROGMEM = "8 KB";
static const char N64SectorItem2[] PROGMEM = "32 KB";
static const char N64SectorItem3[] PROGMEM = "64 KB";
static const char N64SectorItem4[] PROGMEM = "128 KB";
static const char* const sectorOptionsN64[] PROGMEM = { N64SectorItem1, N64SectorItem2, N64SectorItem3, N64SectorItem4 };

// N64 start menu
void n64Menu() {
  vselect(true);
  // create menu with title and 5 options to choose from
  unsigned char n64Dev;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsN64, 5);
  n64Dev = question_box(F("Select N64 device"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (n64Dev) {
    case 0:
      display_Clear();
      display_Update();
      setup_N64_Cart();
      printCartInfo_N64();
      mode = mode_N64_Cart;
      break;

    case 1:
      display_Clear();
      display_Update();
      setup_N64_Controller();
      mode = mode_N64_Controller;
      break;

    case 2:
      display_Clear();
      display_Update();
      setup_N64_Cart();
      flashRepro_N64();
      printCartInfo_N64();
      mode = mode_N64_Cart;
      break;

    case 3:
      display_Clear();
      display_Update();
      setup_N64_Cart();
      flashGameshark_N64();
      printCartInfo_N64();
      mode = mode_N64_Cart;
      break;

    case 4:
      resetArduino();
      break;
  }
}

// N64 Controller Menu
void n64ControllerMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsN64Controller, 4);
  mainMenu = question_box(F("N64 Controller"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      resetController();
      display_Clear();
      display_Update();
#if (defined(enable_OLED) || defined(enable_LCD))
      controllerTest_Display();
#elif defined(enable_serial)
      controllerTest_Serial();
#endif
      quit = 1;
      break;

    case 1:
      resetController();
      checkController();
      display_Clear();
      display_Update();
      readMPK();
      verifyCRC();
      validateMPK();
      println_Msg(F(""));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 2:
      resetController();
      checkController();
      display_Clear();
      display_Update();
      // Change to root
      filePath[0] = '\0';
      sd.chdir("/");
      // Launch file browser
      fileBrowser(F("Select mpk file"));
      display_Clear();
      display_Update();
      writeMPK();
      delay(500);
      verifyMPK();
      println_Msg(F(""));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 3:
      resetArduino();
      break;
  }
}

// N64 Cartridge Menu
void n64CartMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsN64Cart, 5);
  mainMenu = question_box(F("N64 Cart Reader"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      sd.chdir("/");
#ifndef fastcrc
      // Dumping ROM slow
      readRom_N64();
      sd.chdir("/");
      compareCRC("n64.txt", 0, 1, 0);
#else
      // Dumping ROM fast
      compareCRC("n64.txt", readRom_N64(), 1, 0);
#endif

#ifdef global_log
      save_log();
#endif

      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 1:
      sd.chdir("/");
      display_Clear();

      if (saveType == 1) {
        println_Msg(F("Reading SRAM..."));
        display_Update();
        readSram(32768, 1);
      } else if (saveType == 4) {
        getFramType();
        println_Msg(F("Reading FLASH..."));
        display_Update();
        readFram(flashramType);
      } else if ((saveType == 5) || (saveType == 6)) {
        println_Msg(F("Reading EEPROM..."));
        display_Update();
        readEeprom();
      } else {
        print_Error(F("Savetype Error"));
      }
      println_Msg(F(""));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 2:
      filePath[0] = '\0';
      sd.chdir("/");
      if (saveType == 1) {
        // Launch file browser
        fileBrowser(F("Select sra file"));
        display_Clear();

        writeSram(32768);
        writeErrors = verifySram(32768, 1);
        if (writeErrors == 0) {
          println_Msg(F("SRAM verified OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(writeErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else if (saveType == 4) {
        // Launch file browser
        fileBrowser(F("Select fla file"));
        display_Clear();
        getFramType();
        writeFram(flashramType);
        print_STR(verifying_STR, 0);
        display_Update();
        writeErrors = verifyFram(flashramType);
        if (writeErrors == 0) {
          println_Msg(F("OK"));
          display_Update();
        } else {
          println_Msg("");
          print_STR(error_STR, 0);
          print_Msg(writeErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else if ((saveType == 5) || (saveType == 6)) {
        // Launch file browser
        fileBrowser(F("Select eep file"));
        display_Clear();

        writeEeprom();
        writeErrors = verifyEeprom();

        if (writeErrors == 0) {
          println_Msg(F("EEPROM verified OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(writeErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else {
        display_Clear();
        print_Error(F("Save Type Error"));
      }
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 3:
      // create submenu with title and 6 options to choose from
      unsigned char N64SaveMenu;
      // Copy menuOptions out of progmem
      convertPgm(saveOptionsN64, 5);
      N64SaveMenu = question_box(F("Select save type"), menuOptions, 5, 0);

      // wait for user choice to come back from the question box menu
      switch (N64SaveMenu) {
        case 0:
          // None
          saveType = 0;
          break;

        case 1:
          // 4K EEPROM
          saveType = 5;
          eepPages = 64;
          break;

        case 2:
          // 16K EEPROM
          saveType = 6;
          eepPages = 256;
          break;

        case 3:
          // SRAM
          saveType = 1;
          break;

        case 4:
          // FLASHRAM
          saveType = 4;
          break;
      }
      break;

    case 4:
      resetArduino();
      break;
  }
}

/******************************************
   Setup
 *****************************************/
void setup_N64_Controller() {
  // Output a low signal
  PORTH &= ~(1 << 4);
  // Set Controller Data Pin(PH4) to Input
  DDRH &= ~(1 << 4);
}

void setup_N64_Cart() {
  // Set Address Pins to Output and set them low
  //A0-A7
  DDRF = 0xFF;
  PORTF = 0x00;
  //A8-A15
  DDRK = 0xFF;
  PORTK = 0x00;

  // Set Control Pins to Output RESET(PH0) WR(PH5) RD(PH6) aleL(PC0) aleH(PC1)
  DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
  DDRC |= (1 << 0) | (1 << 1);
  // Pull RESET(PH0) low until we are ready
  PORTH &= ~(1 << 0);
  // Output a high signal on WR(PH5) RD(PH6), pins are active low therefore everything is disabled now
  PORTH |= (1 << 5) | (1 << 6);
  // Pull aleL(PC0) low and aleH(PC1) high
  PORTC &= ~(1 << 0);
  PORTC |= (1 << 1);

#ifdef clockgen_installed
  // Adafruit Clock Generator

  initializeClockOffset();

  if (!i2c_found) {
    display_Clear();
    print_FatalError(F("Clock Generator not found"));
  }

  // Set Eeprom clock to 2Mhz
  clockgen.set_freq(200000000ULL, SI5351_CLK1);

  // Start outputting Eeprom clock
  clockgen.output_enable(SI5351_CLK1, 1);  // Eeprom clock

#else
  // Set Eeprom Clock Pin(PH1) to Output
  DDRH |= (1 << 1);
  // Output a high signal
  PORTH |= (1 << 1);
#endif

  // Set Eeprom Data Pin(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set sram base address
  sramBase = 0x08000000;

#ifdef clockgen_installed
  // Wait for clock generator
  clockgen.update_status();
#endif

  // Wait until all is stable
  delay(300);

  // Pull RESET(PH0) high to start eeprom
  PORTH |= (1 << 0);
}

/******************************************
   Low level functions
 *****************************************/
// Switch Cartridge address/data pins to write
void adOut_N64() {
  //A0-A7
  DDRF = 0xFF;
  PORTF = 0x00;
  //A8-A15
  DDRK = 0xFF;
  PORTK = 0x00;
}

// Switch Cartridge address/data pins to read
void adIn_N64() {
  //A0-A7
  DDRF = 0x00;
  //A8-A15
  DDRK = 0x00;
  //Enable internal pull-up resistors
  //PORTF = 0xFF;
  //PORTK = 0xFF;
}

// Set Cartridge address
void setAddress_N64(unsigned long myAddress) {
  // Set address pins to output
  adOut_N64();

  // Split address into two words
  word myAdrLowOut = myAddress & 0xFFFF;
  word myAdrHighOut = myAddress >> 16;

  // Switch WR(PH5) RD(PH6) ale_L(PC0) ale_H(PC1) to high (since the pins are active low)
  PORTH |= (1 << 5) | (1 << 6);
  PORTC |= (1 << 1);
  __asm__("nop\n\t");
  PORTC |= (1 << 0);

  // Output high part to address pins
  PORTF = myAdrHighOut & 0xFF;
  PORTK = (myAdrHighOut >> 8) & 0xFF;

  // Leave ale_H high for additional 62.5ns
  __asm__("nop\n\t");

  // Pull ale_H(PC1) low
  PORTC &= ~(1 << 1);

  // Output low part to address pins
  PORTF = myAdrLowOut & 0xFF;
  PORTK = (myAdrLowOut >> 8) & 0xFF;

  // Leave ale_L high for ~125ns
  __asm__("nop\n\t"
          "nop\n\t");

  // Pull ale_L(PC0) low
  PORTC &= ~(1 << 0);

  // Wait ~600ns just to be sure address is set
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Set data pins to input
  adIn_N64();
}

// Read one word out of the cartridge
word readWord_N64() {
  // Pull read(PH6) low
  PORTH &= ~(1 << 6);

  // Wait ~310ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Join bytes from PINF and PINK into a word
  word tempWord = ((PINK & 0xFF) << 8) | (PINF & 0xFF);

  // Pull read(PH6) high
  PORTH |= (1 << 6);

  // Wait 62.5ns
  __asm__("nop\n\t");
  return tempWord;
}

// Write one word to data pins of the cartridge
void writeWord_N64(word myWord) {
  // Set address pins to output
  adOut_N64();

  // Output word to AD0-AD15
  PORTF = myWord & 0xFF;
  PORTK = (myWord >> 8) & 0xFF;

  // Wait ~62.5ns
  __asm__("nop\n\t");

  // Pull write(PH5) low
  PORTH &= ~(1 << 5);

  // Wait ~310ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Pull write(PH5) high
  PORTH |= (1 << 5);

  // Wait ~125ns
  __asm__("nop\n\t"
          "nop\n\t");

  // Set data pins to input
  adIn_N64();
}

/******************************************
   N64 Controller CRC Functions
 *****************************************/
static word addrCRC(word address) {
  const char n64_address_crc_table[] = { 0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01 };
  const char* cur_xor = n64_address_crc_table;
  byte crc = 0;
  for (word mask = 0x0020; mask; mask <<= 1, cur_xor++) {
    if (address & mask) {
      crc ^= *cur_xor;
    }
  }
  return (address & 0xFFE0) | crc;
}

static uint8_t dataCRC(uint8_t* data) {
  uint8_t ret = 0;
  for (uint8_t i = 0; i <= 32; i++) {
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
      uint8_t tmp = ret & 0x80 ? 0x85 : 0;
      ret <<= 1;
      if (i < 32) {
        if (data[i] & mask) {
          ret |= 0x1;
        }
      }
      ret ^= tmp;
    }
  }
  return ret;
}

// Macro producing a delay loop waiting an number of cycles multiple of 3, with
// a range of 3 to 768 cycles (187.5ns to 48us). It takes 6 bytes to do so
// (3 instructions) making it the same size as the equivalent 3-cycles NOP
// delay. For shorter delays or non-multiple-of-3-cycle delays, add your own
// NOPs.
#define N64_DELAY_LOOP(cycle_count) \
  do { \
    byte i; \
    __asm__ __volatile__("\n" \
                         "\tldi %[i], %[loop_count]\n" \
                         ".delay_loop_%=:\n" \
                         "\tdec %[i]\n" \
                         "\tbrne .delay_loop_%=\n" \
                         : [i] "=r"(i) \
                         : [loop_count] "i"(cycle_count / 3) \
                         : "cc"); \
  } while (0)

/******************************************
   N64 Controller Protocol Functions
 *****************************************/
void sendJoyBus(const byte* buffer, char length) {
  // Implemented in assembly as there is very little wiggle room, timing-wise.
  // Overall structure:
  //   outer_loop:
  //     mask = 0x80
  //     cur_byte = *(buffer++)
  //   inner_loop:
  //     falling edge
  //     if (cur_byte & mask) {
  //       wait 1us starting at the falling edge
  //       rising edge
  //       wait 2us starting at the rising edge
  //     } else {
  //       wait 3us starting at the falling edge
  //       rising edge
  //     }
  //   inner_common_codepath:
  //     mask >>= 1
  //     if (mask == 0)
  //       goto outer_loop_trailer
  //     wait +1us from the rising edge
  //     goto inner_loop
  //   outer_loop_trailer:
  //     length -= 1
  //     if (length == 0)
  //       goto stop_bit
  //     wait +1us from the rising edge
  //     goto outer_loop
  //   stop_bit:
  //     wait +1us from the rising edge
  //     falling edge
  //     wait 1us from the falling edge
  //     rising edge

  byte mask, cur_byte, scratch;
  // Note on DDRH: retrieve the current DDRH value, and pre-compute the values
  // to write in order to drive the line high or low. This saves 3 cycles per
  // transition: sts (2 cycles) instead of lds, or/and, sts (2 + 1 + 2 cycles).
  // This means that no other code may run in parallel, but this function anyway
  // requires interrupts to be disabled in order to work in the expected amount
  // of time.
  const byte line_low = DDRH | 0x10;
  const byte line_high = line_low & 0xef;
  __asm__ __volatile__("\n"
                       ".outer_loop_%=:\n"
                       // mask = 0x80
                       "\tldi  %[mask], 0x80\n"  // 1
                       // load byte to send from memory
                       "\tld   %[cur_byte], Z+\n"  // 2
                       ".inner_loop_%=:\n"
                       // Falling edge
                       "\tsts  %[out_byte], %[line_low]\n"  // 2
                       // Test cur_byte & mask, without clobbering either
                       "\tmov  %[scratch], %[cur_byte]\n"  // 1
                       "\tand  %[scratch], %[mask]\n"      // 1
                       "\tbreq .bit_is_0_%=\n"             // bit is 1: 1, bit is 0: 2

                       // bit is a 1
                       // Stay low for 1us (16 cycles).
                       // Time before: 3 cycles (mov, and, breq-false).
                       // Time after: sts (2 cycles).
                       // So 11 to go, so 3 3-cycles iterations and 2 nop.
                       "\tldi  %[scratch], 3\n"  // 1
                       ".delay_1_low_%=:\n"
                       "\tdec  %[scratch]\n"       // 1
                       "\tbrne .delay_1_low_%=\n"  // exit: 1, loop: 2
                       "\tnop\n"                   // 1
                       "\tnop\n"                   // 1
                       // Rising edge
                       "\tsts  %[out_byte], %[line_high]\n"  // 2
                       // Wait for 2us (32 cycles) to sync with the bot_is_0 codepath.
                       // Time before: 0 cycles.
                       // Time after: 2 cycles (rjmp).
                       // So 30 to go, so 10 3-cycles iterations and 0 nop.
                       "\tldi  %[scratch], 10\n"  // 1
                       ".delay_1_high_%=:\n"
                       "\tdec  %[scratch]\n"             // 1
                       "\tbrne .delay_1_high_%=\n"       // exit: 1, loop: 2
                       "\trjmp .inner_common_path_%=\n"  // 2

                       ".bit_is_0_%=:\n"
                       // bit is a 0
                       // Stay high for 3us (48 cycles).
                       // Time before: 4 cycles (mov, and, breq-true).
                       // Time after: 2 cycles (sts).
                       // So 42 to go, so 14 3-cycles iterations, and 0 nop.
                       "\tldi  %[scratch], 14\n"  // 1
                       ".delay_0_low_%=:\n"
                       "\tdec  %[scratch]\n"       // 1
                       "\tbrne .delay_0_low_%=\n"  // exit: 1, loop: 2
                       // Rising edge
                       "\tsts  %[out_byte], %[line_high]\n"  // 2

                       // codepath common to both possible values
                       ".inner_common_path_%=:\n"
                       "\tnop\n"                          // 1
                       "\tlsr  %[mask]\n"                 // 1
                       "\tbreq .outer_loop_trailer_%=\n"  // mask!=0: 1, mask==0: 2
                       // Stay high for 1us (16 cycles).
                       // Time before: 3 cycles (nop, lsr, breq-false).
                       // Time after: 4 cycles (rjmp, sts)
                       // So 9 to go, so 3 3-cycles iterations and 0 nop.
                       "\tldi  %[scratch], 3\n"  // 1
                       ".delay_common_high_%=:\n"
                       "\tdec  %[scratch]\n"             // 1
                       "\tbrne .delay_common_high_%=\n"  // exit: 1, loop: 2
                       "\trjmp .inner_loop_%=\n"         // 2

                       ".outer_loop_trailer_%=:\n"
                       "\tdec %[length]\n"      // 1
                       "\tbreq .stop_bit_%=\n"  // length!=0: 1, length==0: 2
                       // Stay high for 1us (16 cycles).
                       // Time before: 6 cycles (lsr, nop, breq-true, dec, breq-false).
                       // Time after: 7 cycles (rjmp, ldi, ld, sts).
                       // So 3 to go, so 3 nop (for simplicity).
                       "\tnop\n"                  // 1
                       "\tnop\n"                  // 1
                       "\tnop\n"                  // 1
                       "\trjmp .outer_loop_%=\n"  // 2
                       // Done sending data, send a stop bit.
                       ".stop_bit_%=:\n"
                       // Stay high for 1us (16 cycles).
                       // Time before: 7 cycles (lsr, nop, breq-true, dec, breq-true).
                       // Time after: 2 cycles (sts).
                       // So 7 to go, so 2 3-cycles iterations and 1 nop.
                       "\tldi  %[scratch], 2\n"  // 1
                       ".delay_stop_high_%=:\n"
                       "\tdec  %[scratch]\n"           // 1
                       "\tbrne .delay_stop_high_%=\n"  // exit: 1, loop: 2
                       "\tnop\n"
                       "\tsts  %[out_byte], %[line_low]\n"  // 2
                       // Stay low for 1us (16 cycles).
                       // Time before: 0 cycles.
                       // Time after: 2 cycles (sts).
                       // So 14 to go, so 4 3-cycles iterations and 2 nop.
                       "\tldi  %[scratch], 5\n"  // 1
                       ".delay_stop_low_%=:\n"
                       "\tdec  %[scratch]\n"          // 1
                       "\tbrne .delay_stop_low_%=\n"  // exit: 1, loop: 2
                       "\tnop\n"
                       "\tnop\n"
                       "\tsts  %[out_byte], %[line_high]\n"  // 2
                       // Notes on arguments:
                       // - mask and scratch are used wth "ldi", which can only work on registers
                       //   16 to 31, so tag these with "a" rather than the generic "r"
                       // - mark all output-only arguments as early-clobber ("&"), as input
                       //   registers are used throughout all iterations and both sets must be
                       //   strictly distinct
                       // - tag buffer with "z", to use the "ld r?, Z+" instruction (load from
                       //   16bits RAM address and postincrement, in 2 cycles).
                       //   XXX: any pointer register pair would do, but mapping to Z explicitly
                       //   because I cannot find a way to get one of "X", "Y" or "Z" to appear
                       //   when expanding "%[buffer]", causing the assembler to reject the
                       //   instruction. Pick Z as it is the only call-used such register,
                       //   avoiding the need to preserve any value a caller may have set it to.
                       : [buffer] "+z"(buffer),
                         [length] "+r"(length),
                         [cur_byte] "=&r"(cur_byte),
                         [mask] "=&a"(mask),
                         [scratch] "=&a"(scratch)
                       : [line_low] "r"(line_low),
                         [line_high] "r"(line_high),
                         [out_byte] "i"(&DDRH)
                       : "cc", "memory");
}

word recvJoyBus(byte* output, byte byte_count) {
  // listen for expected byte_count bytes of data back from the controller
  // return the number of bytes not (fully) received if the delay for a signal
  // edge takes too long.

  // Implemented in assembly as there is very little wiggle room, timing-wise.
  // Overall structure:
  //     mask = 0x80
  //     cur_byte = 0
  //   read_loop:
  //     wait for falling edge
  //     wait for a bit more than 1us
  //     if input:
  //       cur_byte |= mask
  //     mask >>= 1
  //     if (mask == 0)
  //       if (--byte_count == 0)
  //         goto read_end
  //       append cur_byte to output
  //       mask = 0x80
  //       cur_byte = 0
  //     wait for data high
  //     goto read_loop
  //   read_end:
  //     return byte_count

  byte mask, cur_byte, timeout, scratch;
  __asm__ __volatile__("\n"
                       "\tldi  %[mask], 0x80\n"
                       "\tclr  %[cur_byte]\n"
                       ".read_loop_%=:\n"
                       // Wait for input to be low. Time out if it takes more than ~27us (~7 bits
                       // worth of time) for it to go low.
                       // Takes 5 cycles to exit on input-low iteration (lds, sbrs-false, rjmp).
                       // Takes 7 cycles to loop on input-high iteration (lds, sbrs-true, dec,
                       //  brne-true).
                       "\tldi  %[timeout], 0x3f\n"  // 1
                       ".read_wait_falling_edge_%=:\n"
                       "\tlds  %[scratch], %[in_byte]\n"      // 2
                       "\tsbrs %[scratch], %[in_bit]\n"       // low: 1, high: 2
                       "\trjmp .read_input_low_%=\n"          // 2
                       "\tdec  %[timeout]\n"                  // 1
                       "\tbrne .read_wait_falling_edge_%=\n"  // timeout==0: 1, timeout!=0: 2
                       "\trjmp .read_end_%=\n"                // 2

                       ".read_input_low_%=:\n"
                       // Wait for 1500 us (24 cycles) before reading input.
                       // As it takes from 5 to 7 cycles for the prevous loop to exit,
                       // this means this loop exits from 1812.5us to 1937.5us after the falling
                       // edge, so at least 812.5us after a 1-bit rising edge, and at least
                       // 1062.5us before a 0-bit rising edge.
                       // This also leaves us with up to 2062.5us (33 cycles) to update cur_byte,
                       // possibly moving on to the next byte, waiting for a high input, and
                       // waiting for the next falling edge.
                       // Time taken until waiting for input high for non-last byte:
                       // - shift to current byte:
                       //   - 1: 4 cycles (lds, sbrc-false, or)
                       //   - 0: 4 cycles (lds, sbrc-true)
                       // - byte done: 8 cycles (lsr, brne-false, st, dec, brne-false, ldi, clr)
                       // - byte not done: 3 cycles (lsr, brne-true)
                       // Total: 7 to 12 cycles, so there are at least 21 cycles left until the
                       // next bit.
                       "\tldi  %[timeout], 8\n"  // 1
                       ".read_wait_low_%=:\n"
                       "\tdec  %[timeout]\n"         // 1
                       "\tbrne .read_wait_low_%=\n"  // timeout=0: 1, timeout!=0: 2

                       // Sample input
                       "\tlds  %[scratch], %[in_byte]\n"  // 2
                       // Add to cur_byte
                       "\tsbrc %[scratch], %[in_bit]\n"  // high: 1, low: 2
                       "\tor   %[cur_byte], %[mask]\n"   // 1
                       // Shift mask
                       "\tlsr  %[mask]\n"
                       "\tbrne .read_wait_input_high_init_%=\n"  // mask==0: 1, mask!=0: 2
                       // A wole byte was read, store in output
                       "\tst   Z+, %[cur_byte]\n"  // 2
                       // Decrement byte count
                       "\tdec  %[byte_count]\n"  // 1
                       // Are we done reading ?
                       "\tbreq .read_end_%=\n"  // byte_count!=0: 1, byte_count==0: 2
                       // No, prepare for reading another
                       "\tldi  %[mask], 0x80\n"
                       "\tclr  %[cur_byte]\n"

                       // Wait for rising edge
                       ".read_wait_input_high_init_%=:"
                       "\tldi  %[timeout], 0x3f\n"  // 1
                       ".read_wait_input_high_%=:\n"
                       "\tlds  %[scratch], %[in_byte]\n"    // 2
                       "\tsbrc %[scratch], %[in_bit]\n"     // high: 1, low: 2
                       "\trjmp .read_loop_%=\n"             // 2
                       "\tdec  %[timeout]\n"                // 1
                       "\tbrne .read_wait_input_high_%=\n"  // timeout==0: 1, timeout!=0: 2
                       "\trjmp .read_end_%=\n"              // 2
                       ".read_end_%=:\n"
                       : [output] "+z"(output),
                         [byte_count] "+r"(byte_count),
                         [mask] "=&a"(mask),
                         [cur_byte] "=&r"(cur_byte),
                         [timeout] "=&a"(timeout),
                         [scratch] "=&a"(scratch)
                       : [in_byte] "i"(&PINH),
                         [in_bit] "i"(4)
                       : "cc", "memory");
  return byte_count;
}

/******************************************
   N64 Controller Functions
 *****************************************/
void get_button() {
  // Command to send to the gamecube
  // The last bit is rumble, flip it to rumble
  const byte command[] = { 0x01 };
  byte response[4];

  // don't want interrupts getting in the way
  noInterrupts();
  sendJoyBus(command, sizeof(command));
  recvJoyBus(response, sizeof(response));
  // end of time sensitive code
  interrupts();

  // These are 8 bit values centered at 0x80 (128)
  N64_status.stick_x = response[2];
  N64_status.stick_y = response[3];

  // Buttons (A,B,Z,S,DU,DD,DL,DR,0,0,L,R,CU,CD,CL,CR)
  if (response[0] & 0x80)
    button = F("A");
  else if (response[0] & 0x40)
    button = F("B");
  else if (response[0] & 0x20)
    button = F("Z");
  else if (response[0] & 0x10)
    button = F("START");
  else if (response[0] & 0x08)
    button = F("D-Up");
  else if (response[0] & 0x04)
    button = F("D-Down");
  else if (response[0] & 0x02)
    button = F("D-Left");
  else if (response[0] & 0x01)
    button = F("D-Right");
  //else if (response[1] & 0x80)
  //else if (response[1] & 0x40)
  else if (response[1] & 0x20)
    button = F("L");
  else if (response[1] & 0x10)
    button = F("R");
  else if (response[1] & 0x08)
    button = F("C-Up");
  else if (response[1] & 0x04)
    button = F("C-Down");
  else if (response[1] & 0x02)
    button = F("C-Left");
  else if (response[1] & 0x01)
    button = F("C-Right");
  else {
    lastbutton = button;
    button = F("Press a button");
  }
}


/******************************************
  N64 Controller Test
 *****************************************/
#ifdef enable_serial
void controllerTest_Serial() {
  while (quit) {
    // Get Button and analog stick
    get_button();

    // Print Button
    String buttonc = String("Button: " + String(button) + "   ");
    Serial.print(buttonc);

    // Print Stick X Value
    String stickx = String("X: " + String(N64_status.stick_x, DEC) + "   ");
    Serial.print(stickx);

    // Print Stick Y Value
    String sticky = String(" Y: " + String(N64_status.stick_y, DEC) + "   ");
    Serial.println(sticky);

    if (button == "Press a button" && lastbutton == "Z") {
      // Quit
      Serial.println("");
      quit = 0;
    }
  }
}
#endif

#if (defined(enable_LCD) || defined(enable_OLED))
#define CENTER 64
// on which screens do we start
int startscreen = 1;
int test = 1;

void printSTR(String st, int x, int y) {
  char buf[st.length() + 1];

  if (x == CENTER) {
    x = 64 - (((st.length() - 5) / 2) * 4);
  }

  st.toCharArray(buf, st.length() + 1);
  display.drawStr(x, y, buf);
}

void nextscreen() {
  if (button == "Press a button" && lastbutton == "START") {
    // reset button
    lastbutton = "N/A";

    display.clearDisplay();
    if (startscreen != 4)
      startscreen = startscreen + 1;
    else {
      startscreen = 1;
      test = 1;
    }
  } else if (button == "Press a button" && lastbutton == "Z" && startscreen == 4) {
    // Quit
    quit = 0;
  }
}

void controllerTest_Display() {
  int mode = 0;

  //name of the current displayed result
  String anastick = "";

  // Graph
  int xax = 24;  // midpoint x
  int yax = 24;  // midpoint y

  // variables to display test data of different sticks
  int upx = 0;
  int upy = 0;
  int uprightx = 0;
  int uprighty = 0;
  int rightx = 0;
  int righty = 0;
  int downrightx = 0;
  int downrighty = 0;
  int downx = 0;
  int downy = 0;
  int downleftx = 0;
  int downlefty = 0;
  int leftx = 0;
  int lefty = 0;
  int upleftx = 0;
  int uplefty = 0;

  // variables to save test data
  int bupx = 0;
  int bupy = 0;
  int buprightx = 0;
  int buprighty = 0;
  int brightx = 0;
  int brighty = 0;
  int bdownrightx = 0;
  int bdownrighty = 0;
  int bdownx = 0;
  int bdowny = 0;
  int bdownleftx = 0;
  int bdownlefty = 0;
  int bleftx = 0;
  int blefty = 0;
  int bupleftx = 0;
  int buplefty = 0;
  int results = 0;
  int prevStickX = 0;

  String stickx;
  String sticky;
  String stickx_old;
  String sticky_old;
  String button_old;

  while (quit) {
    // Get Button and analog stick
    get_button();

    switch (startscreen) {
      case 1:
        {
          display.drawStr(32, 8, "Controller Test");
          display.drawLine(0, 10, 128, 10);

          // Delete old button value
          if (button_old != button) {
            display.setDrawColor(0);
            for (byte y = 13; y < 22; y++) {
              display.drawLine(0, y, 128, y);
            }
            display.setDrawColor(1);
          }
          // Print button
          printSTR("       " + button + "       ", CENTER, 20);
          // Save value
          button_old = button;

          // Update stick values
          stickx = String("X: " + String(N64_status.stick_x, DEC) + "   ");
          sticky = String("Y: " + String(N64_status.stick_y, DEC) + "   ");

          // Delete old stick values
          if ((stickx_old != stickx) || (sticky_old != sticky)) {
            display.setDrawColor(0);
            for (byte y = 31; y < 38; y++) {
              display.drawLine(0, y, 128, y);
            }
            display.setDrawColor(1);
          }

          // Print stick values
          printSTR(stickx, 36, 38);
          printSTR(sticky, 74, 38);
          // Save values
          stickx_old = stickx;
          sticky_old = sticky;

          printSTR("(Continue with START)", 16, 55);

          //Update LCD
          display.updateDisplay();

          // go to next screen
          nextscreen();
          break;
        }
      case 2:
        {
          display.drawStr(36, 8, "Range Test");
          display.drawLine(0, 9, 128, 9);

          if (mode == 0) {
            // Print Stick X Value
            String stickx = String("X:" + String(N64_status.stick_x, DEC) + "   ");
            printSTR(stickx, 22 + 54, 26);

            // Print Stick Y Value
            String sticky = String("Y:" + String(N64_status.stick_y, DEC) + "   ");
            printSTR(sticky, 22 + 54, 36);
          }

          // Draw Axis
          display.drawPixel(10 + xax, 12 + yax);
          display.drawPixel(10 + xax, 12 + yax - 80 / 4);
          display.drawPixel(10 + xax, 12 + yax + 80 / 4);
          display.drawPixel(10 + xax + 80 / 4, 12 + yax);
          display.drawPixel(10 + xax - 80 / 4, 12 + yax);

          // Draw corners
          display.drawPixel(10 + xax - 68 / 4, 12 + yax - 68 / 4);
          display.drawPixel(10 + xax + 68 / 4, 12 + yax + 68 / 4);
          display.drawPixel(10 + xax + 68 / 4, 12 + yax - 68 / 4);
          display.drawPixel(10 + xax - 68 / 4, 12 + yax + 68 / 4);

          //Draw Analog Stick
          if (mode == 1) {
            display.drawPixel(10 + xax + N64_status.stick_x / 4, 12 + yax - N64_status.stick_y / 4);
            //Update LCD
            display.updateDisplay();
          } else {
            display.drawCircle(10 + xax + N64_status.stick_x / 4, 12 + yax - N64_status.stick_y / 4, 2);
            //Update LCD
            display.updateDisplay();
            display_Clear_Slow();
          }

          // switch mode
          if (button == "Press a button" && lastbutton == "Z") {
            if (mode == 0) {
              mode = 1;
              display.clearDisplay();
            } else {
              mode = 0;
              display.clearDisplay();
            }
          }
          // go to next screen
          nextscreen();
          break;
        }
      case 3:
        {
          display.setDrawColor(0);
          display.drawPixel(22 + prevStickX, 40);
          display.setDrawColor(1);
          printSTR("Skipping Test", 34, 8);
          display.drawLine(0, 9, 128, 9);
          display.drawFrame(22 + 0, 15, 22 + 59, 21);
          if (N64_status.stick_x > 0) {
            display.drawLine(22 + N64_status.stick_x, 15, 22 + N64_status.stick_x, 35);
            display.drawPixel(22 + N64_status.stick_x, 40);
            prevStickX = N64_status.stick_x;
          }

          printSTR("Try to fill the box by", 22, 45);
          printSTR("slowly moving right", 22, 55);
          //Update LCD
          display.updateDisplay();

          if (button == "Press a button" && lastbutton == "Z") {
            // reset button
            lastbutton = "N/A";

            display.clearDisplay();
          }
          // go to next screen
          nextscreen();
          break;
        }
      case 4:
        {
          switch (test) {
            case 0:  // Display results
              {
                switch (results) {
                  case 0:
                    {
                      anastick = "Your Stick";
                      upx = bupx;
                      upy = bupy;
                      uprightx = buprightx;
                      uprighty = buprighty;
                      rightx = brightx;
                      righty = brighty;
                      downrightx = bdownrightx;
                      downrighty = bdownrighty;
                      downx = bdownx;
                      downy = bdowny;
                      downleftx = bdownleftx;
                      downlefty = bdownlefty;
                      leftx = bleftx;
                      lefty = blefty;
                      upleftx = bupleftx;
                      uplefty = buplefty;

                      if (button == "Press a button" && lastbutton == "A") {
                        // reset button
                        lastbutton = "N/A";
                        results = 1;
                        display.clearDisplay();
                        break;
                      }
                      printSTR(anastick, 22 + 50, 15);

                      display.drawStr(22 + 50, 25, "U:");
                      printSTR(String(upy), 100, 25);
                      display.drawStr(22 + 50, 35, "D:");
                      printSTR(String(downy), 100, 35);
                      display.drawStr(22 + 50, 45, "L:");
                      printSTR(String(leftx), 100, 45);
                      display.drawStr(22 + 50, 55, "R:");
                      printSTR(String(rightx), 100, 55);

                      display.drawLine(xax + upx / 4, yax - upy / 4, xax + uprightx / 4, yax - uprighty / 4);
                      display.drawLine(xax + uprightx / 4, yax - uprighty / 4, xax + rightx / 4, yax - righty / 4);
                      display.drawLine(xax + rightx / 4, yax - righty / 4, xax + downrightx / 4, yax - downrighty / 4);
                      display.drawLine(xax + downrightx / 4, yax - downrighty / 4, xax + downx / 4, yax - downy / 4);
                      display.drawLine(xax + downx / 4, yax - downy / 4, xax + downleftx / 4, yax - downlefty / 4);
                      display.drawLine(xax + downleftx / 4, yax - downlefty / 4, xax + leftx / 4, yax - lefty / 4);
                      display.drawLine(xax + leftx / 4, yax - lefty / 4, xax + upleftx / 4, yax - uplefty / 4);
                      display.drawLine(xax + upleftx / 4, yax - uplefty / 4, xax + upx / 4, yax - upy / 4);

                      display.drawPixel(xax, yax);

                      //Update LCD
                      display.updateDisplay();
                      break;
                    }
                  case 1:
                    {
                      anastick = "Original";
                      upx = 1;
                      upy = 84;
                      uprightx = 67;
                      uprighty = 68;
                      rightx = 83;
                      righty = -2;
                      downrightx = 67;
                      downrighty = -69;
                      downx = 3;
                      downy = -85;
                      downleftx = -69;
                      downlefty = -70;
                      leftx = -85;
                      lefty = 0;
                      upleftx = -68;
                      uplefty = 68;

                      if (button == "Press a button" && lastbutton == "A") {
                        // reset button
                        lastbutton = "N/A";
                        results = 0;
                        display.clearDisplay();
                        break;
                      }
                      printSTR(anastick, 22 + 50, 15);

                      display.drawStr(22 + 50, 25, "U:");
                      printSTR(String(upy), 100, 25);
                      display.drawStr(22 + 50, 35, "D:");
                      printSTR(String(downy), 100, 35);
                      display.drawStr(22 + 50, 45, "L:");
                      printSTR(String(leftx), 100, 45);
                      display.drawStr(22 + 50, 55, "R:");
                      printSTR(String(rightx), 100, 55);

                      display.drawLine(xax + upx / 4, yax - upy / 4, xax + uprightx / 4, yax - uprighty / 4);
                      display.drawLine(xax + uprightx / 4, yax - uprighty / 4, xax + rightx / 4, yax - righty / 4);
                      display.drawLine(xax + rightx / 4, yax - righty / 4, xax + downrightx / 4, yax - downrighty / 4);
                      display.drawLine(xax + downrightx / 4, yax - downrighty / 4, xax + downx / 4, yax - downy / 4);
                      display.drawLine(xax + downx / 4, yax - downy / 4, xax + downleftx / 4, yax - downlefty / 4);
                      display.drawLine(xax + downleftx / 4, yax - downlefty / 4, xax + leftx / 4, yax - lefty / 4);
                      display.drawLine(xax + leftx / 4, yax - lefty / 4, xax + upleftx / 4, yax - uplefty / 4);
                      display.drawLine(xax + upleftx / 4, yax - uplefty / 4, xax + upx / 4, yax - upy / 4);

                      display.drawPixel(xax, yax);

                      //Update LCD
                      display.updateDisplay();
                      break;
                    }

                }  //results
                break;
              }  //display results

            case 1:  // +y Up
              {
                display.drawStr(34, 26, "Hold Stick Up");
                display.drawStr(34, 34, "then press A");
                //display.drawBitmap(110, 60, ana1);

                if (button == "Press a button" && lastbutton == "A") {
                  bupx = N64_status.stick_x;
                  bupy = N64_status.stick_y;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                  test = 2;
                }
                break;
              }

            case 2:  // +y+x Up-Right
              {
                display.drawStr(42, 26, "Up-Right");
                //display.drawBitmap(110, 60, ana2);

                if (button == "Press a button" && lastbutton == "A") {
                  buprightx = N64_status.stick_x;
                  buprighty = N64_status.stick_y;
                  test = 3;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }

            case 3:  // +x Right
              {
                display.drawStr(50, 26, "Right");
                //display.drawBitmap(110, 60, ana3);

                if (button == "Press a button" && lastbutton == "A") {
                  brightx = N64_status.stick_x;
                  brighty = N64_status.stick_y;
                  test = 4;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }

            case 4:  // -y+x Down-Right
              {
                display.drawStr(38, 26, "Down-Right");
                //display.drawBitmap(110, 60, ana4);

                if (button == "Press a button" && lastbutton == "A") {
                  bdownrightx = N64_status.stick_x;
                  bdownrighty = N64_status.stick_y;
                  test = 5;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }

            case 5:  // -y Down
              {
                display.drawStr(49, 26, "Down");
                //display.drawBitmap(110, 60, ana5);

                if (button == "Press a button" && lastbutton == "A") {
                  bdownx = N64_status.stick_x;
                  bdowny = N64_status.stick_y;
                  test = 6;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }

            case 6:  // -y-x Down-Left
              {
                display.drawStr(39, 26, "Down-Left");
                //display.drawBitmap(110, 60, ana6);

                if (button == "Press a button" && lastbutton == "A") {
                  bdownleftx = N64_status.stick_x;
                  bdownlefty = N64_status.stick_y;
                  test = 7;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }

            case 7:  // -x Left
              {
                display.drawStr(51, 26, "Left");
                //display.drawBitmap(110, 60, ana7);

                if (button == "Press a button" && lastbutton == "A") {
                  bleftx = N64_status.stick_x;
                  blefty = N64_status.stick_y;
                  test = 8;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }

            case 8:  // +y+x Up-Left
              {
                display.drawStr(43, 26, "Up-Left");
                //display.drawBitmap(110, 60, ana8);

                if (button == "Press a button" && lastbutton == "A") {
                  bupleftx = N64_status.stick_x;
                  buplefty = N64_status.stick_y;
                  test = 0;
                  // reset button
                  lastbutton = "N/A";

                  display.clearDisplay();
                }
                break;
              }
          }
          if (test != 0) {
            display.drawStr(38, 8, "Benchmark");
            display.drawLine(0, 9, 128, 9);
          }
          display.updateDisplay();
          // go to next screen
          nextscreen();
          break;
        }
    }
  }
}
#endif

/******************************************
   N64 Controller Pak Functions
   (connected via Controller)
 *****************************************/
// Reset the controller
void resetController() {
  const byte command[] = { 0xFF };
  noInterrupts();
  sendJoyBus(command, sizeof(command));
  interrupts();
  delay(100);
}

// read 3 bytes from controller
void checkController() {
  byte response[8];
  const byte command[] = { 0x00 };

  display_Clear();

  // Check if line is HIGH
  if (!N64_QUERY)
    print_FatalError(F("Data line LOW"));

  // don't want interrupts getting in the way
  noInterrupts();
  sendJoyBus(command, sizeof(command));
  recvJoyBus(response, sizeof(response));
  // end of time sensitive code
  interrupts();

  if (response[0] != 0x05)
    print_FatalError(F("Controller not found"));
  if (response[2] != 0x01)
    print_FatalError(F("Controller Pak not found"));
}

// read 32bytes from controller pak and calculate CRC
byte readBlock(byte* output, word myAddress) {
  byte response_crc;
  // Calculate the address CRC
  word myAddressCRC = addrCRC(myAddress);
  const byte command[] = { 0x02, (byte)(myAddressCRC >> 8), (byte)(myAddressCRC & 0xff) };
  word error;

  // don't want interrupts getting in the way
  noInterrupts();
  sendJoyBus(command, sizeof(command));
  error = recvJoyBus(output, 32);
  if (error == 0)
    error = recvJoyBus(&response_crc, 1);
  // end of time sensitive code
  interrupts();

  if (error) {
    myFile.close();
    println_Msg(F("Controller Pak was"));
    println_Msg(F("not dumped due to a"));
    print_FatalError(F("read timeout"));
  }

  // Compare with computed CRC
  if (response_crc != dataCRC(output)) {
    display_Clear();
    // Close the file:
    myFile.close();
    println_Msg(F("Controller Pak was"));
    println_Msg(F("not dumped due to a"));
    print_FatalError(F("protocol CRC error"));
  }

  return response_crc;
}

// reads the MPK file to the sd card
void readMPK() {
  // Change to root
  sd.chdir("/");
  // Make MPK directory
  sd.mkdir("N64/MPK", true);
  // Change to MPK directory
  sd.chdir("N64/MPK");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "%d", foldern);
  strcat(fileName, ".mpk");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open crc file on sd card
  sprintf(filePath, "%d", foldern - 1);
  strcat(filePath, ".crc");
  FsFile crcFile;
  if (!crcFile.open(filePath, O_RDWR | O_CREAT)) {
    print_FatalError(open_file_STR);
  }

  //open mpk file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(open_file_STR);
  }

  print_Msg(F("Saving N64/MPK/"));
  println_Msg(fileName);
  display_Update();

  // Dummy write because first write to file takes 1 second and messes up timing
  blinkLED();
  myFile.write(0xFF);
  myFile.rewind();
  blinkLED();

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(0x7FFF);
  draw_progressbar(0, totalProgressBar);

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word currSdBuffer = 0x0000; currSdBuffer < 0x8000; currSdBuffer += 512) {
    // Read 32 byte block into sdBuffer
    for (word currBlock = 0; currBlock < sizeof(sdBuffer); currBlock += 32) {
      // Read one block of the Controller Pak into array myBlock and write CRC of that block to crc file
      crcFile.write(readBlock(&sdBuffer[currBlock], currSdBuffer + currBlock));

      // Real N64 has about 627us pause between banks, add a bit extra delay
      if (currBlock < 479)
        delayMicroseconds(800);
    }
    // This will take 1300us
    blinkLED();
    myFile.write(sdBuffer, sizeof(sdBuffer));
    // Blink led
    blinkLED();
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  // Close the file:
  myFile.close();
  crcFile.close();
}

// verifies if read was successful
void verifyCRC() {
  writeErrors = 0;

  print_STR(verifying_STR, 1);
  display_Update();

  //open CRC file on sd card
  FsFile crcFile;
  if (!crcFile.open(filePath, O_READ)) {
    print_FatalError(open_file_STR);
  }

  //open MPK file on sd card
  if (!myFile.open(fileName, O_READ)) {
    print_FatalError(open_file_STR);
  }

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(0x7FFF);
  draw_progressbar(0, totalProgressBar);

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word currSdBuffer = 0x0000; currSdBuffer < 0x8000; currSdBuffer += 512) {
    // Read 32 bytes into SD buffer
    myFile.read(sdBuffer, 512);

    // Compare 32 byte block CRC to CRC from file
    for (word currBlock = 0; currBlock < 512; currBlock += 32) {
      // Calculate CRC of block and compare against crc file
      if (dataCRC(&sdBuffer[currBlock]) != crcFile.read())
        writeErrors++;
    }

    // Blink led
    blinkLED();
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  // Close the file:
  myFile.close();
  crcFile.close();

  if (writeErrors == 0) {
    println_Msg(F("Saved successfully"));
    sd.remove(filePath);
    display_Update();
  } else {
    print_STR(error_STR, 0);
    print_Msg(writeErrors);
    println_Msg(F(" blocks "));
    print_Error(did_not_verify_STR);
  }
}

// Calculates the checksum of the header
boolean checkHeader(byte* buf) {
  word sum = 0;
  word buf_sum = (buf[28] << 8) + buf[29];

  // first 28 bytes are the header, then comes the checksum(word) followed by the reverse checksum(0xFFF2 - checksum)
  for (byte i = 0; i < 28; i += 2) {
    sum += (buf[i] << 8) + buf[i + 1];
  }

  return sum == buf_sum;
}

// verifies if Controller Pak holds valid header data
void validateMPK() {
  byte writeErrors = 0;
  boolean failed = false;
  SdFile mpk_file;
  byte buf[256];

  //open file on sd card
  if (!mpk_file.open(fileName, O_READ)) {
    print_FatalError(open_file_STR);
  }

  // Read first 256 byte which contains the header including checksum and reverse checksum and three copies of it
  mpk_file.read(buf, sizeof(buf));

  //Check all four header copies
  writeErrors = 0;
  if (!checkHeader(&buf[0x20]))
    writeErrors++;
  if (!checkHeader(&buf[0x60]))
    writeErrors++;
  if (!checkHeader(&buf[0x80]))
    writeErrors++;
  if (!checkHeader(&buf[0xC0]))
    writeErrors++;
  if (writeErrors)
    failed = true;

  print_Msg(F("HDR: "));
  print_Msg(4 - writeErrors);
  print_Msg(F("/4 - "));
  display_Update();

  // Check both TOC copies
  writeErrors = 0;

  // Read 2nd and 3rd 256 byte page with TOC info
  for (word currSdBuffer = 0x100; currSdBuffer < 0x300; currSdBuffer += 256) {
    byte sum = 0;

    // Read 256 bytes into SD buffer
    mpk_file.read(buf, sizeof(buf));

    // Calculate TOC checksum
    for (byte i = 5; i < 128; i++) {
      sum += buf[(i << 1) + 1];
    }
    if (buf[1] != sum)
      writeErrors++;
  }
  if (writeErrors)
    failed = true;
  print_Msg(F("ToC: "));
  print_Msg(2 - writeErrors);
  println_Msg(F("/2"));

  print_Msg(F("Consistency check "));
  if (failed) {
    errorLvl = 1;
    print_Msg(F("failed"));
  } else {
    errorLvl = 0;
    print_Msg(F("pased"));
  }
  display_Update();

  // Close the file:
  mpk_file.close();
}

void writeMPK() {
  // 3 command bytes, 32 data bytes
  byte command[3 + 32];
  command[0] = 0x03;

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_Msg(F("Writing "));
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    //Initialize progress bar
    uint32_t totalProgressBar = 0x7FFF;
    draw_progressbar(0, totalProgressBar);

    for (word address = 0x0000; address < 0x8000; address += 32) {
      myFile.read(command + 3, sizeof(command) - 3);

      word address_with_crc = addrCRC(address);
      command[1] = (byte)(address_with_crc >> 8);
      command[2] = (byte)(address_with_crc & 0xff);

      // don't want interrupts getting in the way
      noInterrupts();
      sendJoyBus(command, sizeof(command));
      // Enable interrupts
      interrupts();

      // Real N64 has about 627us pause between banks, add a bit extra delay
      delayMicroseconds(650);

      if ((address & 0x1FF) == 0) {
        // Blink led
        // Update progress bar
        blinkLED();
        draw_progressbar(address, totalProgressBar);
      }
    }
    // Close the file:
    myFile.close();
  } else {
    print_FatalError(open_file_STR);
  }
}

// verifies if write was successful
void verifyMPK() {
  byte block[32];
  writeErrors = 0;

  print_STR(verifying_STR, 1);
  display_Update();

  //open file on sd card
  if (!myFile.open(filePath, O_READ)) {
    print_FatalError(open_file_STR);
  }

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(0x7FFF);
  draw_progressbar(0, totalProgressBar);

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word currSdBuffer = 0x0000; currSdBuffer < 0x8000; currSdBuffer += sizeof(sdBuffer)) {
    // Read 512 bytes into SD buffer
    myFile.read(sdBuffer, sizeof(sdBuffer));

    // Compare 32 byte block
    for (word currBlock = 0; currBlock < sizeof(sdBuffer); currBlock += 32) {
      // Read one block of the Controller Pak
      readBlock(block, currSdBuffer + currBlock);

      // Check against file on SD card
      for (byte currByte = 0; currByte < 32; currByte++) {
        if (sdBuffer[currBlock + currByte] != block[currByte]) {
          writeErrors++;
        }
      }
      // Real N64 has about 627us pause between banks, add a bit extra delay
      if (currBlock < 479)
        delayMicroseconds(1500);
    }

    // Blink led
    blinkLED();
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }

  // Close the file:
  myFile.close();
  if (writeErrors == 0) {
    println_Msg(F("Written successfully"));
    display_Update();
  } else {
    print_STR(error_STR, 0);
    print_Msg(writeErrors);
    print_STR(_bytes_STR, 1);
    print_Error(did_not_verify_STR);
  }
}

/******************************************
  N64 Cartridge functions
*****************************************/
void printCartInfo_N64() {
  // Check cart
  getCartInfo_N64();

  // Print start page
  if (cartSize != 0) {
    display_Clear();
    print_Msg(F("Title: "));
    println_Msg(romName);
    print_Msg(F("Serial: "));
    println_Msg(cartID);
    print_Msg(F("Revision: "));
    println_Msg(romVersion);
    print_Msg(F("ROM Size: "));
    print_Msg(cartSize);
    println_Msg(F(" MB"));
    print_Msg(F("Save Type: "));
    switch (saveType) {
      case 1:
        println_Msg(F("SRAM"));
        break;
      case 4:
        println_Msg(F("FLASH"));
        break;
      case 5:
        println_Msg(F("4K EEPROM"));
        eepPages = 64;
        break;
      case 6:
        println_Msg(F("16K EEPROM"));
        eepPages = 256;
        break;
      default:
        println_Msg(F("None/Unknown"));
        break;
    }

    print_Msg(F("CRC1: "));
    println_Msg(checksumStr);

    // Wait for user input
    println_Msg(F(" "));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  } else {
    // Display error
    display_Clear();
    println_Msg(F("GAMEPAK ERROR"));
    println_Msg("");
    print_Msg(F("Title: "));
    println_Msg(romName);
    print_Msg(F("Serial: "));
    println_Msg(cartID);
    print_Msg(F("CRC1: "));
    println_Msg(checksumStr);
    display_Update();

    strcpy(romName, "GPERROR");
    print_Error(F("Cartridge unknown"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();

    // Set cartsize manually
    unsigned char N64RomMenu;
    // Copy menuOptions out of progmem
    convertPgm(romOptionsN64, 6);
    N64RomMenu = question_box(F("Select ROM size"), menuOptions, 6, 0);

    // wait for user choice to come back from the question box menu
    switch (N64RomMenu) {
      case 0:
        // 4MB
        cartSize = 4;
        break;

      case 1:
        // 8MB
        cartSize = 8;
        break;

      case 2:
        // 12MB
        cartSize = 12;
        break;

      case 3:
        // 16MB
        cartSize = 16;
        break;

      case 4:
        // 32MB
        cartSize = 32;
        break;

      case 5:
        // 64MB
        cartSize = 64;
        break;
    }
  }
}

/* look-up the calculated crc in the file n64.txt on sd card
  boolean searchCRC(char crcStr[9]) {
  boolean result = 0;
  char tempStr2[2];
  char tempStr1[9];
  char tempStr[5];

  // Change to root dir
  sd.chdir("/");

  if (myFile.open("n64.txt", O_READ)) {
    // Loop through file
    while (myFile.available()) {
      // Read 8 bytes into String, do it one at a time so byte order doesn't get mixed up
      sprintf(tempStr1, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(tempStr1, tempStr2);
      }

      // Check if string is a match
      if (strcasecmp(tempStr1, crcStr) == 0) {
        // Skip the , in the file
        myFile.seekCur(1);

        // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
        sprintf(tempStr, "%c", myFile.read());
        for (byte i = 0; i < 3; i++) {
          sprintf(tempStr2, "%c", myFile.read());
          strcat(tempStr, tempStr2);
        }

        if (strcmp(tempStr, cartID) == 0) {
          result = 1;
          break;
        }
        else {
          result = 0;
          break;
        }
      }
      // If no match, empty string, advance by 12 and try again
      else {
        myFile.seekCur(12);
      }
    }
    // Close the file:
    myFile.close();
    return result;
  }
  else {
    print_FatalError(F("n64.txt missing"));
  }
  }*/

// look-up cart id in file n64.txt on sd card
void getCartInfo_N64() {
  char tempStr[9];
  int read_bytes;

  // cart not in list
  cartSize = 0;
  saveType = 0;

  // Read cart id
  idCart();

  display_Clear();
  println_Msg(F("Searching database..."));
  display_Update();

  if (myFile.open("n64.txt", O_READ)) {
    // Loop through file
    while (myFile.available()) {
      // Skip first line with name
      skip_line(&myFile);

      // Skip over the CRC32 checksum
      myFile.seekCur(9);

      // Read 8 bytes into String
      read_bytes = myFile.read(tempStr, 8);
      tempStr[read_bytes == -1 ? 0 : read_bytes] = 0;

      // Check if string is a match
      if (strcmp(tempStr, checksumStr) == 0) {
        // Skip the , in the file
        myFile.seekCur(1);

        read_bytes = myFile.read(tempStr, 2);
        tempStr[read_bytes == -1 ? 0 : read_bytes] = 0;
        cartSize = atoi(tempStr);

        // Skip the , in the file
        myFile.seekCur(1);

        // Read the next ascii character and subtract 48 to convert to decimal
        saveType = myFile.read() - 48;

        // End loop
        break;
      }
      // If no match skip to next entry
      else {
        // skip rest of line
        myFile.seekCur(7);
        // skip third empty line
        skip_line(&myFile);
      }
    }
    // Close the file:
    myFile.close();
  } else {
    print_FatalError(F("n64.txt missing"));
  }
}

// Read rom ID
void idCart() {
  // Set the address
  setAddress_N64(romBase);
  // Read first 64 bytes of rom
  for (int c = 0; c < 64; c += 2) {
    // split word
    word myWord = readWord_N64();
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }

  // CRC1
  sprintf(checksumStr, "%02X%02X%02X%02X", sdBuffer[0x10], sdBuffer[0x11], sdBuffer[0x12], sdBuffer[0x13]);

  // Get cart id
  cartID[0] = sdBuffer[0x3B];
  cartID[1] = sdBuffer[0x3C];
  cartID[2] = sdBuffer[0x3D];
  cartID[3] = sdBuffer[0x3E];

  // Get rom version
  romVersion = sdBuffer[0x3F];

  // If name consists out of all japanese characters use cart id
  if (buildRomName(romName, &sdBuffer[0x20], 20) == 0) {
    strcpy(romName, cartID);
  }

#ifdef savesummarytotxt
  // Get CRC1
  for (int i = 0; i < 4; i++) {
    if (sdBuffer[0x10 + i] < 0x10) {
      CRC1 += '0';
    }
    CRC1 += String(sdBuffer[0x10 + i], HEX);
  }

  // Get CRC2
  for (int i = 0; i < 4; i++) {
    if (sdBuffer[0x14 + i] < 0x10) {
      CRC2 += '0';
    }
    CRC2 += String(sdBuffer[0x14 + i], HEX);
  }
#endif
}

// Write Eeprom to cartridge
void writeEeprom() {
  if ((saveType == 5) || (saveType == 6)) {

    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    println_Msg(F("Writing..."));
    println_Msg(filePath);
    display_Update();

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {
      // 2 command bytes and 8 data bytes
      byte command[2 + 8];
      command[0] = 0x05;

      // Note: eepPages can be 256, so page must be able to get to 256 for the
      // loop to exit. So it is not possible to use command[1] directly as loop
      // counter.
      for (int page = 0; page < eepPages; page++) {
        command[1] = page;
        // TODO: read 512 bytes in a 512 + 2 bytes buffer, and move the command start 32 bytes at a time
        myFile.read(command + 2, sizeof(command) - 2);
        // Disable interrupts for more uniform clock pulses

        // Blink led
        blinkLED();
        if (page)
          delay(50);  // Wait 50ms between pages when writing

        noInterrupts();
        sendJoyBus(command, sizeof(command));
        interrupts();
      }

      // Close the file:
      myFile.close();
      print_STR(done_STR, 1);
      display_Update();
      delay(600);
    } else {
      print_FatalError(sd_error_STR);
    }
  } else {
    print_FatalError(F("Savetype Error"));
  }
}

void readEepromPageList(byte* output, byte page_number, byte page_count) {
  byte command[] = { 0x04, page_number };

  // Disable interrupts for more uniform clock pulses
  while (page_count--) {
    // Blink led
    blinkLED();

    noInterrupts();
    sendJoyBus(command, sizeof(command));
    // XXX: is it possible to read more than 8 bytes at a time ?
    recvJoyBus(output, 8);
    interrupts();

    if (page_count)
      delayMicroseconds(600);  // wait 600us between pages when reading

    command[1]++;
    output += 8;
  }
}

// Dump Eeprom to SD
void readEeprom() {
  if ((saveType == 5) || (saveType == 6)) {
    // Get name, add extension and convert to char array for sd lib
    snprintf_P(fileName, sizeof(fileName), N64_EEP_FILENAME_FMT, romName);

    // create a new folder for the save file
    EEPROM_readAnything(0, foldern);
    snprintf_P(folder, sizeof(folder), N64_SAVE_DIRNAME_FMT, romName, foldern);
    sd.mkdir(folder, true);
    sd.chdir(folder);

    // write new folder number back to eeprom
    foldern = foldern + 1;
    EEPROM_writeAnything(0, foldern);

    // Open file on sd card
    if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
      print_FatalError(create_file_STR);
    }

    for (int i = 0; i < eepPages; i += sizeof(sdBuffer) / 8) {
      readEepromPageList(sdBuffer, i, sizeof(sdBuffer) / 8);
      // Write 64 pages at once to the SD card
      myFile.write(sdBuffer, sizeof(sdBuffer));
    }
    // Close the file:
    myFile.close();
    //clear the screen
    display_Clear();
    print_Msg(F("Saved to "));
    print_Msg(folder);
    println_Msg(F("/"));
    display_Update();
  } else {
    print_FatalError(F("Savetype Error"));
  }
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyEeprom() {
  unsigned long writeErrors;

  if ((saveType == 5) || (saveType == 6)) {
    writeErrors = 0;

    display_Clear();
    print_Msg(F("Verifying against "));
    println_Msg(filePath);
    display_Update();

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {
      for (int i = 0; i < eepPages; i += sizeof(sdBuffer) / 8) {
        readEepromPageList(sdBuffer, i, sizeof(sdBuffer) / 8);
        // Check sdBuffer content against file on sd card
        for (size_t c = 0; c < sizeof(sdBuffer); c++) {
          if (myFile.read() != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
      // Close the file:
      myFile.close();
    } else {
      // SD Error
      writeErrors = 999999;
      print_FatalError(sd_error_STR);
    }
    // Return 0 if verified ok, or number of errors
    return writeErrors;
  } else {
    print_FatalError(F("Savetype Error"));
    return 1;
  }
}

/******************************************
  SRAM functions
*****************************************/
// Write sram to cartridge
void writeSram(unsigned long sramSize) {
  if (saveType == 1) {
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    println_Msg(F("Writing..."));
    println_Msg(filePath);
    display_Update();

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {
      for (unsigned long currByte = sramBase; currByte < (sramBase + sramSize); currByte += 512) {

        // Read save from SD into buffer
        myFile.read(sdBuffer, 512);

        // Set the address for the next 512 bytes
        setAddress_N64(currByte);

        for (int c = 0; c < 512; c += 2) {
          // Join bytes to word
          word myWord = ((sdBuffer[c] & 0xFF) << 8) | (sdBuffer[c + 1] & 0xFF);

          // Write word
          writeWord_N64(myWord);
        }
      }
      // Close the file:
      myFile.close();
      print_STR(done_STR, 1);
      display_Update();
    } else {
      print_FatalError(sd_error_STR);
    }

  } else {
    print_FatalError(F("Savetype Error"));
  }
}

// Read sram and save to the SD card
void readSram(unsigned long sramSize, byte flashramType) {
  int offset = 512;
  int bufferSize = 512;
  if (flashramType == 2) {
    offset = 64;
    bufferSize = 128;
  }

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);

  if (saveType == 4) {
    strcat(fileName, ".fla");
  } else if (saveType == 1) {
    strcat(fileName, ".sra");
  } else {
    print_FatalError(F("Savetype Error"));
  }

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "N64/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  for (unsigned long currByte = sramBase; currByte < (sramBase + (sramSize / flashramType)); currByte += offset) {
    // Set the address
    setAddress_N64(currByte);

    for (int c = 0; c < bufferSize; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    myFile.write(sdBuffer, bufferSize);
  }
  // Close the file:
  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/"));
  display_Update();
}

unsigned long verifySram(unsigned long sramSize, byte flashramType) {
  writeErrors = 0;

  int offset = 512;
  int bufferSize = 512;
  if (flashramType == 2) {
    offset = 64;
    bufferSize = 128;
  }

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currByte = sramBase; currByte < (sramBase + (sramSize / flashramType)); currByte += offset) {
      // Set the address
      setAddress_N64(currByte);

      for (int c = 0; c < bufferSize; c += 2) {
        // split word
        word myWord = readWord_N64();
        byte loByte = myWord & 0xFF;
        byte hiByte = myWord >> 8;

        // write to buffer
        sdBuffer[c] = hiByte;
        sdBuffer[c + 1] = loByte;
      }
      // Check sdBuffer content against file on sd card
      for (int i = 0; i < bufferSize; i++) {
        if (myFile.read() != sdBuffer[i]) {
          writeErrors++;
        }
      }
    }
    // Close the file:
    myFile.close();
  } else {
    print_FatalError(sd_error_STR);
  }
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}

/******************************************
  Flashram functions
*****************************************/
// Send a command to the flashram command register
void sendFramCmd(unsigned long myCommand) {
  // Split command into two words
  word myComLowOut = myCommand & 0xFFFF;
  word myComHighOut = myCommand >> 16;

  // Set address to command register
  setAddress_N64(0x08010000);
  // Send command
  writeWord_N64(myComHighOut);
  writeWord_N64(myComLowOut);
}

// Init fram
void initFram() {
  // FRAM_EXECUTE_CMD
  sendFramCmd(0xD2000000);
  delay(10);
  // FRAM_EXECUTE_CMD
  sendFramCmd(0xD2000000);
  delay(10);
  //FRAM_STATUS_MODE_CMD
  sendFramCmd(0xE1000000);
  delay(10);
}

void writeFram(byte flashramType) {
  if (saveType == 4) {
    // Erase fram
    eraseFram();

    // Check if empty
    if (blankcheck_N64(flashramType) == 0) {
      println_Msg(F("OK"));
      display_Update();
    } else {
      println_Msg(F("FAIL"));
      display_Update();
    }

    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    print_Msg(F("Writing "));
    println_Msg(filePath);
    display_Update();

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {
      // Init fram
      initFram();

      // Write all 8 fram banks
      print_Msg(F("Bank "));
      for (byte bank = 0; bank < 8; bank++) {
        print_Msg(bank);
        print_Msg(F(" "));
        display_Update();

        // Write one bank of 128*128 bytes
        for (byte offset = 0; offset < 128; offset++) {
          // Read save from SD into buffer
          myFile.read(sdBuffer, 128);

          // FRAM_WRITE_MODE_CMD
          sendFramCmd(0xB4000000);
          delay(1);

          // Set the address for the next 128 bytes
          setAddress_N64(0x08000000);

          // Send 128 bytes, 64 words
          for (byte c = 0; c < 128; c += 2) {
            // Join two bytes into one word
            word myWord = ((sdBuffer[c] & 0xFF) << 8) | (sdBuffer[c + 1] & 0xFF);
            // Write word
            writeWord_N64(myWord);
          }
          // Delay between each "DMA"
          delay(1);

          //FRAM_WRITE_OFFSET_CMD + offset
          sendFramCmd((0xA5000000 | (((bank * 128) + offset) & 0xFFFF)));
          delay(1);

          // FRAM_EXECUTE_CMD
          sendFramCmd(0xD2000000);
          while (waitForFram(flashramType)) {
            delay(1);
          }
        }
        // Delay between banks
        delay(20);
      }
      println_Msg("");
      // Close the file:
      myFile.close();
    } else {
      print_FatalError(sd_error_STR);
    }
  } else {
    print_FatalError(F("Savetype Error"));
  }
}

// Delete all 8 flashram banks
void eraseFram() {
  if (saveType == 4) {
    print_Msg(F("Erasing..."));
    display_Update();

    // Init fram
    initFram();

    // Erase fram
    // 0x4B00007F 0x4B0000FF 0x4B00017F 0x4B0001FF 0x4B00027F 0x4B0002FF 0x4B00037F 0x4B0003FF
    for (unsigned long bank = 0x4B00007F; bank < 0x4B00047F; bank += 0x80) {
      sendFramCmd(bank);
      delay(10);
      // FRAM_ERASE_MODE_CMD
      sendFramCmd(0x78000000);
      delay(10);
      // FRAM_EXECUTE_CMD
      sendFramCmd(0xD2000000);
      while (waitForFram(flashramType)) {
        delay(1);
      }
    }
  } else {
    print_FatalError(F("Savetype Error"));
  }
}

// Read flashram
void readFram(byte flashramType) {
  if (saveType == 4) {
    // Put flashram into read mode
    // FRAM_READ_MODE_CMD
    sendFramCmd(0xF0000000);
    // Read Flashram
    readSram(131072, flashramType);
  } else {
    print_FatalError(F("Savetype Error"));
  }
}

// Verify flashram
unsigned long verifyFram(byte flashramType) {
  // Put flashram into read mode
  // FRAM_READ_MODE_CMD
  sendFramCmd(0xF0000000);
  writeErrors = verifySram(131072, flashramType);
  return writeErrors;
}

// Blankcheck flashram
unsigned long blankcheck_N64(byte flashramType) {
  writeErrors = 0;

  int offset = 512;
  int bufferSize = 512;
  if (flashramType == 2) {
    offset = 64;
    bufferSize = 128;
  }

  // Put flashram into read mode
  // FRAM_READ_MODE_CMD
  sendFramCmd(0xF0000000);

  // Read Flashram
  for (unsigned long currByte = sramBase; currByte < (sramBase + (131072 / flashramType)); currByte += offset) {
    // Set the address for the next 512 bytes
    setAddress_N64(currByte);

    for (int c = 0; c < bufferSize; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    // Check sdBuffer content against file on sd card
    for (int i = 0; i < bufferSize; i++) {
      if (0xFF != sdBuffer[i]) {
        writeErrors++;
      }
    }
  }
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}

// Wait until current operation is done
byte waitForFram(byte flashramType) {
  byte framStatus = 0;
  byte statusMXL1100[] = { 0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1E };
  byte statusMXL1101[] = { 0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1D };
  byte statusMN63F81[] = { 0x11, 0x11, 0x80, 0x01, 0x00, 0x32, 0x00, 0xF1 };

  // FRAM_STATUS_MODE_CMD
  sendFramCmd(0xE1000000);
  delay(1);

  // Set address to Fram status register
  setAddress_N64(0x08000000);

  // Read Status
  for (byte c = 0; c < 8; c += 2) {
    // split word
    word myWord = readWord_N64();
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }

  if (flashramType == 2) {
    for (byte c = 0; c < 8; c++) {
      if (statusMXL1100[c] != sdBuffer[c]) {
        framStatus = 1;
      }
    }
  } else if (flashramType == 1) {
    //MX29L1101
    if (MN63F81MPN == false) {
      for (byte c = 0; c < 8; c++) {
        if (statusMXL1101[c] != sdBuffer[c]) {
          framStatus = 1;
        }
      }
    }
    //MN63F81MPN
    else if (MN63F81MPN == true) {
      for (byte c = 0; c < 8; c++) {
        if (statusMN63F81[c] != sdBuffer[c]) {
          framStatus = 1;
        }
      }
    }
  }
  return framStatus;
}

// Get flashram type
void getFramType() {

  // FRAM_STATUS_MODE_CMD
  sendFramCmd(0xE1000000);
  delay(10);

  // Set address to Fram status register
  setAddress_N64(0x08000000);

  // Read Status
  for (byte c = 0; c < 8; c += 2) {
    // split word
    word myWord = readWord_N64();
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }
  //MX29L1100
  if (sdBuffer[7] == 0x1e) {
    flashramType = 2;
    println_Msg(F("Type: MX29L1100"));
    display_Update();
  }
  //MX29L1101
  else if (sdBuffer[7] == 0x1d) {
    flashramType = 1;
    MN63F81MPN = false;
    println_Msg(F("Type: MX29L1101"));
    display_Update();
  }
  //MN63F81MPN
  else if (sdBuffer[7] == 0xf1) {
    flashramType = 1;
    MN63F81MPN = true;
    println_Msg(F("Type: MN63F81MPN"));
    display_Update();
  }
  // 29L1100KC-15B0 compat MX29L1101
  else if ((sdBuffer[7] == 0x8e) || (sdBuffer[7] == 0x84)) {
    flashramType = 1;
    MN63F81MPN = false;
    println_Msg(F("Type: 29L1100KC-15B0"));
    println_Msg(F("(compat. MX29L1101)"));
    display_Update();
  }
  // Type unknown
  else {
    for (byte c = 0; c < 8; c++) {
      print_Msg(sdBuffer[c], HEX);
      print_Msg(F(", "));
    }
    print_FatalError(F("Flashram unknown"));
  }
}

/******************************************
  Rom functions
*****************************************/
// Read rom and save to the SD card
#ifndef fastcrc
// dumping rom slow
void readRom_N64() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".Z64");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "N64/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // clear the screen
  // display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(cartSize)*1024 * 1024;
  draw_progressbar(0, totalProgressBar);

  for (unsigned long currByte = romBase; currByte < (romBase + (cartSize * 1024 * 1024)); currByte += 512) {
    // Blink led
    if ((currByte & 0x3FFF) == 0)
      blinkLED();

    // Set the address for the next 512 bytes
    setAddress_N64(currByte);

    for (word c = 0; c < 512; c += 2) {
      word myWord = readWord_N64();
      sdBuffer[c] = myWord >> 8;
      sdBuffer[c + 1] = myWord & 0xFF;
    }
    myFile.write(sdBuffer, 512);

    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  // Close the file:
  myFile.close();
}
#else
// dumping rom fast
uint32_t readRom_N64() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".Z64");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "N64/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // clear the screen
  // display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }

  byte buffer[1024];

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(cartSize)*1024 * 1024;
  draw_progressbar(0, totalProgressBar);

  // prepare crc32
  uint32_t oldcrc32 = 0xFFFFFFFF;

  // run combined dumper + crc32 routine for better performance, as N64 ROMs are quite large for an 8bit micro
  // currently dumps + checksums a 32MB cart in 170 seconds (down from 347 seconds)
  for (unsigned long currByte = romBase; currByte < (romBase + (cartSize * 1024 * 1024)); currByte += 1024) {
    // Blink led
    if (currByte % 16384 == 0)
      blinkLED();

    // Set the address for the first 512 bytes to dump
    setAddress_N64(currByte);
    // Wait 62.5ns (safety)
    NOP;

    for (int c = 0; c < 512; c += 2) {
      // Pull read(PH6) low
      PORTH &= ~(1 << 6);
      // Wait ~310ns
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;

      // data on PINK and PINF is valid now, read into sd card buffer
      buffer[c] = PINK;      // hiByte
      buffer[c + 1] = PINF;  // loByte

      // Pull read(PH6) high
      PORTH |= (1 << 6);

      // crc32 update
      UPDATE_CRC(oldcrc32, buffer[c]);
      UPDATE_CRC(oldcrc32, buffer[c + 1]);
    }

    // Set the address for the next 512 bytes to dump
    setAddress_N64(currByte + 512);
    // Wait 62.5ns (safety)
    NOP;

    for (int c = 512; c < 1024; c += 2) {
      // Pull read(PH6) low
      PORTH &= ~(1 << 6);
      // Wait ~310ns
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;

      // data on PINK and PINF is valid now, read into sd card buffer
      buffer[c] = PINK;      // hiByte
      buffer[c + 1] = PINF;  // loByte

      // Pull read(PH6) high
      PORTH |= (1 << 6);

      // crc32 update
      UPDATE_CRC(oldcrc32, buffer[c]);
      UPDATE_CRC(oldcrc32, buffer[c + 1]);
    }

    processedProgressBar += 1024;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // write out 1024 bytes to file
    myFile.write(buffer, 1024);
  }

  // Close the file:
  myFile.close();

  // Return checksum
  return oldcrc32;
}
#endif

#ifdef savesummarytotxt
// Save an info.txt with information on the dumped rom to the SD card
void savesummary_N64(boolean checkfound, char crcStr[9], unsigned long timeElapsed) {
  // Open file on sd card
  if (!myFile.open("N64/ROM/n64log.txt", O_RDWR | O_CREAT | O_APPEND)) {
    print_FatalError(sd_error_STR);
  }

  //Write the info
  myFile.print(F("Name\t: "));
  myFile.println(romName);

  myFile.print(F("ID\t: "));
  myFile.println(cartID);

  myFile.print(F("ROM CRC1: "));
  myFile.println(CRC1);

  myFile.print(F("ROM CRC2: "));
  myFile.println(CRC2);

  myFile.print(F("Size\t: "));
  myFile.print(cartSize);

  myFile.println(F(" MB"));
  myFile.print(F("Save\t: "));

  switch (saveType) {
    case 1:
      myFile.println(F("SRAM"));
      break;
    case 4:
      myFile.println(F("FLASH"));
      break;
    case 5:
      myFile.println(F("4K EEPROM"));
      break;
    case 6:
      myFile.println(F("16K EEPROM"));
      break;
    default:
      myFile.println(F("None/Unknown"));
      break;
  }

  myFile.print(F("Version\t: 1."));
  myFile.println(romVersion);

  myFile.print(F("Saved To: "));
  myFile.println(folder);

#ifdef RTC_installed
  myFile.print(F("Dumped\t: "));
  myFile.println(RTCStamp());
#endif

  myFile.print(F("CRC\t: "));
  myFile.print(crcStr);

  if (checkfound) {
    // Dump was a known good rom
    // myFile.println(F("Checksum matches"));
    myFile.println(" [Match]");
  } else {
    // myFile.println(F("Checksum not found"));
    myFile.println(" [No Match]");
  }

  myFile.print(F("Time\t: "));
  myFile.println(timeElapsed);

  myFile.println(F(" "));

  // Close the file:
  myFile.close();
}
#endif

/******************************************
   N64 Repro Flashrom Functions
 *****************************************/
void flashRepro_N64() {
  unsigned long sectorSize = 0;
  byte bufferSize = 0;
  // Check flashrom ID's
  idFlashrom_N64();

  // If the ID is known continue
  if (cartSize != 0) {
    // Print flashrom name
    if ((flashid == 0x227E) && (strcmp(cartID, "2201") == 0)) {
      print_Msg(F("Spansion S29GL256N"));
      if (cartSize == 64)
        println_Msg(F(" x2"));
      else
        println_Msg("");
    } else if ((flashid == 0x227E) && (strcmp(cartID, "2101") == 0)) {
      print_Msg(F("Spansion S29GL128N"));
    } else if ((flashid == 0x227E) && (strcmp(cartID, "2100") == 0)) {
      print_Msg(F("ST M29W128GL"));
    } else if ((flashid == 0x22C9) || (flashid == 0x22CB)) {
      print_Msg(F("Macronix MX29LV640"));
      if (cartSize == 16)
        println_Msg(F(" x2"));
      else
        println_Msg("");
    } else if (flashid == 0x8816)
      println_Msg(F("Intel 4400L0ZDQ0"));
    else if (flashid == 0x7E7E)
      println_Msg(F("Fujitsu MSP55LV100S"));
    else if ((flashid == 0x227E) && (strcmp(cartID, "2301") == 0))
      println_Msg(F("Fujitsu MSP55LV512"));
    else if ((flashid == 0x227E) && (strcmp(cartID, "3901") == 0))
      println_Msg(F("Intel 512M29EW"));

    // Print info
    print_Msg(F("ID: "));
    print_Msg(flashid_str);
    print_Msg(F(" Size: "));
    print_Msg(cartSize);
    println_Msg(F("MB"));
    println_Msg("");
    println_Msg(F("This will erase your"));
    println_Msg(F("Repro Cartridge."));
    println_Msg(F("Attention: Use 3.3V!"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  } else {
    println_Msg(F("Unknown flashrom"));
    print_Msg(F("ID: "));
    print_Msg(vendorID);
    print_Msg(F(" "));
    print_Msg(flashid_str);
    print_Msg(F(" "));
    println_Msg(cartID);
    println_Msg(F(" "));
    println_Msg(F("Press button for"));
    println_Msg(F("manual config"));
    println_Msg(F("This will erase your"));
    println_Msg(F("Repro Cartridge."));
    println_Msg(F("Attention: Use 3.3V!"));
    display_Update();
    wait();

    // clear IDs
    sprintf(vendorID, "%s", "CONF");
    flashid = 0;
    sprintf(flashid_str, "%s", "CONF");
    sprintf(cartID, "%s", "CONF");



    // Set cartsize manually
    unsigned char N64RomMenu;
    // Copy menuOptions out of progmem
    convertPgm(romOptionsN64, 6);
    N64RomMenu = question_box(F("Select flash size"), menuOptions, 6, 0);

    // wait for user choice to come back from the question box menu
    switch (N64RomMenu) {
      case 0:
        // 4MB
        cartSize = 4;
        break;

      case 1:
        // 8MB
        cartSize = 8;
        break;

      case 2:
        // 12MB
        cartSize = 12;
        break;

      case 3:
        // 16MB
        cartSize = 16;
        break;

      case 4:
        // 32MB
        cartSize = 32;
        break;

      case 5:
        // 64MB
        cartSize = 64;
        break;
    }

    // Set flash buffer manually
    unsigned char N64BufferMenu;
    // Copy menuOptions out of progmem
    convertPgm(bufferOptionsN64, 4);
    N64BufferMenu = question_box(F("Select buffer size"), menuOptions, 4, 0);

    // wait for user choice to come back from the question box menu
    switch (N64BufferMenu) {
      case 0:
        // no buffer
        bufferSize = 0;
        break;

      case 1:
        // 32 byte buffer
        bufferSize = 32;
        break;

      case 2:
        // 64 byte buffer
        bufferSize = 64;
        break;

      case 3:
        // 128 byte buffer
        bufferSize = 128;
        break;
    }

    // Set sector size manually
    unsigned char N64SectorMenu;
    // Copy menuOptions out of progmem
    convertPgm(sectorOptionsN64, 4);
    N64SectorMenu = question_box(F("Select sector size"), menuOptions, 4, 0);

    // wait for user choice to come back from the question box menu
    switch (N64SectorMenu) {
      case 0:
        // 8KB sectors
        sectorSize = 0x2000;
        break;

      case 1:
        // 32KB sectors
        sectorSize = 0x8000;
        break;

      case 2:
        // 64KB sectors
        sectorSize = 0x10000;
        break;

      case 3:
        // 128KB sectors
        sectorSize = 0x20000;
        break;
    }
  }

  // Launch file browser
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select z64 file"));
  display_Clear();
  display_Update();

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    print_Msg(F("File size: "));
    print_Msg(fileSize / 1048576);
    println_Msg(F("MB"));
    display_Update();

    // Compare file size to flashrom size
    if ((fileSize / 1048576) > cartSize) {
      print_FatalError(file_too_big_STR);
    }

    // Erase needed sectors
    if (flashid == 0x227E) {
      // Spansion S29GL256N or Fujitsu MSP55LV512 with 0x20000 sector size and 32 byte buffer
      eraseSector_N64(0x20000);
    } else if (flashid == 0x7E7E) {
      // Fujitsu MSP55LV100S
      eraseMSP55LV100_N64();
    } else if ((flashid == 0x8813) || (flashid == 0x8816)) {
      // Intel 4400L0ZDQ0
      eraseIntel4400_N64();
      resetIntel4400_N64();
    } else if ((flashid == 0x22C9) || (flashid == 0x22CB)) {
      // Macronix MX29LV640, C9 is top boot and CB is bottom boot block
      eraseSector_N64(0x8000);
    } else {
      eraseFlashrom_N64();
    }

    // Check if erase was successful
    if (blankcheckFlashrom_N64()) {
      // Write flashrom
      println_Msg(F("OK"));
      print_Msg(F("Writing "));
      println_Msg(filePath);
      display_Update();

      if ((strcmp(cartID, "3901") == 0) && (flashid == 0x227E)) {
        // Intel 512M29EW(64MB) with 0x20000 sector size and 128 byte buffer
        writeFlashBuffer_N64(0x20000, 128);
      } else if ((strcmp(cartID, "2100") == 0) && (flashid == 0x227E)) {
        // ST M29W128GH(16MB) with 0x20000 sector size and 64 byte buffer
        writeFlashBuffer_N64(0x20000, 64);
      } else if (flashid == 0x227E) {
        // Spansion S29GL128N/S29GL256N or Fujitsu MSP55LV512 with 0x20000 sector size and 32 byte buffer
        writeFlashBuffer_N64(0x20000, 32);
      } else if (flashid == 0x7E7E) {
        //Fujitsu MSP55LV100S
        writeMSP55LV100_N64(0x20000);
      } else if ((flashid == 0x22C9) || (flashid == 0x22CB)) {
        // Macronix MX29LV640 without buffer and 0x8000 sector size
        writeFlashrom_N64(0x8000);
      } else if ((flashid == 0x8813) || (flashid == 0x8816)) {
        // Intel 4400L0ZDQ0
        writeIntel4400_N64();
        resetIntel4400_N64();
      } else if (sectorSize) {
        if (bufferSize) {
          writeFlashBuffer_N64(sectorSize, bufferSize);
        } else {
          writeFlashrom_N64(sectorSize);
        }
      } else {
        print_FatalError(F("sectorSize not set"));
      }

      // Close the file:
      myFile.close();

      // Verify
      print_STR(verifying_STR, 0);
      display_Update();
      writeErrors = verifyFlashrom_N64();
      if (writeErrors == 0) {
        println_Msg(F("OK"));
        display_Update();
      } else {
        print_Msg(writeErrors);
        print_Msg(F(" bytes "));
        print_Error(did_not_verify_STR);
      }
    } else {
      // Close the file
      myFile.close();
      print_Error(F("failed"));
    }
  } else {
    print_Error(F("Can't open file"));
  }

  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
  display_Clear();
  display_Update();
}

// Reset to read mode
void resetIntel4400_N64() {
  for (unsigned long currPartition = 0; currPartition < (cartSize * 0x100000); currPartition += 0x20000) {
    setAddress_N64(romBase + currPartition);
    writeWord_N64(0xFF);
  }
}

// Reset Fujitsu MSP55LV100S
void resetMSP55LV100_N64(unsigned long flashBase) {
  // Send reset Command
  setAddress_N64(flashBase);
  writeWord_N64(0xF0F0);
  delay(100);
}

// Common reset command
void resetFlashrom_N64(unsigned long flashBase) {
  // Send reset Command
  setAddress_N64(flashBase);
  writeWord_N64(0xF0);
  delay(100);
}

void idFlashrom_N64() {
  // Set size to 0 if no ID is found
  cartSize = 0;

  // Send flashrom ID command
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0xAA);
  setAddress_N64(romBase + (0x2AA << 1));
  writeWord_N64(0x55);
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0x90);

  // Read 1 byte vendor ID
  setAddress_N64(romBase);
  sprintf(vendorID, "%02X", readWord_N64());
  // Read 2 bytes flashrom ID
  flashid = readWord_N64();
  sprintf(flashid_str, "%04X", flashid);
  // Read 2 bytes secondary flashrom ID
  setAddress_N64(romBase + 0x1C);
  sprintf(cartID, "%04X", ((readWord_N64() << 8) | (readWord_N64() & 0xFF)));

  // Spansion S29GL256N(32MB/64MB) with either one or two flashrom chips
  if ((strcmp(cartID, "2201") == 0) && (flashid == 0x227E)) {
    cartSize = 32;

    // Reset flashrom
    resetFlashrom_N64(romBase);

    // Test for second flashrom chip at 0x2000000 (32MB)
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(romBase + 0x2000000 + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0x90);

    char tempID[5];
    setAddress_N64(romBase + 0x2000000);
    // Read manufacturer ID
    readWord_N64();
    // Read flashrom ID
    sprintf(tempID, "%04X", readWord_N64());

    // Check if second flashrom chip is present
    if (strcmp(tempID, "227E") == 0) {
      cartSize = 64;
    }
    resetFlashrom_N64(romBase + 0x2000000);
  }

  // Macronix MX29LV640(8MB/16MB) with either one or two flashrom chips
  else if ((flashid == 0x22C9) || (flashid == 0x22CB)) {
    cartSize = 8;

    resetFlashrom_N64(romBase + 0x800000);

    // Test for second flashrom chip at 0x800000 (8MB)
    setAddress_N64(romBase + 0x800000 + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(romBase + 0x800000 + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + 0x800000 + (0x555 << 1));
    writeWord_N64(0x90);

    char tempID[5];
    setAddress_N64(romBase + 0x800000);
    // Read manufacturer ID
    readWord_N64();
    // Read flashrom ID
    sprintf(tempID, "%04X", readWord_N64());

    // Check if second flashrom chip is present
    if ((strcmp(tempID, "22C9") == 0) || (strcmp(tempID, "22CB") == 0)) {
      cartSize = 16;
    }
    resetFlashrom_N64(romBase + 0x800000);
  }

  // Intel 4400L0ZDQ0 (64MB)
  else if (flashid == 0x8816) {
    // Found first flashrom chip, set to 32MB
    cartSize = 32;
    resetIntel4400_N64();

    // Test if second half of the flashrom might be hidden
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(romBase + 0x2000000 + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + 0x2000000 + (0x555 << 1));
    writeWord_N64(0x90);

    // Read manufacturer ID
    setAddress_N64(romBase + 0x2000000);
    readWord_N64();
    // Read flashrom ID
    sprintf(cartID, "%04X", readWord_N64());
    if (strcmp(cartID, "8813") == 0) {
      cartSize = 64;
      flashid = 0x8813;
      strncpy(flashid_str, cartID, 5);
    }
    resetIntel4400_N64();
    // Empty cartID string
    cartID[0] = '\0';
  }

  //Fujitsu MSP55LV512/Spansion S29GL512N (64MB)
  else if ((strcmp(cartID, "2301") == 0) && (flashid == 0x227E)) {
    cartSize = 64;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // Spansion S29GL128N(16MB) with one flashrom chip
  else if ((strcmp(cartID, "2101") == 0) && (flashid == 0x227E)) {
    cartSize = 16;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // ST M29W128GL(16MB) with one flashrom chip
  else if ((strcmp(cartID, "2100") == 0) && (flashid == 0x227E)) {
    cartSize = 16;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // Intel 512M29EW(64MB) with one flashrom chip
  else if ((strcmp(cartID, "3901") == 0) && (flashid == 0x227E)) {
    cartSize = 64;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  // Unknown 227E type
  else if (flashid == 0x227E) {
    cartSize = 0;
    // Reset flashrom
    resetFlashrom_N64(romBase);
  }

  //Test for Fujitsu MSP55LV100S (64MB)
  else {
    // Send flashrom ID command
    setAddress_N64(romBase + (0x555 << 1));
    writeWord_N64(0xAAAA);
    setAddress_N64(romBase + (0x2AA << 1));
    writeWord_N64(0x5555);
    setAddress_N64(romBase + (0x555 << 1));
    writeWord_N64(0x9090);

    setAddress_N64(romBase);
    // Read 1 byte vendor ID
    readWord_N64();
    // Read 2 bytes flashrom ID
    sprintf(cartID, "%04X", readWord_N64());

    if (strcmp(cartID, "7E7E") == 0) {
      resetMSP55LV100_N64(romBase);
      cartSize = 64;
      flashid = 0x7E7E;
      strncpy(flashid_str, cartID, 5);
    }
  }
  if ((flashid == 0x1240) && (strcmp(cartID, "1240") == 0)) {
    print_FatalError(F("Please reseat cartridge"));
  }
}

// Erase Intel flashrom
void eraseIntel4400_N64() {
  unsigned long flashBase = romBase;

  print_Msg(F("Erasing..."));
  display_Update();

  // If the game is smaller than 32Mbit only erase the needed blocks
  unsigned long lastBlock = 0x1FFFFFF;
  if (fileSize < 0x1FFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock block command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x60);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);
    // Erase command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x20);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);

    // Read the status register
    setAddress_N64(flashBase + currBlock);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(flashBase + currBlock);
      statusReg = readWord_N64();
    }
  }

  // Erase up to 255 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
    // Unlock block command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x60);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);
    // Erase command
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0x20);
    setAddress_N64(flashBase + currBlock);
    writeWord_N64(0xD0);

    // Read the status register
    setAddress_N64(flashBase + currBlock);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(flashBase + currBlock);
      statusReg = readWord_N64();
    }

    // Blink led
    blinkLED();
  }

  // Check if we should erase the second chip too
  if ((cartSize = 64) && (fileSize > 0x2000000)) {
    // Switch base address to second chip
    flashBase = romBase + 0x2000000;

    // 255 blocks with 64kwords each
    for (unsigned long currBlock = 0x0; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF) {
      // Unlock block command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x60);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);
      // Erase command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x20);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);

      // Read the status register
      setAddress_N64(flashBase + currBlock);
      word statusReg = readWord_N64();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress_N64(flashBase + currBlock);
        statusReg = readWord_N64();
      }

      // Blink led
      blinkLED();
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000) {
      // Unlock block command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x60);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);
      // Erase command
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0x20);
      setAddress_N64(flashBase + currBlock);
      writeWord_N64(0xD0);

      // Read the status register
      setAddress_N64(flashBase + currBlock);
      word statusReg = readWord_N64();
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        setAddress_N64(flashBase + currBlock);
        statusReg = readWord_N64();
      }
    }
  }
}

// Erase Fujutsu MSP55LV100S
void eraseMSP55LV100_N64() {
  unsigned long flashBase = romBase;
  unsigned long sectorSize = 0x20000;

  print_Msg(F("Erasing..."));
  display_Update();

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    blinkLED();

    // Send Erase Command to first chip
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAAAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x5555);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0x8080);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAAAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x5555);
    setAddress_N64(romBase + currSector);
    writeWord_N64(0x3030);

    // Read the status register
    setAddress_N64(romBase + currSector);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(romBase + currSector);
      statusReg = readWord_N64();
    }

    // Read the status register
    setAddress_N64(romBase + currSector);
    statusReg = readWord_N64();
    while ((statusReg | 0x7FFF) != 0xFFFF) {
      setAddress_N64(romBase + currSector);
      statusReg = readWord_N64();
    }
  }
}

// Common chip erase command
void eraseFlashrom_N64() {
  print_Msg(F("Chip erase..."));
  display_Update();

  // Send Erase Command
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0xAA);
  setAddress_N64(romBase + (0x2AA << 1));
  writeWord_N64(0x55);
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0x80);
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0xAA);
  setAddress_N64(romBase + (0x2AA << 1));
  writeWord_N64(0x55);
  setAddress_N64(romBase + (0x555 << 1));
  writeWord_N64(0x10);

  // Read the status register
  setAddress_N64(romBase);
  word statusReg = readWord_N64();
  while ((statusReg | 0xFF7F) != 0xFFFF) {
    setAddress_N64(romBase);
    statusReg = readWord_N64();
    // Blink led
    blinkLED();
    delay(500);
  }
}

// Common sector erase command
void eraseSector_N64(unsigned long sectorSize) {
  unsigned long flashBase = romBase;

  print_Msg(F("Sector erase..."));
  display_Update();

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    blinkLED();

    // Spansion S29GL256N(32MB/64MB) with two flashrom chips
    if ((currSector == 0x2000000) && (strcmp(cartID, "2201") == 0) && (flashid == 0x227E)) {
      // Change to second chip
      flashBase = romBase + 0x2000000;
    }
    // Macronix MX29LV640(8MB/16MB) with two flashrom chips
    else if ((currSector == 0x800000) && ((flashid == 0x22C9) || (flashid == 0x22CB))) {
      flashBase = romBase + 0x800000;
    }

    // Send Erase Command
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0x80);
    setAddress_N64(flashBase + (0x555 << 1));
    writeWord_N64(0xAA);
    setAddress_N64(flashBase + (0x2AA << 1));
    writeWord_N64(0x55);
    setAddress_N64(romBase + currSector);
    writeWord_N64(0x30);

    // Read the status register
    setAddress_N64(romBase + currSector);
    word statusReg = readWord_N64();
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      setAddress_N64(romBase + currSector);
      statusReg = readWord_N64();
    }
  }
}

boolean blankcheckFlashrom_N64() {
  for (unsigned long currByte = romBase; currByte < romBase + fileSize; currByte += 512) {
    // Blink led
    if (currByte % 131072 == 0)
      blinkLED();

    // Set the address
    setAddress_N64(currByte);

    for (int c = 0; c < 512; c += 2) {
      if (readWord_N64() != 0xFFFF) {
        return 0;
      }
    }
  }
  return 1;
}

// Write Intel flashrom
void writeIntel4400_N64() {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) {
    // Blink led
    blinkLED();

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Buffered program command
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0xE8);

        // Check Status register
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        word statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
          statusReg = readWord_N64();
        }

        // Write word count (minus 1)
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x1F);

        // Write buffer
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          word currWord = ((sdBuffer[currWriteBuffer + currByte] & 0xFF) << 8) | (sdBuffer[currWriteBuffer + currByte + 1] & 0xFF);
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + currByte);
          writeWord_N64(currWord);
        }

        // Write Buffer to Flash
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 62);
        writeWord_N64(0xD0);

        // Read the status register at last written address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 62);
        statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 62);
          statusReg = readWord_N64();
        }
      }
    }
  }
}
// Write Fujitsu MSP55LV100S flashrom consisting out of two MSP55LV512 flashroms one used for the high byte the other for the low byte
void writeMSP55LV100_N64(unsigned long sectorSize) {
  unsigned long flashBase = romBase;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    blinkLED();

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);

      // Write 32 bytes at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32) {

        // 2 unlock commands
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xAAAA);
        setAddress_N64(flashBase + (0x2AA << 1));
        writeWord_N64(0x5555);

        // Write buffer load command at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x2525);
        // Write word count (minus 1) at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x0F0F);

        // Define variable before loop so we can use it later when reading the status register
        word currWord;

        for (byte currByte = 0; currByte < 32; currByte += 2) {
          // Join two bytes into one word
          currWord = ((sdBuffer[currWriteBuffer + currByte] & 0xFF) << 8) | (sdBuffer[currWriteBuffer + currByte + 1] & 0xFF);

          // Load Buffer Words
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + currByte);
          writeWord_N64(currWord);
        }

        // Write Buffer to Flash
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 30);
        writeWord_N64(0x2929);

        // Read the status register at last written address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 30);
        word statusReg = readWord_N64();
        while ((statusReg | 0x7F7F) != (currWord | 0x7F7F)) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + 30);
          statusReg = readWord_N64();
        }
      }
    }
  }
}

// Write Spansion S29GL256N flashrom using the 32 byte write buffer
void writeFlashBuffer_N64(unsigned long sectorSize, byte bufferSize) {
  unsigned long flashBase = romBase;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    blinkLED();

    // Spansion S29GL256N(32MB/64MB) with two flashrom chips
    if ((currSector == 0x2000000) && (strcmp(cartID, "2201") == 0)) {
      flashBase = romBase + 0x2000000;
    }

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);

      // Write 32 bytes at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += bufferSize) {

        // 2 unlock commands
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xAA);
        setAddress_N64(flashBase + (0x2AA << 1));
        writeWord_N64(0x55);

        // Write buffer load command at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64(0x25);
        // Write word count (minus 1) at sector address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer);
        writeWord_N64((bufferSize / 2) - 1);

        // Define variable before loop so we can use it later when reading the status register
        word currWord = 0;

        for (byte currByte = 0; currByte < bufferSize; currByte += 2) {
          // Join two bytes into one word
          currWord = ((sdBuffer[currWriteBuffer + currByte] & 0xFF) << 8) | (sdBuffer[currWriteBuffer + currByte + 1] & 0xFF);

          // Load Buffer Words
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + currByte);
          writeWord_N64(currWord);
        }

        // Write Buffer to Flash
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + bufferSize - 2);
        writeWord_N64(0x29);

        // Read the status register at last written address
        setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + bufferSize - 2);
        word statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          setAddress_N64(romBase + currSector + currSdBuffer + currWriteBuffer + bufferSize - 2);
          statusReg = readWord_N64();
        }
      }
    }
  }
}

// Write MX29LV640 flashrom without write buffer
void writeFlashrom_N64(unsigned long sectorSize) {
  unsigned long flashBase = romBase;

  for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
    // Blink led
    blinkLED();

    // Macronix MX29LV640(8MB/16MB) with two flashrom chips
    if (currSector == 0x800000) {
      flashBase = romBase + 0x800000;
    }

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);
      for (int currByte = 0; currByte < 512; currByte += 2) {
        // Join two bytes into one word
        word currWord = ((sdBuffer[currByte] & 0xFF) << 8) | (sdBuffer[currByte + 1] & 0xFF);
        // 2 unlock commands
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xAA);
        setAddress_N64(flashBase + (0x2AA << 1));
        writeWord_N64(0x55);
        // Program command
        setAddress_N64(flashBase + (0x555 << 1));
        writeWord_N64(0xA0);
        // Write word
        setAddress_N64(romBase + currSector + currSdBuffer + currByte);
        writeWord_N64(currWord);

        // Read the status register
        setAddress_N64(romBase + currSector + currSdBuffer + currByte);
        word statusReg = readWord_N64();
        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          setAddress_N64(romBase + currSector + currSdBuffer + currByte);
          statusReg = readWord_N64();
        }
      }
    }
  }
}

unsigned long verifyFlashrom_N64() {
  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    writeErrors = 0;

    for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) {
      // Blink led
      blinkLED();
      for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) {
        // Fill SD buffer
        myFile.read(sdBuffer, 512);
        for (int currByte = 0; currByte < 512; currByte += 2) {
          // Join two bytes into one word
          word currWord = ((sdBuffer[currByte] & 0xFF) << 8) | (sdBuffer[currByte + 1] & 0xFF);
          // Read flash
          setAddress_N64(romBase + currSector + currSdBuffer + currByte);
          // Compare both
          if (readWord_N64() != currWord) {
            writeErrors++;
            // Abord if too many errors
            if (writeErrors > 20) {
              print_Msg(F("More than "));
              // Close the file:
              myFile.close();
              return writeErrors;
            }
          }
        }
      }
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
    return 9999;
  }
}

/******************************************
   N64 Gameshark Flash Functions
 *****************************************/
void flashGameshark_N64() {
  // Check flashrom ID's
  idGameshark_N64();

  // Check for SST 29LE010 (0808)/SST 28LF040 (0404)/AMTEL AT29LV010A (3535)/SST 29EE010 (0707)
  // !!!!          This has been confirmed to allow reading of v1.07, v1.09, v2.0-2.21, v3.2-3.3           !!!!
  // !!!!   29LE010/29EE010/AT29LV010A are very similar and can possibly be written to with this process.  !!!!
  // !!!!                                                                                                  !!!!
  // !!!!                                PROCEED AT YOUR OWN RISK                                          !!!!
  // !!!!                                                                                                  !!!!
  // !!!! SST 29EE010 may have a 5V requirement for writing however dumping works at 3V. As such it is not !!!!
  // !!!!        advised to write to a cart with this chip until further testing can be completed.         !!!!

  if (flashid == 0x0808 || flashid == 0x0404 || flashid == 0x3535 || flashid == 0x0707) {
    backupGameshark_N64();
    println_Msg("");
    println_Msg(F("This will erase your"));
    println_Msg(F("Gameshark cartridge"));
    println_Msg(F("Attention: Use 3.3V!"));
    println_Msg(F("Power OFF if Unsure!"));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();

    // Launch file browser
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser(F("Select z64 file"));
    display_Clear();
    display_Update();

    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {
      // Get rom size from file
      fileSize = myFile.fileSize();
      print_Msg(F("File size: "));
      print_Msg(fileSize / 1024);
      println_Msg(F("KB"));
      display_Update();

      // Compare file size to flashrom size
      if (fileSize > 262144) {
        print_FatalError(file_too_big_STR);
      }

      // SST 29LE010, chip erase not needed as this eeprom automaticly erases during the write cycle
      eraseGameshark_N64();

      // Write flashrom
      print_Msg(F("Writing "));
      println_Msg(filePath);
      display_Update();
      writeGameshark_N64();

      // Close the file:
      myFile.close();

      // Verify
      print_STR(verifying_STR, 0);
      display_Update();
      writeErrors = verifyGameshark_N64();

      if (writeErrors == 0) {
        println_Msg(F("OK"));
        println_Msg(F(""));
        println_Msg(F("Turn Cart Reader off now"));
        display_Update();
        while (1)
          ;
      } else {
        print_Msg(writeErrors);
        print_Msg(F(" bytes "));
        print_Error(did_not_verify_STR);
      }
    } else {
      print_Error(F("Can't open file"));
    }
  }
  // If the ID is unknown show error message
  else {
    print_Msg(F("ID: "));
    println_Msg(flashid_str);
    print_Error(F("Unknown flashrom"));
  }

  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
  display_Clear();
  display_Update();
}


//Test for SST 29LE010  or SST 28LF040 (0404) or AMTEL AT29LV010A (3535) or SST 29EE010 (0707)
void idGameshark_N64() {
  //Send flashrom ID command
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0x9090);

  setAddress_N64(romBase);
  // Read 1 byte vendor ID
  readWord_N64();
  // Read 2 bytes flashrom ID
  flashid = readWord_N64();
  sprintf(flashid_str, "%04X", flashid);
  // Reset flashrom
  resetGameshark_N64();
}

//Reset ST29LE010
void resetGameshark_N64() {
  // Send reset Command
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xF0F0);
  delay(100);
}

// Read rom and save to the SD card
void backupGameshark_N64() {
  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "GS%d", foldern);
  strcat(fileName, ".z64");
  sd.mkdir("N64/ROM/Gameshark", true);
  sd.chdir("N64/ROM/Gameshark");

  display_Clear();
  print_Msg(F("Saving "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  for (unsigned long currByte = romBase + 0xC00000; currByte < (romBase + 0xC00000 + 262144); currByte += 512) {
    // Blink led
    if (currByte % 16384 == 0)
      blinkLED();

    // Set the address for the next 512 bytes
    setAddress_N64(currByte);

    for (int c = 0; c < 512; c += 2) {
      // split word
      word myWord = readWord_N64();
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    myFile.write(sdBuffer, 512);
  }
  // Close the file:
  myFile.close();
}

// Send chip erase to the two SST29LE010 inside the Gameshark
void eraseGameshark_N64() {
  println_Msg(F("Erasing..."));
  display_Update();

  //Sending erase command according to datasheet
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0x8080);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0xAAAA);
  setAddress_N64(romBase + 0x5554);
  writeWord_N64(0x5555);
  setAddress_N64(romBase + 0xAAAA);
  writeWord_N64(0x1010);

  delay(20);
}

// Write Gameshark with 2x SST29LE010 Eeproms
void writeGameshark_N64() {
  // Each 29LE010 has 1024 pages, each 128 bytes in size
  for (unsigned long currPage = 0; currPage < fileSize / 2; currPage += 128) {
    // Fill SD buffer with twice the amount since we flash 2 chips
    myFile.read(sdBuffer, 256);
    // Blink led
    blinkLED();

    //Send page write command to both flashroms
    setAddress_N64(romBase + 0xAAAA);
    writeWord_N64(0xAAAA);
    setAddress_N64(romBase + 0x5554);
    writeWord_N64(0x5555);
    setAddress_N64(romBase + 0xAAAA);
    writeWord_N64(0xA0A0);

    // Write 1 page each, one flashrom gets the low byte, the other the high byte.
    for (unsigned long currByte = 0; currByte < 256; currByte += 2) {
      // Set address
      setAddress_N64(romBase + 0xC00000 + (currPage * 2) + currByte);
      // Join two bytes into one word
      word currWord = ((sdBuffer[currByte] & 0xFF) << 8) | (sdBuffer[currByte + 1] & 0xFF);
      // Send byte data
      writeWord_N64(currWord);
    }
    delay(30);
  }
}

unsigned long verifyGameshark_N64() {
  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    writeErrors = 0;

    for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) {
      // Blink led
      blinkLED();
      for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) {
        // Fill SD buffer
        myFile.read(sdBuffer, 512);
        for (int currByte = 0; currByte < 512; currByte += 2) {
          // Join two bytes into one word
          word currWord = ((sdBuffer[currByte] & 0xFF) << 8) | (sdBuffer[currByte + 1] & 0xFF);
          // Read flash
          setAddress_N64(romBase + 0xC00000 + currSector + currSdBuffer + currByte);
          // Compare both
          if (readWord_N64() != currWord) {
            if ((flashid == 0x0808) && (currSector + currSdBuffer + currByte > 0x3F) && (currSector + currSdBuffer + currByte < 0x1080)) {
              // Gameshark maps this area to the bootcode of the plugged in cartridge
            } else {
              writeErrors++;
            }
          }
        }
      }
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
    return 9999;
  }
}

#endif

//******************************************
// End of File
//******************************************