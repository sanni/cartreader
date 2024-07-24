//******************************************
// PC Engine & TurboGrafx dump code by tamanegi_taro
// April 18th 2018 Revision 1.0.1 Initial version
// August 12th 2019 Revision 1.0.2 Added Tennokoe Bank support
// June 29th 2024 Revision 1.0.3 Added repro HuCard flashing
//
// Special thanks
// sanni - Arduino cart reader
// skaman - ROM size detection
// NO-INTRO - CRC list for game name detection
// Chris Covell - Tennokoe bank support
// partlyhuman - repro HuCard support
//
//******************************************
#ifdef ENABLE_PCE

/******************************************
  Defines
 *****************************************/
#define DETECTION_SIZE 64
#define FORCED_SIZE 1024
#define CHKSUM_SKIP 0
#define CHKSUM_OK 1
#define CHKSUM_ERROR 2
enum PCE_MODE_T : uint8_t { HUCARD,
                            TURBOCHIP,
                            HUCARD_NOSWAP,
                            PCE_FLASH };

/******************************************
   Prototype Declarations
 *****************************************/
/* Several PCE dedicated functions */
void pin_read_write_PCE(void);
void pin_init_PCE(void);
void setup_cart_PCE(void);
void reset_cart_PCE(void);
uint8_t read_byte_PCE(uint32_t address);
void write_byte_PCE(uint32_t address, uint8_t data);
uint32_t detect_rom_size_PCE(void);
void read_bank_PCE_ROM(uint32_t address_start, uint32_t address_end, uint32_t *processed_size, uint32_t total_size);
void read_bank_PCE_RAM(uint32_t address_start);
void read_rom_PCE(void);

/******************************************
   Variables
 *****************************************/
uint8_t pce_internal_mode;  //0 - HuCARD, 1 - TurboChip
uint16_t pce_force_rom_size = 0;
uint8_t tennokoe_bank_index = 0;

/******************************************
  Menu
*****************************************/
// PCE start menu
static const char pceMenuItem1[] PROGMEM = "HuCARD(swapped)";
static const char pceMenuItem2[] PROGMEM = "HuCARD(not swapped)";
static const char pceMenuItem3[] PROGMEM = "Turbochip";
static const char pceMenuItem4[] PROGMEM = "Flash Repro HuCARD";
static const char *const menuOptionspce[] PROGMEM = { pceMenuItem1, pceMenuItem2, pceMenuItem3, pceMenuItem4, FSTRING_RESET };

// PCE card menu items
static const char menuOptionspceCart_1[] PROGMEM = "Read RAM Bank %d";
static const char menuOptionspceCart_2[] PROGMEM = "Write RAM Bank %d";
static const char menuOptionspceCart_3[] PROGMEM = "Inc Bank Number";
static const char menuOptionspceCart_4[] PROGMEM = "Dec Bank Number";
static const char menuOptionspceCart_5[] PROGMEM = "Set %dK ROM size";
static const char menuOptionspceCart_5_fmt[] PROGMEM = "ROM size now %dK";

// Turbochip menu items
static const char *const menuOptionspceTC[] PROGMEM = { FSTRING_READ_ROM, FSTRING_RESET };

#ifdef ENABLE_FLASH
// Flash repro menu items
// static const char menuOptionspceFlash1[] PROGMEM = "Program";
static const char *const menuOptionspceFlash[] PROGMEM = { flashMenuItemWrite, FSTRING_RESET };
#endif

