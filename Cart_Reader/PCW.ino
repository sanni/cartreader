//******************************************
// BENESSE POCKET CHALLENGE W MODULE
//******************************************
#ifdef ENABLE_PCW

// Benesse Pocket Challenge W
// Cartridge Pinout
// 38P 1.27mm pitch connector
//
// CART            CART
// TOP             EDGE
//     +---------+
//     |   1   |-- VCC
//     |   2   |-- GND
//     |   3   |-- AD0 (A0 IN/D0 OUT)
//     |   4   |-- AD1 (A1 IN/D1 OUT)
//     |   5   |-- AD2 (A2 IN/D2 OUT)
//     |   6   |-- AD3 (A3 IN/D3 OUT)
//     |   7   |-- AD4 (A4 IN/D4 OUT)
//     |   8   |-- AD5 (A5 IN/D5 OUT)
//     |   9   |-- AD6 (A6 IN/D6 OUT)
//     |  10   |-- AD7 (A7 IN/D7 OUT)
//     |  11   |-- A08
//     |  12   |-- A09
//     |  13   |-- A10
//     |  14   |-- A11
//     |  15   |-- A12
//     |  16   |-- A13
//     |  17   |-- A14
//     |  18   |-- A15
//     |  19   |-- A16
//     |  20   |-- A17
//     |  21   |-- A18
//     |  22   |-- A19
//     |  23   |-- A20
//     |  24   |-- A21
//     |  25   |-- NC [CPU-INT2/T15(P47)]
//     |  26   |-- 1B (7W00F) -> HIGH -+
//     |  27   |-- 1A (7W00F) -> HIGH -+-> OE (ROM) = LOW
//     |  28   |-- OE (LH5164A)
//     |  29   |-- WE (LH5164A)
//     |  30   |-- LE (HC573) [LATCH ENABLE FOR AD0-AD7]
//     |  31   |-- NC [CPU-TO5(P43)]
//     |  32   |-- NC [CPU-RESET]
//     |  33   |-- VCC
//     |  34   |-- GND
//     |  35   |-- NC
//     |  36   |-- NC [CPU-VCC]
//     |  37   |-- NC [CONSOLE-GND]
//     |  38   |-- NC
//     +---------+

// CONTROL PINS:
// LE - (PH0) - PCW PIN 30 - SNES RST
// 1A - (PH3) - PCW PIN 27 - SNES /CS
// 1B - (PH4) - PCW PIN 26 - SNES /IRQ
// WE - (PH5) - PCW PIN 29 - SNES /WR
// OE - (PH6) - PCW PIN 28 - SNES /RD

// NOT CONNECTED:
// CLK(PH1) - N/C

// ADDRESS PINS:
// ADDR/DATA [AD0-AD7] - PORTC
// ADDR A8-A15 - PORTK
// ADDR A16-A21 - PORTL

//******************************************
// DEFINES
//******************************************

// CONTROL PINS - LE/1A/1B/WE/OE
#define LE_HIGH PORTH |= (1 << 0)
#define LE_LOW PORTH &= ~(1 << 0)
#define NAND_1A_HIGH PORTH |= (1 << 3)
#define NAND_1A_LOW PORTH &= ~(1 << 3)
#define NAND_1B_HIGH PORTH |= (1 << 4)
#define NAND_1B_LOW PORTH &= ~(1 << 4)  // Built-in RAM + I/O
#define WE_HIGH_PCW PORTH |= (1 << 5)
#define WE_LOW_PCW PORTH &= ~(1 << 5)
#define OE_HIGH_PCW PORTH |= (1 << 6)
#define OE_LOW_PCW PORTH &= ~(1 << 6)

#define MODE_READ DDRC = 0      // [INPUT]
#define MODE_WRITE DDRC = 0xFF  //[OUTPUT]

#define DATA_READ \
  { \
    DDRC = 0; \
    PORTC = 0xFF; \
  }                             // [INPUT PULLUP]
#define ADDR_WRITE DDRC = 0xFF  // [OUTPUT]
#define DETECTION_SIZE 64

uint32_t rom_size;
boolean multipack;
byte bank0;
byte bank1;

//******************************************
// SETUP
//******************************************

void setup_PCW()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  //A8-A15
  DDRK = 0xFF;
  //A16-A21
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       LE(PH0)    --(PH1)    1A(PH3)    1B(PH4)    WE(PH5)    OE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Address/Data Pins AD0-AD7 (PC0-PC7) to Input
  MODE_READ;  // DDRC = 0

  // Setting Control Pins to HIGH
  //        LE(PH0)   ---(PH1)    1A(PH3)    1B(PH4)    WE(PH5)    OE(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Pins HIGH
  PORTJ |= (1 << 0);  // TIME(PJ0)

  strcpy(romName, "PCW");

  mode = CORE_PCW;
}

//******************************************
//  MENU
//******************************************
static const char* const menuOptionsPCW[] PROGMEM = { FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, FSTRING_RESET };

void pcwMenu()
{
  convertPgm(menuOptionsPCW, 4);
  uint8_t mainMenu = question_box(F(" POCKET CHALLENGE W"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Read ROM
      sd.chdir("/");
      check_multi_PCW();
      if (multipack)
        readMultiROM_PCW();
      else
        readSingleROM_PCW();
      sd.chdir("/");
      break;

    case 1:
      // Read SRAM
      sd.chdir("/");
      display_Clear();
      println_Msg(F("Reading SRAM..."));
      display_Update();
      readSRAM_PCW();
      sd.chdir("/");
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;

    case 2:
      // Write SRAM
      sd.chdir("/");
      fileBrowser(F("Select SRM file"));
      display_Clear();
      writeSRAM_PCW();
      display_Clear();
      writeErrors = verifySRAM_PCW();
      if (writeErrors == 0) {
        println_Msg(F("SRAM verified OK"));
        display_Update();
      } else {
        print_STR(error_STR, 0);
        print_Msg(writeErrors);
        print_STR(_bytes_STR, 1);
        print_Error(did_not_verify_STR);
      }
      break;

    case 3:
      // Reset
      resetArduino();
      break;
  }
}

//******************************************
//  LOW LEVEL FUNCTIONS
//******************************************

// Max ROM Size 0x400000 (Highest Address = 0x3FFFFF) - 3F FFFF
// NAND 1A + 1B HIGH = LOW = ROM Output Enabled
void read_setup_PCW()
{
  NAND_1A_HIGH;
  NAND_1B_HIGH;
  OE_HIGH_PCW;
  WE_HIGH_PCW;
  LE_LOW;
}

// READ ROM BYTE WITH ADDITIONAL DELAY
// NEEDED FOR PROBLEM CARTS TO SWITCH FROM ADDRESS TO DATA
unsigned char read_rom_byte_PCW(unsigned long address)
{
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  // Latch Address on AD0-AD7
  ADDR_WRITE;
  LE_HIGH;                 // Latch Enable
  PORTC = address & 0xFF;  // A0-A7
  LE_LOW;                  // Address Latched
  __asm__("nop\n\t"
          "nop\n\t");
  // Read Data on AD0-AD7
  OE_LOW_PCW;
  DATA_READ;
  delayMicroseconds(5);  // 3+ Microseconds for Problem Carts
  unsigned char data = PINC;
  OE_HIGH_PCW;

  return data;
}

// SRAM Size 0x8000 (Highest Address = 0x7FFF)
// NAND 1A LOW = SRAM Enabled [ROM DISABLED]
unsigned char read_ram_byte_1A_PCW(unsigned long address)
{
  NAND_1A_LOW;
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  // Latch Address on AD0-AD7
  ADDR_WRITE;
  LE_HIGH;                 // Latch Enable
  PORTC = address & 0xFF;  // A0-A7
  LE_LOW;                  // Address Latched
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  // Read Data on AD0-AD7
  OE_LOW_PCW;
  DATA_READ;
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  unsigned char data = PINC;
  OE_HIGH_PCW;
  NAND_1A_HIGH;
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return data;
}

// Toshiba TMP90C845A
// 0xFEC0-0xFFBF Built-in RAM (256 bytes)
// 0xFFC0-0xFFF7 Built-in I/O (56 bytes)
// 0xFF00-0xFFBF (192 byte area available in direct addressing mode)
// 0xFF18-0xFF68 (Micro DMA parameters (if used))

// TEST CODE TO READ THE CPU BUILT-IN RAM + I/O
// NAND 1B LOW = Built-In RAM + I/O Enabled [ROM DISABLED]
unsigned char read_ram_byte_1B_PCW(unsigned long address)
{
  NAND_1B_LOW;
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  // Latch Address on AD0-AD7
  ADDR_WRITE;
  LE_HIGH;                 // Latch Enable
  PORTC = address & 0xFF;  // A0-A7
  LE_LOW;                  // Address Latched
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  // Read Data on AD0-AD7
  OE_LOW_PCW;
  DATA_READ;
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  unsigned char data = PINC;
  OE_HIGH_PCW;
  NAND_1B_HIGH;
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return data;
}

// WRITE SRAM 32K
void write_ram_byte_1A_PCW(unsigned long address, unsigned char data)
{
  NAND_1A_LOW;
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  // Latch Address on AD0-AD7
  ADDR_WRITE;
  LE_HIGH;                 // Latch Enable
  PORTC = address & 0xFF;  // A0-A7
  LE_LOW;                  // Address Latched
  // Write Data on AD0-AD7 - WE LOW ~240-248ns
  WE_LOW_PCW;
  PORTC = data;
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  WE_HIGH_PCW;
  NAND_1A_HIGH;
}

// WRITE CPU BUILT-IN RAM + I/O AREA
// MODIFIED TO MATCH WORKING BANK SWITCH ROUTINE
void write_ram_byte_1B_PCW(unsigned long address, unsigned char data)
{
  NAND_1A_LOW;
  NAND_1A_HIGH;
  NAND_1B_LOW;
  PORTL = (address >> 16) & 0xFF;
  PORTK = (address >> 8) & 0xFF;
  // Latch Address on AD0-AD7
  ADDR_WRITE;
  LE_HIGH;                 // Latch Enable
  PORTC = address & 0xFF;  // A0-A7
  LE_LOW;                  // Address Latched
  // Write Data on AD0-AD7 - WE LOW ~740ns
  WE_LOW_PCW;
  PORTC = data;
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  WE_HIGH_PCW;
  NAND_1B_HIGH;
}

//******************************************
//  SINGLE-PACK FUNCTIONS
//******************************************

uint32_t detect_rom_size_PCW(void)
{
  uint8_t read_byte;
  uint8_t current_byte;
  uint8_t detect_1m, detect_2m;

  //Initialize variables
  detect_1m = 0;
  detect_2m = 0;

  //Confirm where mirror address starts from (1MB, 2MB or 4MB)
  for (current_byte = 0; current_byte < DETECTION_SIZE; current_byte++) {
    if ((current_byte != detect_1m) && (current_byte != detect_2m)) {
      //If none matched, size is 4MB
      break;
    }

    read_byte = read_rom_byte_PCW(current_byte);

    if (current_byte == detect_1m) {
      if (read_rom_byte_PCW(0x100000 + current_byte) == read_byte) {
        detect_1m++;
      }
    }
    if (current_byte == detect_2m) {
      if (read_rom_byte_PCW(0x200000 + current_byte) == read_byte) {
        detect_2m++;
      }
    }
  }

  //ROM size detection
  if (detect_1m == DETECTION_SIZE) {
    rom_size = 0x100000;
  } else if (detect_2m == DETECTION_SIZE) {
    rom_size = 0x200000;
  } else {
    rom_size = 0x400000;
  }

  return rom_size;
}

void readSingleROM_PCW()
{
  // Setup read mode
  read_setup_PCW();

  // Detect rom size
  uint32_t rom_size = detect_rom_size_PCW();
  display_Clear();
  print_Msg(F("READING "));
  print_Msg(rom_size / 1024 / 1024);
  print_Msg("MB SINGLE-PACK");
  println_Msg(FS(FSTRING_EMPTY));

  // Create file
  createFolder("PCW", "ROM", romName, "pcw");

  printAndIncrementFolder();

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  // Init progress bar
  uint32_t progress = 0;
  draw_progressbar(0, rom_size);

  for (unsigned long address = 0; address < rom_size; address += 512) {  // 4MB
    for (unsigned int x = 0; x < 512; x++) {
      sdBuffer[x] = read_rom_byte_PCW(address + x);
    }
    myFile.write(sdBuffer, 512);
    progress += 512;
    draw_progressbar(progress, rom_size);
  }
  myFile.flush();
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  compareCRC("pcw.txt", 0, 1, 0);

  // Wait for user input
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
//  MULTI-PACK FUNCTIONS
//******************************************

// Known Multi-Pack Carts (Yellow Label Carts)
// 0BD400 [PS] (2MB Version)
// 0BD400 [PS] (4MB Version)
// 0BF400 [PL]
// 1BF400 [PZ]
// 8BD400 [CR]
// 8BF400 [LP]
// 9BF400 [SLP]

// Per Overload, identify multi-pack cart by reading 0x3FFA-0x3FFE. Multi-Pack carts are non-zero.
// 0x3FFA - Current Cartridge Bank
// 0x3FFC - Value to Switch to Cartridge Bank 0
// 0x3FFD - Value to Switch to Cartridge Bank 1
// 0x3FFE - Last Value written to 0xFFFF

// Bank Settings for 2MB
// Write 0x28 to 0xFFFF to read 1st half of ROM
// Write 0x2E to 0xFFFF to read 2nd half of ROM

// Bank Settings for 4MB
// Write 0x20 to 0xFFFF to read 1st half of ROM
// Write 0x31 to 0xFFFF to read 2nd half of ROM

void check_multi_PCW()
{
  // init variables
  read_setup_PCW();
  multipack = 0;
  bank0 = 0;
  bank1 = 0;

  byte tempbyte = read_rom_byte_PCW(0x3FFC); // Check for a bank 0 switch value
  if (tempbyte) {
    bank0 = tempbyte; // Store bank 0 switch value
    tempbyte = read_rom_byte_PCW(0x3FFD); // Check for a bank 1 switch value
    if (tempbyte) {
      bank1 = tempbyte; // Store bank 1 switch value
      if (!read_rom_byte_PCW(0x3FFB) && !read_rom_byte_PCW(0x3FFF)) { // Check for 00s
        multipack = 1; // Flag as multi-pack
        display_Clear();
        if ((bank0 == 0x28) && (bank1 == 0x2E)) // 2MB multi-pack cart
          rom_size = 0x200000;
        else if ((bank0 == 0x20) && (bank1 == 0x31)) // 4MB multi-pack cart
          rom_size = 0x400000;
        else { // Warn for unknown bank switch values, size set to 4MB
          println_Msg(F("Warning: Unknown cart size"));
          rom_size = 0x400000;
        }
      }
    }
  }
}

void write_bank_byte_PCW(unsigned char data)
{
  NAND_1A_LOW;
  NAND_1A_HIGH;
  NAND_1B_LOW;
  // Write to Address 0xFFFF
  PORTL = 0x00;
  PORTK = 0xFF;  // A8-A15
  // Latch Address on AD0-AD7
  ADDR_WRITE;
  LE_HIGH;       // Latch Enable
  PORTC = 0xFF;  // A0-A7
  LE_LOW;        // Address Latched
  // Write Data on AD0-AD7 - WE LOW ~728-736ns
  WE_LOW_PCW;
  PORTC = data;

  for (unsigned int x = 0; x < 40; x++)
      __asm__("nop\n\t");

  WE_HIGH_PCW;
  NAND_1B_HIGH;
}

void switchBank_PCW(int bank)
{
  if (bank == 1) {  // Upper Half
    write_bank_byte_PCW(bank1);
  } else {  // Lower Half (default)
    write_bank_byte_PCW(bank0);
  }
}

void readMultiROM_PCW()
{
  print_Msg(F("READING "));
  print_Msg(rom_size / 1024 / 1024);
  print_Msg("MB MULTI-PACK");
  println_Msg(FS(FSTRING_EMPTY));

  // Create file
  createFolder("PCW", "ROM", romName, "pcw");

  printAndIncrementFolder();

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  // Init progress bar
  uint32_t progress = 0;
  draw_progressbar(0, rom_size);

  // Lower Half
  read_setup_PCW();
  switchBank_PCW(0);
  for (unsigned long address = 0; address < (rom_size / 2); address += 512) {
    for (unsigned int x = 0; x < 512; x++) {
      sdBuffer[x] = read_rom_byte_PCW(address + x);
    }
    myFile.write(sdBuffer, 512);
    progress += 512;
    draw_progressbar(progress, rom_size);
  }

  // Upper Half
  read_setup_PCW();
  switchBank_PCW(1);
  for (unsigned long address = 0x200000; address < (0x200000 + (rom_size / 2)); address += 512) {
    for (unsigned int x = 0; x < 512; x++) {
      sdBuffer[x] = read_rom_byte_PCW(address + x);
    }
    myFile.write(sdBuffer, 512);
    progress += 512;
    draw_progressbar(progress, rom_size);
  }

  myFile.flush();
  myFile.close();

  // Reset Bank
  switchBank_PCW(0);

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  compareCRC("pcw.txt", 0, 1, 0);

  // Wait for user input
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// SRAM FUNCTIONS
//******************************************

void readSRAM_PCW()
{  // readSRAM_1A()
  createFolder("PCW", "SAVE", romName, "srm");

  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }
  display_Clear();
  read_setup_PCW();
  for (unsigned long address = 0x0; address < 0x8000; address += 512) {  // 32K
    for (unsigned int x = 0; x < 512; x++) {
      sdBuffer[x] = read_ram_byte_1A_PCW(address + x);
    }
    myFile.write(sdBuffer, 512);
  }
  myFile.flush();
  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/"));
  display_Update();
  //  calcCRC(fileName, 0x8000, NULL, 0); // 32K
}

// SRAM
void writeSRAM_PCW()
{
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);
    display_Clear();
    println_Msg(F("Writing File: "));
    println_Msg(filePath);
    println_Msg(fileName);
    display_Update();
    //open file on sd card
    if (myFile.open(filePath, O_READ)) {
      read_setup_PCW();
      for (unsigned int address = 0x0; address < 0x8000; address += 512) {  // 32K
        myFile.read(sdBuffer, 512);
        for (unsigned int x = 0; x < 512; x++) {
          write_ram_byte_1A_PCW(address + x, sdBuffer[x]);
        }
      }
      myFile.close();
      print_STR(done_STR, 1);
      display_Update();
    } else {
      print_FatalError(sd_error_STR);
    }
  } else {
    print_FatalError(sd_error_STR);
  }
  display_Clear();
}

unsigned long verifySRAM_PCW()
{
  writeErrors = 0;

  if (myFile.open(filePath, O_READ)) {
    read_setup_PCW();
    for (unsigned int address = 0x0; address < 0x8000; address += 512) {  // 32K
      for (unsigned int x = 0; x < 512; x++) {
        byte myByte = read_ram_byte_1A_PCW(address + x);
        sdBuffer[x] = myByte;
      }
      for (int i = 0; i < 512; i++) {
        if (myFile.read() != sdBuffer[i]) {
          writeErrors++;
        }
      }
    }
    myFile.close();
  } else {
    print_FatalError(sd_error_STR);
  }

  return writeErrors;
}

// avoid warnings
#undef MODE_READ
#undef MODE_WRITE

#endif
//******************************************
// End of File
//******************************************