// PCE start menu, first a device type is selected and set in pce_internal_mode
void pcsMenu(void) {
  unsigned char pceDev;
  convertPgm(menuOptionspce, 5);
  pceDev = question_box(F("Select device"), menuOptions, 5, 0);

  switch (pceDev) {
    case 0:
      //Hucard
      display_Clear();
      display_Update();
      pce_internal_mode = HUCARD;
      setup_cart_PCE();
      mode = CORE_PCE;
      break;

    case 1:
      //Hucard not swapped
      display_Clear();
      display_Update();
      pce_internal_mode = HUCARD_NOSWAP;
      setup_cart_PCE();
      mode = CORE_PCE;
      break;

    case 2:
      //Turbografx
      display_Clear();
      display_Update();
      pce_internal_mode = TURBOCHIP;
      setup_cart_PCE();
      mode = CORE_PCE;
      break;

#ifdef ENABLE_FLASH
    case 3:
      //Flash Repro
      display_Clear();
      display_Update();
      pce_internal_mode = PCE_FLASH;
      setup_cart_PCE();
      mode = CORE_PCE;
      break;
#endif

    case 4:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

void pin_read_write_PCE(void) {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A19
  DDRL = (DDRL & 0xF0) | 0x0F;

  //Set Control Pin to Output CS(PL4)
  DDRL |= (1 << 4);

  //Set CS(PL4) to HIGH
  PORTL |= (1 << 4);

  // Set Control Pins to Output RST(PH0) RD(PH3) WR(PH5)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5);
  // Switch all of above to HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  // Enable Internal Pullups
  PORTC = 0xFF;

  set_cs_rd_low_PCE();

  reset_cart_PCE();
}

void pin_init_PCE(void) {
  //Set Address Pins to input and pull up
  DDRF = 0x00;
  PORTF = 0xFF;
  DDRK = 0x00;
  PORTK = 0xFF;
  DDRL = 0x00;
  PORTL = 0xFF;
  DDRH &= ~((1 << 0) | (1 << 3) | (1 << 5) | (1 << 6));
  PORTH = (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  PORTC = 0xFF;
}

void setup_cart_PCE(void) {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high to disable CIC
  PORTG |= (1 << 1);

  pin_init_PCE();
}

void reset_cart_PCE(void) {
  //Set RESET as Low
  PORTH &= ~(1 << 0);
  delay(200);
  //Set RESET as High
  PORTH |= (1 << 0);
  delay(200);
}

void set_address_PCE(uint32_t address) {
  //Set address
  PORTF = address & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  PORTL = (PORTL & 0xF0) | ((address >> 16) & 0x0F);
}

void set_cs_rd_low_PCE() {
  // Set CS(PL4) and RD(PH3) as LOW
  PORTL &= ~(1 << 4);
  PORTH &= ~(1 << 3);
}

uint8_t read_byte_PCE(uint32_t address) {
  uint8_t ret;

  set_address_PCE(address);

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

  //read byte
  ret = PINC;

  //Swap bit order for PC Engine HuCARD
  if (pce_internal_mode == HUCARD || pce_internal_mode == PCE_FLASH) {
    ret = ((ret & 0x01) << 7) | ((ret & 0x02) << 5) | ((ret & 0x04) << 3) | ((ret & 0x08) << 1) | ((ret & 0x10) >> 1) | ((ret & 0x20) >> 3) | ((ret & 0x40) >> 5) | ((ret & 0x80) >> 7);
  }

  //return read data
  return ret;
}

void data_output_PCE() {
  // Set Data Pins (D0-D7) to Output
  DDRC = 0xFF;
}

void data_input_PCE() {
  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  PORTC = 0xFF;

  set_cs_rd_low_PCE();
}

void write_byte_PCE(uint32_t address, uint8_t data) {
  //PORTH |= (1 << 3); // RD HIGH
  set_address_PCE(address);

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

  //Swap bit order for PC Engine HuCARD
  if (pce_internal_mode == HUCARD || pce_internal_mode == PCE_FLASH) {
    data = ((data & 0x01) << 7) | ((data & 0x02) << 5) | ((data & 0x04) << 3) | ((data & 0x08) << 1) | ((data & 0x10) >> 1) | ((data & 0x20) >> 3) | ((data & 0x40) >> 5) | ((data & 0x80) >> 7);
  }

  //write byte
  PORTC = data;

  // Set CS(PL4) and WR(PH5) as LOW
  PORTL &= ~(1 << 4);
  PORTH &= ~(1 << 5);

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

  // Set CS(PL4) and WR(PH5) as HIGH
  PORTL |= (1 << 4);
  PORTH |= (1 << 5);
}

//Confirm the size of ROM
uint32_t detect_rom_size_PCE(void) {
  uint32_t rom_size;
  uint8_t read_byte;
  uint8_t current_byte;
  uint8_t detect_16, detect_32, detect_128, detect_256, detect_512, detect_768;

  //Initialize variables
  detect_16 = 0;
  detect_32 = 0;
  detect_128 = 0;
  detect_256 = 0;
  detect_512 = 0;
  detect_768 = 0;

  //Set pins to read PC Engine cart
  pin_read_write_PCE();

  //Confirm where mirror address start from (16KB, 32KB, 128KB, 256KB, 512KB, 768KB, or 1024KB)
  for (current_byte = 0; current_byte < DETECTION_SIZE; current_byte++) {
    if ((current_byte != detect_16) && (current_byte != detect_32) && (current_byte != detect_128) && (current_byte != detect_256) && (current_byte != detect_512) && (current_byte != detect_768)) {
      //If none matched, it is 1024KB
      break;
    }

    //read byte for 16KB, 32KB, 128KB, 256KB, 512KB detection
    read_byte = read_byte_PCE(current_byte);

    //16KB detection
    if (current_byte == detect_16) {
      if (read_byte_PCE(current_byte + 16UL * 1024UL) == read_byte) {
        detect_16++;
      }
    }

    //32KB detection
    if (current_byte == detect_32) {
      if (read_byte_PCE(current_byte + 32UL * 1024UL) == read_byte) {
        detect_32++;
      }
    }

    //128KB detection
    if (current_byte == detect_128) {
      if (read_byte_PCE(current_byte + 128UL * 1024UL) == read_byte) {
        detect_128++;
      }
    }

    //256KB detection
    if (current_byte == detect_256) {
      if (read_byte_PCE(current_byte + 256UL * 1024UL) == read_byte) {
        detect_256++;
      }
    }

    //512KB detection
    if (current_byte == detect_512) {
      if (read_byte_PCE(current_byte + 512UL * 1024UL) == read_byte) {
        detect_512++;
      }
    }

    //768KB detection
    read_byte = read_byte_PCE(current_byte + 512UL * 1024UL);
    if (current_byte == detect_768) {
      if (read_byte_PCE(current_byte + 768UL * 1024UL) == read_byte) {
        detect_768++;
      }
    }
  }

  //debug
  //sprintf(fileName, "%d %d %d %d %d %d", detect_16, detect_32, detect_128, detect_256, detect_512, detect_768); //using filename global variable as string. Initialzed in below anyways.
  //println_Msg(fileName);

  //ROM size detection by result
  if (detect_16 == DETECTION_SIZE) {
    rom_size = 16;
  } else if (detect_32 == DETECTION_SIZE) {
    rom_size = 32;
  } else if (detect_128 == DETECTION_SIZE) {
    rom_size = 128;
  } else if (detect_256 == DETECTION_SIZE) {
    if (detect_512 == DETECTION_SIZE) {
      rom_size = 256;
    } else {
      //rom_size = 1024;
      //Another confirmation for 384KB because 384KB hucard has data in 0x0--0x40000 and 0x80000--0xA0000(0x40000 is mirror of 0x00000)
      rom_size = 384;
    }
  } else if (detect_512 == DETECTION_SIZE) {
    rom_size = 512;
  } else if (detect_768 == DETECTION_SIZE) {
    rom_size = 768;
  } else {
    rom_size = 1024;
  }

  //If rom size is more than or equal to 512KB, detect special cards
  if (rom_size >= 512) {
    //Street Fighter II' - Champion Edition (Japan)
    if (read_byte_PCE(0x7FFF9) == 'N' && read_byte_PCE(0x7FFFA) == 'E' && read_byte_PCE(0x7FFFB) == 'C' && read_byte_PCE(0x7FFFC) == ' ' && read_byte_PCE(0x7FFFD) == 'H' && read_byte_PCE(0x7FFFE) == 'E') {
      rom_size = 2560;
    }
    //Populous (Japan)
    if (read_byte_PCE(0x1F26) == 'P' && read_byte_PCE(0x1F27) == 'O' && read_byte_PCE(0x1F28) == 'P' && read_byte_PCE(0x1F29) == 'U' && read_byte_PCE(0x1F2A) == 'L' && read_byte_PCE(0x1F2B) == 'O' && read_byte_PCE(0x1F2C) == 'U' && read_byte_PCE(0x1F2D) == 'S') {
      rom_size = 512;
    }
    //Dinoforce (Japan)
    if (read_byte_PCE(0x15A) == 'D' && read_byte_PCE(0x15B) == 'I' && read_byte_PCE(0x15C) == 'N' && read_byte_PCE(0x15D) == 'O' && read_byte_PCE(0x15E) == '-' && read_byte_PCE(0x15F) == 'F' && read_byte_PCE(0x160) == 'O' && read_byte_PCE(0x161) == 'R' && read_byte_PCE(0x162) == 'C' && read_byte_PCE(0x163) == 'E') {
      rom_size = 512;
    }
  }
  if (rom_size == 384) {
    //"CD-ROM² Super System Card (v3.0)(Japan)" or "Arcade Card Pro CD-ROM²"
    if (read_byte_PCE(0x29D1) == 'V' && read_byte_PCE(0x29D2) == 'E' && read_byte_PCE(0x29D3) == 'R' && read_byte_PCE(0x29D4) == '.' && read_byte_PCE(0x29D5) == ' ' && read_byte_PCE(0x29D6) == '3' && read_byte_PCE(0x29D7) == '.' && read_byte_PCE(0x29D8) == '0' && read_byte_PCE(0x29D9) == '0') {
      rom_size = 256;
    }
  }

  return rom_size;
}

/* Must be address_start and address_end should be 512 byte aligned */
void read_bank_PCE_ROM(uint32_t address_start, uint32_t address_end, uint32_t *processed_size, uint32_t total_size, uint32_t *crcp) {
  uint32_t currByte;
  uint16_t c;

  for (currByte = address_start; currByte < address_end; currByte += 512) {
    for (c = 0; c < 512; c++) {
      sdBuffer[c] = read_byte_PCE(currByte + c);
    }
    if (crcp != NULL) {
      *crcp = calculate_crc32(512, sdBuffer, *crcp);
    }
    myFile.write(sdBuffer, 512);
    *processed_size += 512;
    draw_progressbar(*processed_size, total_size);
  }
}

void read_bank_PCE_RAM(uint32_t address_start, int block_index) {
  uint32_t start = address_start + block_index * 512;
  for (uint16_t c = 0; c < 512; c++) {
    sdBuffer[c] = read_byte_PCE(start + c);
  }
}

uint32_t calculate_crc32(int n, unsigned char c[], uint32_t r) {
  int i, j;

  for (i = 0; i < n; i++) {
    r ^= c[i];
    for (j = 0; j < 8; j++)
      if (r & 1) r = (r >> 1) ^ 0xEDB88320UL;
      else r >>= 1;
  }
  return r;
}

void crc_search(char *file_p, char *folder_p, uint32_t rom_size __attribute__((unused)), uint32_t crc) {
  FsFile rom, script;
  char gamename[100];
  char crc_file[9], crc_search[9];
  uint8_t flag;
  flag = CHKSUM_SKIP;

  //Open list file. If no list file found, just skip
  sd.chdir("/");  //Set read directry to root
  if (script.open("pce.txt", O_READ)) {
    //Calculate CRC of ROM file
    sd.chdir(folder_p);
    if (rom.open(file_p, O_READ)) {
      //Initialize flag as error
      flag = CHKSUM_ERROR;
      crc = crc ^ 0xFFFFFFFFUL;  //Finish CRC calculation and progress bar
      //Display calculated CRC
      sprintf(crc_file, "%08lX", crc);

      //Search for same CRC in list
      while (script.available()) {
        //Read 2 lines (game name and CRC)
        get_line(gamename, &script, 96);
        get_line(crc_search, &script, 9);
        skip_line(&script);  //Skip every 3rd line

        //if checksum search successful, rename the file and end search
        if (strcmp(crc_search, crc_file) == 0) {
          print_Msg(F("Chksum OK "));
          println_Msg(crc_file);
          print_Msg(F("Saved to "));
          print_Msg(folder_p);
          print_Msg(F("/"));
          print_Msg(gamename);
          flag = CHKSUM_OK;
          rom.rename(gamename);
          break;
        }
      }
      rom.close();
    }
  }

  if (flag == CHKSUM_SKIP) {
    print_Msg(F("Saved to "));
    print_Msg(folder_p);
    print_Msg(F("/"));
    print_Msg(file_p);
  } else if (flag == CHKSUM_ERROR) {
    print_Msg(F("Chksum Error "));
    println_Msg(crc_file);
    print_Msg(F("Saved to "));
    print_Msg(folder_p);
    print_Msg(F("/"));
    print_Msg(file_p);
  }

  script.close();
}

void unlock_tennokoe_bank_RAM() {
  write_byte_PCE(0x0D0000, 0x68);  //Unlock RAM sequence 1 Bank 68
  write_byte_PCE(0x0F0000, 0x00);  //Unlock RAM sequence 2 Bank 78
  write_byte_PCE(0x0F0000, 0x73);  //Unlock RAM sequence 3 Bank 78
  write_byte_PCE(0x0F0000, 0x73);  //Unlock RAM sequence 4 Bank 78
  write_byte_PCE(0x0F0000, 0x73);  //Unlock RAM sequence 5 Bank 78
}

void lock_tennokoe_bank_RAM() {
  write_byte_PCE(0x0D0000, 0x68);  //Lock RAM sequence 1 Bank 68
  write_byte_PCE(0x0F0001, 0x00);  //Lock RAM sequence 2 Bank 78
  write_byte_PCE(0x0C0001, 0x60);  //Lock RAM sequence 3 Bank 60
}

void read_tennokoe_bank_PCE(int bank_index) {
  //clear the screen
  display_Clear();

  println_Msg(F("RAM bank size: 2KB"));

  // Get name, add extension and convert to char array for sd lib
  sprintf(fileName, "BANKRAM%d.sav", bank_index + 1);

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sd.chdir("/");
  sprintf(folder, "PCE/RAM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  print_Msg(F("Saving RAM to "));
  print_Msg(folder);
  print_Msg(F("/"));
  println_Msg(fileName);
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }

  pin_read_write_PCE();

  for (int block_index = 0; block_index < 4; block_index++) {
    //Unlock Tennokoe Bank RAM
    //Disable interrupts
    noInterrupts();
    data_output_PCE();
    unlock_tennokoe_bank_RAM();
    data_input_PCE();

    //Read Tennokoe bank RAM
    read_bank_PCE_RAM(0x080000 + 2048UL * bank_index, block_index);

    //Lock Tennokoe Bank RAM
    data_output_PCE();
    lock_tennokoe_bank_RAM();
    data_input_PCE();
    //Enable interrupts
    interrupts();

    // hexdump:
    // for (int c = 0; c < 512; c += 16) {
    //   for (int i = 0; i < 16; i++) {
    //     uint8_t b = sdBuffer[c + i];
    //     print_Msg_PaddedHexByte(b);
    //     //print_Msg(FS(FSTRING_SPACE));
    //   }
    //   println_Msg(FS(FSTRING_EMPTY));
    // }

    if (block_index == 0) {
      print_Msg(F("header: "));
      for (int i = 0; i < 4; i++) {
        uint8_t b = sdBuffer[i];
        print_Msg_PaddedHexByte(b);
      }
      println_Msg(FS(FSTRING_EMPTY));
    }
    if (block_index == 0 && sdBuffer[2] == 0x42 && sdBuffer[3] == 0x4D) {
      if (sdBuffer[0] != 0x48 || sdBuffer[1] != 0x55) {
        sdBuffer[0] = 0x48;  // H
        sdBuffer[1] = 0x55;  // U
        println_Msg(F("Corrected header"));
      } else {
        println_Msg(F("Header is correct"));
      }
    }
    myFile.write(sdBuffer, 512);
  }

  pin_init_PCE();

  //Close the file:
  myFile.close();

  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void write_tennokoe_bank_PCE(int bank_index) {
  //Display file Browser and wait user to select a file. Size must be 2KB.
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select RAM file"));
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  display_Clear();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    fileSize = myFile.fileSize();
    if (fileSize != 2 * 1024UL) {
      println_Msg(F("File must be 2KB"));
      display_Update();
      myFile.close();
      wait();
      return;
    }

    pin_read_write_PCE();

    for (int block_index = 0; block_index < 4; block_index++) {
      for (uint16_t c = 0; c < 512; c++) {
        sdBuffer[c] = myFile.read();
      }
      //Unlock Tennokoe Bank RAM
      //Disable interrupts
      noInterrupts();
      data_output_PCE();
      unlock_tennokoe_bank_RAM();
      data_input_PCE();

      //Write file to Tennokoe BANK RAM
      data_output_PCE();
      uint32_t offset = 0x080000 + (bank_index * 2048UL) + (block_index * 512UL);
      for (uint16_t c = 0; c < 512; c++) {
        write_byte_PCE(offset + c, sdBuffer[c]);
      }

      //Lock Tennokoe Bank RAM
      lock_tennokoe_bank_RAM();
      data_input_PCE();
      //Enable interrupts
      interrupts();
    }

    // verify
    int diffcnt = 0;
    myFile.seekSet(0);
    for (int block_index = 0; block_index < 4; block_index++) {
      //Unlock Tennokoe Bank RAM
      //Disable interrupts
      noInterrupts();
      data_output_PCE();
      unlock_tennokoe_bank_RAM();
      data_input_PCE();
      //Read Tennokoe bank RAM
      read_bank_PCE_RAM(0x080000 + 2048UL * bank_index, block_index);
      //Lock Tennokoe Bank RAM
      data_output_PCE();
      lock_tennokoe_bank_RAM();
      data_input_PCE();
      //Enable interrupts
      interrupts();
      int diffcnt = 0;
      for (int c = 0; c < 512; c += 16) {
        uint8_t ram_b = sdBuffer[c];
        uint8_t file_b = myFile.read();
        if (ram_b != file_b) {
          diffcnt++;
        }
      }
    }
    if (diffcnt == 0) {
      println_Msg(F("Verify OK..."));
    } else {
      println_Msg(F("Verify failed..."));
      print_Msg(diffcnt);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }

    pin_init_PCE();

    // Close the file:
    myFile.close();
    println_Msg(F("Finished"));

  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }

  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void read_rom_PCE(void) {
  uint32_t rom_size;
  uint32_t processed_size = 0;

  //clear the screen
  display_Clear();
  
  rom_size = detect_rom_size_PCE();
  if (pce_force_rom_size > 0) {
    rom_size = pce_force_rom_size;
    print_Msg(F("Forced size: "));
  } else {
    print_Msg(F("Detected size: "));
  }
  print_Msg(rom_size);
  println_Msg(F("KB"));

  // Get name, add extension and convert to char array for sd lib
  sd.chdir("/");
  createFolder("PCE", "ROM", "PCEROM", "pce");
  printAndIncrementFolder();

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }

  pin_read_write_PCE();

  //Initialize progress bar by setting processed size as 0
  draw_progressbar(0, rom_size * 1024UL);

  uint32_t crc = 0xFFFFFFFFUL;  //Initialize CRC
  if (rom_size == 384) {
    //Read two sections. 0x000000--0x040000 and 0x080000--0x0A0000 for 384KB
    read_bank_PCE_ROM(0, 0x40000, &processed_size, rom_size * 1024UL, &crc);
    read_bank_PCE_ROM(0x80000, 0xA0000, &processed_size, rom_size * 1024UL, &crc);
  } else if (rom_size == 2560) {
    //Dump Street fighter II' Champion Edition
    read_bank_PCE_ROM(0, 0x80000, &processed_size, rom_size * 1024UL, &crc);  //Read first bank
    data_output_PCE();
    write_byte_PCE(0x1FF0, 0xFF);  //Display second bank
    data_input_PCE();
    read_bank_PCE_ROM(0x80000, 0x100000, &processed_size, rom_size * 1024UL, &crc);  //Read second bank
    data_output_PCE();
    write_byte_PCE(0x1FF1, 0xFF);  //Display third bank
    data_input_PCE();
    read_bank_PCE_ROM(0x80000, 0x100000, &processed_size, rom_size * 1024UL, &crc);  //Read third bank
    data_output_PCE();
    write_byte_PCE(0x1FF2, 0xFF);  //Display forth bank
    data_input_PCE();
    read_bank_PCE_ROM(0x80000, 0x100000, &processed_size, rom_size * 1024UL, &crc);  //Read forth bank
    data_output_PCE();
    write_byte_PCE(0x1FF3, 0xFF);  //Display fifth bank
    data_input_PCE();
    read_bank_PCE_ROM(0x80000, 0x100000, &processed_size, rom_size * 1024UL, &crc);  //Read fifth bank
  } else {
    //Read start form 0x000000 and keep reading until end of ROM
    read_bank_PCE_ROM(0, rom_size * 1024UL, &processed_size, rom_size * 1024UL, &crc);
  }

  pin_init_PCE();

  //Close the file:
  myFile.close();

  //CRC search and rename ROM
  compareCRC("pce.txt", 0, 1, 0);

  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, true);
  display_Update();
  wait();
}

#ifdef ENABLE_FLASH
void flash_mode_PCE() {
  pin_read_write_PCE();
  data_output_PCE();
  PORTH |= (1 << 3);  // RD HIGH
  // write_byte_PCE sets WR
}

// Implements data complement status checking
// We only look at D7, or the highest bit of expected
void flash_wait_status_PCE(uint8_t expected) {
  set_cs_rd_low_PCE();
  data_input_PCE();
  
  uint8_t status;
  do {
    PORTH &= ~(1 << 3);  // RD low
    // one nop = 62.5ns
    // tOE = 30-50ns depending on flash
    NOP;
    status = PINC;
    PORTH |= (1 << 3);  // RD high
    // reversed, bit 0 is the MSB
  } while ((status & 0x1) != (expected >> 7));

  data_output_PCE();
  // leave RD high on exit
}

// Flashes a reproduction HuCard that's directly wired to a flash chip
// Supported flash: SST39SF0x0, MX29F0x0 1Mbit-8Mbit
// Developed against Ichigobankai's design https://github.com/partlyhuman/HuCARD-repro
void flash_PCE() {
  println_Msg(F("Detecting..."));
  display_Update();

  // SOFTWARE ID PROGRAM
  flash_mode_PCE();
  write_byte_PCE(0x5555, 0xAA);
  write_byte_PCE(0x2AAA, 0x55);
  write_byte_PCE(0x5555, 0x90);
  data_input_PCE();
  // tIDA = 150ns
  NOP;NOP;NOP;
  // MFG,DEVICE
  uint16_t deviceId = (read_byte_PCE(0x0) << 8) | read_byte_PCE(0x1);

  // EXIT SOFTWARE ID PROGRAM
  flash_mode_PCE();
  write_byte_PCE(0x5555, 0xAA);
  write_byte_PCE(0x2AAA, 0x55);
  write_byte_PCE(0x5555, 0xF0);

  flashSize = 0;
  switch (deviceId) {
    case 0xBFB5:
      // SST39SF010 = 1Mbit
      flashSize = 131072UL;
      break;
    case 0xBFB6:
      // SST39SF020 = 2Mbit
      flashSize = 262144UL;
      break;
    case 0xBFB7:
      // SST39SF040 = 4Mbit
      flashSize = 524288UL;
      break;
    case 0xC2A4:
      // MX29F040 = 4Mbit
      flashSize = 524288UL;
      break;
    case 0xC2D5:
      // MX29F080 = 8Mbit
      flashSize = 1048576UL;
      break;
  }

  if (flashSize <= 0) {
    print_Msg(F("UNKNOWN "));
    println_Msg(deviceId);
    display_Update();
    wait();
    resetArduino();
    return;
  } else {
    print_Msg(FS(FSTRING_SIZE));
    print_Msg(flashSize / 131072UL);
    println_Msg(F("Mbit"));
    display_Update();
    wait();
  }

  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(FS(FSTRING_SELECT_FILE));
  display_Clear();

  if (openFlashFile()) {

    println_Msg(F("Erasing..."));
    display_Update();

    // CHIP ERASE PROGRAM
    flash_mode_PCE();
    write_byte_PCE(0x5555, 0xAA);
    write_byte_PCE(0x2AAA, 0x55);
    write_byte_PCE(0x5555, 0x80);
    write_byte_PCE(0x5555, 0xAA);
    write_byte_PCE(0x2AAA, 0x55);
    write_byte_PCE(0x5555, 0x10);
    // Data complement polling, wait until highest bit is 1
    flash_wait_status_PCE(0xFF);

    print_STR(flashing_file_STR, true);
    display_Update();
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    flash_mode_PCE();
    const size_t BUFSIZE = 512;
    for (unsigned long currAddr = 0; currAddr < fileSize; currAddr += BUFSIZE) {
      myFile.read(sdBuffer, BUFSIZE);

      if (currAddr % 4096 == 0) {
        blinkLED();
      }

      for (int currByte = 0; currByte < BUFSIZE; currByte++) {
        // BYTE PROGRAM
        byte b = sdBuffer[currByte];
        write_byte_PCE(0x5555, 0xAA);
        write_byte_PCE(0x2AAA, 0x55);
        write_byte_PCE(0x5555, 0xA0);
        write_byte_PCE(currAddr + currByte, b);
        flash_wait_status_PCE(b);
      }

      // update progress bar
      processedProgressBar += BUFSIZE;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    myFile.close();
    pin_init_PCE();
    print_STR(done_STR, true);
  }
  display_Update();
  wait();
}
#endif

// PC Engine Menu
void pceMenu() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;

  if (pce_internal_mode == HUCARD || pce_internal_mode == HUCARD_NOSWAP) {
    strcpy_P(menuOptions[0], FSTRING_READ_ROM);
    sprintf_P(menuOptions[1], menuOptionspceCart_1, tennokoe_bank_index + 1);
    sprintf_P(menuOptions[2], menuOptionspceCart_2, tennokoe_bank_index + 1);
    strcpy_P(menuOptions[3], menuOptionspceCart_3);
    strcpy_P(menuOptions[4], menuOptionspceCart_4);
    if (pce_force_rom_size > 0) {
      sprintf_P(menuOptions[5], menuOptionspceCart_5_fmt, pce_force_rom_size);
    } else {
      sprintf_P(menuOptions[5], menuOptionspceCart_5, FORCED_SIZE);
    }
    strcpy_P(menuOptions[6], FSTRING_RESET);
    mainMenu = question_box(F("PCE HuCARD menu"), menuOptions, 7, 0);

    // wait for user choice to come back from the question box menu
    switch (mainMenu) {
      case 0:
        read_rom_PCE();
        break;

      case 1:
        read_tennokoe_bank_PCE(tennokoe_bank_index);
        break;

      case 2:
        write_tennokoe_bank_PCE(tennokoe_bank_index);
        break;

      case 3:
        if (tennokoe_bank_index < 3) {
          tennokoe_bank_index++;
        }
        break;

      case 4:
        if (tennokoe_bank_index > 0) {
          tennokoe_bank_index--;
        }
        break;

      case 5:
        pce_force_rom_size = FORCED_SIZE;
        break;

      case 6:
        resetArduino();
        break;
    }
  } else if (pce_internal_mode == TURBOCHIP) {
    // Copy menuOptions out of progmem
    convertPgm(menuOptionspceTC, 2);
    mainMenu = question_box(F("TG TurboChip menu"), menuOptions, 2, 0);

    // wait for user choice to come back from the question box menu
    switch (mainMenu) {
      case 0:
        read_rom_PCE();
        break;

      case 1:
        resetArduino();
        break;
    }
  }
#ifdef ENABLE_FLASH
  else if (pce_internal_mode == PCE_FLASH) {
    const int max = 2;
    convertPgm(menuOptionspceFlash, max);
    mainMenu = question_box(F("Flash Repro menu"), menuOptions, max, 0);

    switch (mainMenu) {
      case 0:
        flash_PCE();
        break;

      case 1:
        resetArduino();
        break;
    }
  }
#endif
  else {
    print_MissingModule();  // does not return
  }
}

#endif

//******************************************
// End of File
//******************************************
