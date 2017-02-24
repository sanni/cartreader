//******************************************
// NINTENDO 64
//******************************************

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
bool tempBits[65];
int eepPages;

// N64 Controller
// 256 bits of received Controller data
char N64_raw_dump[257];
// Array that holds one Controller Pak block of 32 bytes
byte myBlock[33];
String rawStr = ""; // above char array read into a string
struct {
  char stick_x;
  char stick_y;
}
N64_status;
//stings that hold the buttons
String button = "N/A";
String lastbutton = "N/A";

// Rom base address
unsigned long romBase = 0x10000000;
unsigned long sramBase = 0x08000000;

// Flashram type
byte flashramType = 1;
boolean MN63F81MPN = false;

/******************************************
  Menu
*****************************************/
// N64 controller menu items
const char N64ContMenuItem1[] PROGMEM = "Test Controller";
const char N64ContMenuItem2[] PROGMEM = "Read ControllerPak";
const char N64ContMenuItem3[] PROGMEM = "Write ControllerPak";
const char N64ContMenuItem4[] PROGMEM = "Reset";
const char* const menuOptionsN64Controller[] PROGMEM = {N64ContMenuItem1, N64ContMenuItem2, N64ContMenuItem3, N64ContMenuItem4};

// N64 cart menu items
const char N64CartMenuItem1[] PROGMEM = "Read Rom";
const char N64CartMenuItem2[] PROGMEM = "Read Save";
const char N64CartMenuItem3[] PROGMEM = "Write Save";
const char N64CartMenuItem4[] PROGMEM = "Reset";
const char* const menuOptionsN64Cart[] PROGMEM = {N64CartMenuItem1, N64CartMenuItem2, N64CartMenuItem3, N64CartMenuItem4};

// N64 CRC32 error menu items
const char N64CRCMenuItem1[] PROGMEM = "Recalc CRC";
const char N64CRCMenuItem2[] PROGMEM = "Redump";
const char N64CRCMenuItem3[] PROGMEM = "Ignore";
const char N64CRCMenuItem4[] PROGMEM = "Reset";
const char* const menuOptionsN64CRC[] PROGMEM = {N64CRCMenuItem1, N64CRCMenuItem2, N64CRCMenuItem3, N64CRCMenuItem4};

// N64 Controller Menu
void n64ControllerMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsN64Controller, 4);
  mainMenu = question_box("N64 Controller", menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      display_Update();
      readController();
      break;

    case 1:
      display_Clear();
      display_Update();
      readMPK();
      println_Msg(F(""));
      println_Msg(F("Press Button."));
      display_Update();
      wait();
      break;

    case 2:
      display_Clear();
      display_Update();
      // Change to root
      filePath[0] = '\0';
      sd.chdir("/");
      // Launch file browser
      fileBrowser("Select mpk file");
      display_Clear();
      display_Update();
      writeMPK();
      verifyMPK();
      println_Msg(F(""));
      println_Msg(F("Press Button."));
      display_Update();
      wait();
      break;

    case 3:
      asm volatile ("  jmp 0");
      break;
  }
}

// N64 Cartridge Menu
void n64CartMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsN64Cart, 4);
  mainMenu = question_box("N64 Cart Reader", menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      sd.chdir("/");
      display_Clear();

      println_Msg(F("Reading Rom..."));
      display_Update();
      readRom_N64();
      break;

    case 1:
      sd.chdir("/");
      display_Clear();

      if (saveType == 1) {
        println_Msg(F("Reading Sram..."));
        display_Update();
        readSram(32768, 1);
      }
      else  if (saveType == 4) {
        getFramType();
        println_Msg(F("Reading Flashram..."));
        display_Update();
        readFram(flashramType);
      }
      else if ((saveType == 5) || (saveType == 6)) {
        println_Msg(F("Reading Eep..."));
        display_Update();
        readEeprom();
      }
      else {
        print_Error(F("Savetype Error"), false);
      }
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    case 2:
      filePath[0] = '\0';
      sd.chdir("/");
      if (saveType == 1) {
        // Launch file browser
        fileBrowser("Select sra file");
        display_Clear();

        writeSram(32768);
        writeErrors = verifySram(32768, 1);
        if (writeErrors == 0) {
          println_Msg(F("Sram verified OK"));
          display_Update();
        }
        else {
          print_Msg(F("Error: "));
          print_Msg(writeErrors);
          println_Msg(F(" bytes "));
          print_Error(F("did not verify."), false);
        }
      }
      else if (saveType == 4) {
        // Launch file browser
        fileBrowser("Select fla file");
        display_Clear();
        getFramType();
        writeFram(flashramType);
        print_Msg(F("Verifying..."));
        display_Update();
        writeErrors = verifyFram(flashramType);
        if (writeErrors == 0) {
          println_Msg(F("OK"));
          display_Update();
        }
        else {
          println_Msg("");
          print_Msg(F("Error: "));
          print_Msg(writeErrors);
          println_Msg(F(" bytes "));
          print_Error(F("did not verify."), false);
        }
      }
      else if ((saveType == 5) || (saveType == 6)) {
        // Launch file browser
        fileBrowser("Select eep file");
        display_Clear();

        writeEeprom();
        writeErrors = verifyEeprom();
        if (writeErrors == 0) {
          println_Msg(F("Eeprom verified OK"));
          display_Update();
        }
        else {
          print_Msg(F("Error: "));
          print_Msg(writeErrors);
          println_Msg(F(" bytes "));
          print_Error(F("did not verify."), false);
        }
      }
      else {
        print_Error(F("Savetype Error"), false);
      }
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    case 3:
      asm volatile ("  jmp 0");
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
  //PORTH &= ~(1 << 0);
  // Output a high signal on WR(PH5) RD(PH6), pins are active low therefore everything is disabled now
  PORTH |= (1 << 5) | (1 << 6);
  // Pull aleL(PC0) low and aleH(PC1) high
  PORTC &= ~(1 << 0);
  PORTC |= (1 << 1);

  // Set Eeprom Clock Pin(PH1) to Output
  DDRH |= (1 << 1);
  // Output a high signal
  PORTH |= (1 << 1);

  // Set Eeprom Data Pin(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Wait until all is stable
  delay(300);

  // Pull RESET(PH0) high
  //PORTH |= (1 << 0);
  //delay(10);

  // Print start page
  getCartInfo_N64();
  if (cartSize != 0) {
    println_Msg(F("N64 Cartridge Info"));
    println_Msg(F(""));
    print_Msg(F("Name: "));
    println_Msg(romName);
    print_Msg(F("ID: "));
    print_Msg(cartID);
    print_Msg(F(" Size: "));
    print_Msg(cartSize);
    println_Msg(F("MB"));
    print_Msg(F("Save: "));
    switch (saveType) {
      case 1:
        println_Msg(F("Sram"));
        break;
      case 4:
        println_Msg(F("Flashram"));
        break;
      case 5:
        println_Msg(F("4K Eeprom"));
        eepPages = 64;
        break;
      case 6:
        println_Msg(F("16K Eeprom"));
        eepPages = 256;
        break;
      default:
        println_Msg(F("unknown"));
        break;
    }
    print_Msg(F("Version: 1."));
    println_Msg(romVersion);

    // Wait for user input
    println_Msg(F(" "));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
  else {
    println_Msg(F("GAMEPAK ERROR"));
    println_Msg("");
    print_Msg(F("Name: "));
    println_Msg(romName);
    print_Msg(F("ID: "));
    println_Msg(cartID);
    println_Msg("");
    display_Update();
    print_Error(F("Cartridge unknown"), true);
  }
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
  __asm__("nop\n\t""nop\n\t");

  // Pull ale_L(PC0) low
  PORTC &= ~(1 << 0);

  // Wait ~600ns just to be sure address is set
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Set data pins to input
  adIn_N64();
}

// Read one word out of the cartridge
word readWord_N64() {
  // Pull read(PH6) low
  PORTH &= ~(1 << 6);

  // Wait ~310ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Join bytes from PINF and PINK into a word
  word tempWord = ( ( PINK & 0xFF ) << 8 ) | ( PINF & 0xFF );

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
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull write(PH5) high
  PORTH |= (1 << 5);

  // Wait ~125ns
  __asm__("nop\n\t""nop\n\t");

  // Set data pins to input
  adIn_N64();
}

/******************************************
   N64 Controller CRC Functions
 *****************************************/
static word addrCRC(word address) {
  // CRC table
  word xor_table[16] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01 };
  word crc = 0;
  // Make sure we have a valid address
  address &= ~0x1F;
  // Go through each bit in the address, and if set, xor the right value into the output
  for (int i = 15; i >= 5; i--) {
    // Is this bit set?
    if ( ((address >> i) & 0x1)) {
      crc ^= xor_table[i];
    }
  }
  // Just in case
  crc &= 0x1F;
  // Create a new address with the CRC appended
  return address | crc;
}

static byte dataCRC(byte * data) {
  byte ret = 0;
  for (byte i = 0; i <= 32; i++) {
    for (byte j = 7; j >= 0; j--) {
      int tmp = 0;
      if (ret & 0x80) {
        tmp = 0x85;
      }
      ret <<= 1;
      if ( i < 32 ) {
        if (data[i] & (0x01 << j)) {
          ret |= 0x1;
        }
      }
      ret ^= tmp;
    }
  }
  return ret;
}

/******************************************
   N64 Controller Protocol Functions
 *****************************************/
void N64_send(unsigned char *buffer, char length) {
  // Send these bytes
  char bits;
  bool bit;

  // This routine is very carefully timed by examining the assembly output.
  // Do not change any statements, it could throw the timings off
  //
  // We get 16 cycles per microsecond, which should be plenty, but we need to
  // be conservative. Most assembly ops take 1 cycle, but a few take 2
  //
  // I use manually constructed for-loops out of gotos so I have more control
  // over the outputted assembly. I can insert nops where it was impossible
  // with a for loop

  asm volatile (";Starting outer for loop");
outer_loop:
  {
    asm volatile (";Starting inner for loop");
    bits = 8;
inner_loop:
    {
      // Starting a bit, set the line low
      asm volatile (";Setting line to low");
      N64_LOW; // 1 op, 2 cycles

      asm volatile (";branching");
      if (*buffer >> 7) {
        asm volatile (";Bit is a 1");
        // 1 bit
        // remain low for 1us, then go high for 3us
        // nop block 1
        asm volatile ("nop\nnop\nnop\nnop\nnop\n");

        asm volatile (";Setting line to high");
        N64_HIGH;

        // nop block 2
        // we'll wait only 2us to sync up with both conditions
        // at the bottom of the if statement
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                     );

      }
      else {
        asm volatile (";Bit is a 0");
        // 0 bit
        // remain low for 3us, then go high for 1us
        // nop block 3
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\n");

        asm volatile (";Setting line to high");
        N64_HIGH;

        // wait for 1us
        asm volatile ("; end of conditional branch, need to wait 1us more before next bit");

      }
      // end of the if, the line is high and needs to remain
      // high for exactly 16 more cycles, regardless of the previous
      // branch path

      asm volatile (";finishing inner loop body");
      --bits;
      if (bits != 0) {
        // nop block 4
        // this block is why a for loop was impossible
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\n");
        // rotate bits
        asm volatile (";rotating out bits");
        *buffer <<= 1;

        goto inner_loop;
      } // fall out of inner loop
    }
    asm volatile (";continuing outer loop");
    // In this case: the inner loop exits and the outer loop iterates,
    // there are /exactly/ 16 cycles taken up by the necessary operations.
    // So no nops are needed here (that was lucky!)
    --length;
    if (length != 0) {
      ++buffer;
      goto outer_loop;
    } // fall out of outer loop
  }
}

void N64_stop() {
  // send a single stop (1) bit
  // nop block 5
  asm volatile ("nop\nnop\nnop\nnop\n");
  N64_LOW;
  // wait 1 us, 16 cycles, then raise the line
  // 16-2=14
  // nop block 6
  asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\n");
  N64_HIGH;
}

void N64_get(word bitcount) {
  // listen for the expected bitcount/8 bytes of data back from the controller and
  // blast it out to the N64_raw_dump array, one bit per byte for extra speed.
  asm volatile (";Starting to listen");
  unsigned char timeout;
  char *bitbin = N64_raw_dump;

  // Again, using gotos here to make the assembly more predictable and
  // optimization easier (please don't kill me)
read_loop:
  timeout = 0x3f;
  // wait for line to go low
  while (N64_QUERY) {
    if (!--timeout)
      return;
  }
  // wait approx 2us and poll the line
  asm volatile (
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\n"
  );
  *bitbin = N64_QUERY;
  ++bitbin;
  --bitcount;
  if (bitcount == 0)
    return;

  // wait for line to go high again
  // it may already be high, so this should just drop through
  timeout = 0x3f;
  while (!N64_QUERY) {
    if (!--timeout)
      return;
  }
  goto read_loop;
}

/******************************************
   N64 Controller Functions
 *****************************************/
void get_button()
{
  // Command to send to the gamecube
  // The last bit is rumble, flip it to rumble
  // yes this does need to be inside the loop, the
  // array gets mutilated when it goes through N64_send
  unsigned char command[] = {
    0x01
  };

  // don't want interrupts getting in the way
  noInterrupts();
  // send those 3 bytes
  N64_send(command, 1);
  N64_stop();
  // read in 32bits of data and dump it to N64_raw_dump
  N64_get(32);
  // end of time sensitive code
  interrupts();

  // The get_N64_status function sloppily dumps its data 1 bit per byte
  // into the get_status_extended char array. It's our job to go through
  // that and put each piece neatly into the struct N64_status
  int i;
  memset(&N64_status, 0, sizeof(N64_status));

  // bits: joystick x value
  // These are 8 bit values centered at 0x80 (128)
  for (i = 0; i < 8; i++) {
    N64_status.stick_x |= N64_raw_dump[16 + i] ? (0x80 >> i) : 0;
  }
  for (i = 0; i < 8; i++) {
    N64_status.stick_y |= N64_raw_dump[24 + i] ? (0x80 >> i) : 0;
  }

  // read char array N64_raw_dump into string rawStr
  rawStr = "";
  for (i = 0; i < 16; i++) {
    rawStr = rawStr + String(N64_raw_dump[i], DEC);
  }

  // Buttons (A,B,Z,S,DU,DD,DL,DR,0,0,L,R,CU,CD,CL,CR)
  if (rawStr.substring(0, 16) == "0000000000000000") {
    lastbutton = button;
    button = "Press a button";
  }
  else
  {
    for (int i = 0; i < 16; i++)
    {
      // seems to be 16, 8 or 4 depending on what pin is used
      if (N64_raw_dump[i] == 16)
      {
        switch (i)
        {
          case 7:
            button = "D-Right";
            break;

          case 6:
            button = "D-Left";
            break;

          case 5:
            button = "D-Down";
            break;

          case 4:
            button = "D-Up";
            break;

          case 3:
            button = "START";
            break;

          case 2:
            button = "Z";
            break;

          case 1:
            button = "B";
            break;

          case 0:
            button = "A";
            break;

          case 15:
            button = "C-Right";
            break;

          case 14:
            button = "C-Left";
            break;

          case 13:
            button = "C-Down";
            break;

          case 12:
            button = "C-Up";
            break;

          case 11:
            button = "R";
            break;

          case 10:
            button = "L";
            break;
        }
      }
    }
  }
}

void readController() {
  bool quit = 1;

  while (quit) {
    display_Clear();

    // Get Button and analog stick
    get_button();

    println_Msg(F("Controller Test"));
    println_Msg("");
    println_Msg(button);
    println_Msg("");
    String stickx = String("X: " + String(N64_status.stick_x, DEC) + "   ");
    println_Msg(stickx);
    String sticky = String("Y: " + String(N64_status.stick_y, DEC) + "   ");
    println_Msg(sticky);
    println_Msg("");
    println_Msg(F("Press START to quit"));

    display_Update();
    delay(100);

    if (button == "START") {
      quit = 0;
    }
  }
}

/******************************************
   N64 Controller Pak Functions
   (connected via Controller)
 *****************************************/
// read 32bytes from controller pak
void readBlock(word myAddress) {
  // Calculate the address CRC
  word myAddressCRC = addrCRC(myAddress);

  // Read Controller Pak command
  unsigned char command[] = {0x02};
  // Address Command
  unsigned char addressHigh[] = {(unsigned char)(myAddressCRC >> 8)};
  unsigned char addressLow[] = {(unsigned char)(myAddressCRC & 0xff)};

  // don't want interrupts getting in the way
  noInterrupts();
  // send those 3 bytes
  N64_send(command, 1);
  N64_send(addressHigh, 1);
  N64_send(addressLow, 1);
  N64_stop();
  // read in data
  N64_get(256);
  // end of time sensitive code
  interrupts();

  // Empty N64_raw_dump into myBlock
  for (word i = 0; i < 256; i += 8) {
    boolean byteFlipped[9];

    // Flip byte order
    byteFlipped[0] = N64_raw_dump[i + 7];
    byteFlipped[1] = N64_raw_dump[i + 6];
    byteFlipped[2] = N64_raw_dump[i + 5];
    byteFlipped[3] = N64_raw_dump[i + 4];
    byteFlipped[4] = N64_raw_dump[i + 3];
    byteFlipped[5] = N64_raw_dump[i + 2];
    byteFlipped[6] = N64_raw_dump[i + 1];
    byteFlipped[7] = N64_raw_dump[i + 0];

    // Join bits into one byte
    unsigned char myByte = 0;
    for (byte j = 0; j < 8; ++j) {
      if (byteFlipped[j]) {
        myByte |= 1 << j;
      }
    }
    // Save byte into block array
    myBlock[i / 8] = myByte;
  }
}

// reads the MPK file to the sd card
void readMPK() {
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "%d", foldern);
  strcat(fileName, ".mpk");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't open file on SD"), true);
  }

  println_Msg(F("Please wait..."));
  display_Update();

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Write block to SD card
    for (byte j = 0; j < 32; j++) {
      myFile.write(myBlock[j]);
    }
  }
  // Close the file:
  myFile.close();
  print_Msg(F("Saved as /MPK/"));
  println_Msg(fileName);
  display_Update();
}

void writeMPK() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    for (word myAddress = 0x0000; myAddress < 0x8000; myAddress += 32) {
      // Read 32 bytes into SD buffer
      myFile.read(sdBuffer, 32);

      // Calculate the address CRC
      word myAddressCRC = addrCRC(myAddress);

      // Write Controller Pak command
      unsigned char command[] = {0x03};
      // Address Command
      unsigned char addressHigh[] = {(unsigned char)(myAddressCRC >> 8)};
      unsigned char addressLow[] = {(unsigned char)(myAddressCRC & 0xff)};

      // don't want interrupts getting in the way
      noInterrupts();
      // Send write command
      N64_send(command, 1);
      // Send block number
      N64_send(addressHigh, 1);
      N64_send(addressLow, 1);
      // Send data to write
      N64_send(sdBuffer, 32);
      // Send stop
      N64_stop();
      // Enable interrupts
      interrupts();
    }
    // Close the file:
    myFile.close();
    println_Msg(F("Done"));
    display_Update();
  }
  else {
    print_Error(F("Can't create file on SD"), true);
  }
}

// verifies if write was successful
void verifyMPK() {
  writeErrors = 0;

  println_Msg(F("Verifying..."));
  display_Update();

  //open file on sd card
  if (!myFile.open(filePath, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Check against file on SD card
    for (byte j = 0; j < 32; j++) {
      if (myFile.read() != myBlock[j]) {
        writeErrors++;
      }
    }
  }
  // Close the file:
  myFile.close();
  if (writeErrors == 0) {
    println_Msg(F("OK"));
    display_Update();
  }
  else {
    print_Msg(F("Error: "));
    print_Msg(writeErrors);
    println_Msg(F(" bytes "));
    print_Error(F("did not verify."), false);
  }
}

/******************************************
  N64 Cartridge functions
*****************************************/
// CRC32 lookup table
static const uint32_t crc_32_tab[] PROGMEM = { /* CRC polynomial 0xedb88320 */
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

// Calculate dumped rom's CRC32
inline uint32_t updateCRC32(uint8_t ch, uint32_t crc) {
  uint32_t idx = ((crc) ^ (ch)) & 0xff;
  uint32_t tab_value = pgm_read_dword(crc_32_tab + idx);
  return tab_value ^ ((crc) >> 8);
}

// Read rom from sd
uint32_t crc32() {
  if (myFile.open(fileName, O_READ)) {
    uint32_t oldcrc32 = 0xFFFFFFFF;

    for (unsigned long currByte = 0; currByte < cartSize * 2048; currByte++) {
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 512; c++) {
        oldcrc32 = updateCRC32(sdBuffer[c], oldcrc32);
      }
    }
    // Close the file:
    myFile.close();
    return ~oldcrc32;
  }
  else {
    print_Error(F("File not found"), true);
  }
}

// look-up the calculated crc in the file n64.txt on sd card
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
      if (strcmp(tempStr1, crcStr) == 0) {
        // Skip the , in the file
        myFile.seekSet(myFile.curPosition() + 1);

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
        myFile.seekSet(myFile.curPosition() + 12);
      }
    }
    // Close the file:
    myFile.close();
    return result;
  }
  else {
    print_Error(F("N64.txt missing"), true);
  }
}

// look-up cart id in file n64.txt on sd card
void getCartInfo_N64() {
  char tempStr2[2];
  char tempStr[5];
  char sizeStr[3];
  char saveStr[2];

  // cart not in list
  cartSize = 0;
  saveType = 0;

  // Read cart id
  idCart();

  if (myFile.open("n64.txt", O_READ)) {
    // Skip over the first crc
    myFile.seekSet(myFile.curPosition() + 9);
    // Loop through file
    while (myFile.available()) {
      // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
      sprintf(tempStr, "%c", myFile.read());
      for (byte i = 0; i < 3; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(tempStr, tempStr2);
      }

      // Check if string is a match
      if (strcmp(tempStr, cartID) == 0) {
        // Skip the , in the file
        myFile.seekSet(myFile.curPosition() + 1);

        // Read the next ascii character and subtract 48 to convert to decimal
        cartSize = myFile.read() - 48;
        // Remove leading 0 for single digit cart sizes
        if (cartSize != 0) {
          cartSize = cartSize * 10 +  myFile.read() - 48;
        }
        else {
          cartSize = myFile.read() - 48;
        }

        // Skip the , in the file
        myFile.seekSet(myFile.curPosition() + 1);

        // Read the next ascii character and subtract 48 to convert to decimal
        saveType = myFile.read() - 48;
      }
      // If no match, empty string, advance by 16 and try again
      else {
        myFile.seekSet(myFile.curPosition() + 16);
      }
    }
    // Close the file:
    myFile.close();
  }
  else {
    print_Error(F("N64.txt missing"), true);
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

  // Get cart id
  cartID[0] = sdBuffer[0x3B];
  cartID[1] = sdBuffer[0x3C];
  cartID[2] = sdBuffer[0x3D];
  cartID[3] = sdBuffer[0x3E];

  // Get rom version
  romVersion = sdBuffer[0x3F];

  // Get name in 8.3 compatible format
  byte myLength = 0;
  for (unsigned int i = 0; i < 20; i++) {
    if (((char(sdBuffer[0x20 + i]) >= 48 && char(sdBuffer[0x20 + i]) <= 57) || (char(sdBuffer[0x20 + i]) >= 65 && char(sdBuffer[0x20 + i]) <= 122)) && myLength < 8) {
      romName[myLength] = char(sdBuffer[0x20 + i]);
      myLength++;
    }
  }
}

/******************************************
  Eeprom functions
*****************************************/
// Send a clock pulse of 2us length, 50% duty, 500kHz
void pulseClock_N64(unsigned int times) {
  for (unsigned int i = 0; i < (times * 2); i++) {
    // Switch the clock pin to 0 if it's 1 and 0 if it's 1
    PORTH ^= (1 << 1);
    // without the delay the clock pulse would be 1.5us and 666kHz
    //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t"));
  }
}

// Send one byte of data to eeprom
void sendData(byte data) {
  for (byte i = 0; i < 8; i++) {
    // pull data line low
    N64_LOW;

    // if current bit is 1, pull high after ~1us, 2 cycles
    if (data >> 7) {
      pulseClock_N64(2);
      N64_HIGH;
      pulseClock_N64(6);
    }
    // if current bit is 0 pull high after ~3us, 6 cycles
    else {
      pulseClock_N64(6);
      N64_HIGH;
      pulseClock_N64(2);
    }

    // rotate to the next bit
    data <<= 1;
  }
}

// Send stop bit to eeprom
void sendStop() {
  N64_LOW;
  pulseClock_N64(2);
  N64_HIGH;
  pulseClock_N64(4);
}

// Capture 8 bytes in 64 bits into bit array tempBits
void readData() {
  for (byte i = 0; i < 64; i++) {

    // pulse clock until we get response from eeprom
    while (N64_QUERY) {
      pulseClock_N64(1);
    }

    // Skip over the 1us low part of a high bit
    pulseClock_N64(3);

    // Read bit
    tempBits[i] = N64_QUERY;

    // wait for line to go high again
    while (!N64_QUERY) {
      pulseClock_N64(1);
    }
  }
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

      for (byte i = 0; i < (eepPages / 64); i++) {
        myFile.read(sdBuffer, 512);

        for (byte pageNumber = 0; pageNumber < 64; pageNumber++) {
          // Disable interrupts for more uniform clock pulses
          noInterrupts();

          // Wait ~50ms between page writes or eeprom will have write errors
          pulseClock_N64(26000);

          // Send write command
          sendData(0x05);
          // Send page number
          sendData(pageNumber + (i * 64));
          // Send data to write
          for (byte j = 0; j < 8; j++) {
            sendData(sdBuffer[(pageNumber * 8) + j]);
          }
          sendStop();

          interrupts();
        }
      }
      // Close the file:
      myFile.close();
      println_Msg(F("Done"));
      display_Update();
    }
    else {
      print_Error(F("SD Error"), true);
    }
  }
  else {
    print_Error(F("Savetype Error"), true);
  }
}

// Dump Eeprom to SD
void readEeprom() {
  if ((saveType == 5) || (saveType == 6)) {

    // Wait 0.6ms
    pulseClock_N64(300);

    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".eep");

    // create a new folder for the save file
    EEPROM_readAnything(0, foldern);
    sprintf(folder, "SAVE/%s/%d", romName, foldern);
    sd.mkdir(folder, true);
    sd.chdir(folder);

    // write new folder number back to eeprom
    foldern = foldern + 1;
    EEPROM_writeAnything(0, foldern);

    // Open file on sd card
    if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
      print_Error(F("Can't create file on SD"), true);
    }

    for (byte i = 0; i < (eepPages / 64); i++) {
      for (byte pageNumber = 0; pageNumber < 64; pageNumber++) {
        // Disable interrupts for more uniform clock pulses
        noInterrupts();

        // Send read command
        sendData(0x04);
        // Send Page number
        sendData(pageNumber + (i * 64));
        // Send stop bit
        sendStop();

        // read data
        readData();
        sendStop();

        interrupts();

        // OR 8 bits into one byte for a total of 8 bytes
        for (byte j = 0; j < 64; j += 8) {
          sdBuffer[(pageNumber * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
        }
        // Wait ~0.6ms between pages or eeprom will lock up
        pulseClock_N64(300);
      }

      // Write 64 pages at once to the SD card
      myFile.write(sdBuffer, 512);
    }
    // Close the file:
    myFile.close();
    print_Msg(F("Saved to SAVE/"));
    print_Msg(romName);
    print_Msg(F("/"));
    print_Msg(foldern - 1);
    print_Msg(F("/"));
    println_Msg(fileName);
    display_Update();
  }
  else {
    print_Error(F("Savetype Error"), true);
  }
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyEeprom() {
  if ((saveType == 5) || (saveType == 6)) {
    writeErrors = 0;

    // Wait 0.6ms
    pulseClock_N64(300);

    print_Msg(F("Verifying against "));
    println_Msg(filePath);
    display_Update();

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {

      for (byte i = 0; i < (eepPages / 64); i++) {
        for (byte pageNumber = 0; pageNumber < 64; pageNumber++) {
          // Disable interrupts for more uniform clock pulses
          noInterrupts();

          // Send read command
          sendData(0x04);
          // Send Page number
          sendData(pageNumber + (i * 64));
          // Send stop bit
          sendStop();

          // read data
          readData();
          sendStop();

          interrupts();

          // OR 8 bits into one byte for a total of 8 bytes
          for (byte j = 0; j < 64; j += 8) {
            sdBuffer[(pageNumber * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
          }
          // Wait ~0.6ms between pages or eeprom will lock up
          pulseClock_N64(300);
        }

        // Check sdBuffer content against file on sd card
        for (int c = 0; c < 512; c++) {
          if (myFile.read() != sdBuffer[c]) {
            writeErrors++;
          }
        }
      }
      // Close the file:
      myFile.close();
    }
    else {
      // SD Error
      writeErrors = 999999;
      print_Error(F("SD Error"), true);
    }
    // Return 0 if verified ok, or number of errors
    return writeErrors;
  }
  else {
    print_Error(F("Savetype Error"), true);
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
          word myWord = ( ( sdBuffer[c] & 0xFF ) << 8 ) | ( sdBuffer[c + 1] & 0xFF );

          // Write word
          writeWord_N64(myWord);
        }
      }
      // Close the file:
      myFile.close();
      println_Msg(F("Done"));
      display_Update();
    }
    else {
      print_Error(F("SD Error"), true);
    }

  }
  else {
    print_Error(F("Savetype Error"), true);
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
  }
  else if (saveType == 1) {
    strcat(fileName, ".sra");
  }
  else {
    print_Error(F("Savetype Error"), true);
  }

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
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
  print_Msg(F("Saved to SAVE/"));
  print_Msg(romName);
  print_Msg(F("/"));
  print_Msg(foldern - 1);
  print_Msg(F("/"));
  println_Msg(fileName);
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
  }
  else {
    print_Error(F("SD Error"), true);
  }
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}

/******************************************
  Flashram functions
*****************************************/
// Send a command to the flashram command register
void sendFramCmd (unsigned long myCommand) {
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
    }
    else {
      println_Msg("FAILED");
      print_Error(F("Flash is not blank"), true);
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
            word myWord = ( ( sdBuffer[c] & 0xFF ) << 8 ) | ( sdBuffer[c + 1] & 0xFF );
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
      // Close the file:
      myFile.close();
    }
    else {
      print_Error(F("SD Error"), true);
    }
  }
  else {
    print_Error(F("Savetype Error"), true);
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
  }
  else {
    print_Error(F("Savetype Error"), true);
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
  }
  else {
    print_Error(F("Savetype Error"), true);
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
  byte statusMXL1100[] = {0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1E};
  byte statusMXL1101[] = {0x11, 0x11, 0x80, 0x01, 0x00, 0xC2, 0x00, 0x1D};
  byte statusMN63F81[] = {0x11, 0x11, 0x80, 0x01, 0x00, 0x32, 0x00, 0xF1};

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
  }
  else if (flashramType == 1) {
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
  if (sdBuffer[7] == 0x1e ) {
    flashramType = 2;
    println_Msg(F("Type: MX29L1100"));
    display_Update();
  }
  //MX29L1101
  else if (sdBuffer[7] == 0x1d )  {
    flashramType = 1;
    MN63F81MPN = false;
    println_Msg(F("Type: MX29L1101"));
    display_Update();
  }
  //MN63F81MPN
  else if (sdBuffer[7] == 0xf1 )  {
    flashramType = 1;
    MN63F81MPN = true;
    println_Msg(F("Type: MN63F81MPN"));
    display_Update();
  }
  // Type unknown
  else {
    for (byte c = 0; c < 8; c++) {
      print_Msg(sdBuffer[c], HEX);
      print_Msg(F(", "));
    }
    print_Error(F("Flashram unknown"), true);
  }
}

/******************************************
  Rom functions
*****************************************/
// Read rom and save to the SD card
void readRom_N64() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".Z64");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

readn64rom:
  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }

  for (unsigned long currByte = romBase; currByte < (romBase + (cartSize * 1024 * 1024)); currByte += 512) {
    // Blink led
    if (currByte % 16384 == 0)
      PORTB ^= (1 << 4);

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
  print_Msg(F("Saved to ROM/"));
  print_Msg(romName);
  print_Msg(F("/"));
  print_Msg(foldern - 1);
  print_Msg(F("/"));
  println_Msg(fileName);
  display_Update();

calcn64crc:
  // Calculate Checksum and convert to string
  println_Msg(F("Calculating CRC.."));
  display_Update();
  char crcStr[9];
  sprintf(crcStr, "%08lx", crc32());
  // Print checksum
  println_Msg(crcStr);
  display_Update();

  // Search n64.txt for crc
  if (searchCRC(crcStr)) {
    // Dump was a known good rom
    println_Msg(F("Checksum matches"));
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
  else {
    // Dump was bad or unknown
    rgb.setColor(255, 0, 0);
    // N64 CRC32 error Menu
    unsigned char CRCMenu;
    // Copy menuOptions out of progmem
    convertPgm(menuOptionsN64CRC, 4);

    char tempStr3[20];
    strcpy(tempStr3, "CRC ERROR ");
    strcat(tempStr3, crcStr);

    CRCMenu = question_box(tempStr3, menuOptions, 4, 1);

    // wait for user choice to come back from the question box menu
    switch (CRCMenu)
    {
      case 0:
        // Change to last directory
        sd.chdir(folder);
        display_Clear();
        // Calculate CRC again
        rgb.setColor(0, 0, 0);
        goto calcn64crc;
        break;

      case 1:
        // Change to last directory
        sd.chdir(folder);
        // Delete old file
        if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
          print_Error(F("SD Error"), true);
        }
        if (!myFile.remove()) {
          print_Error(F("Delete Error"), true);
        }
        // Dump again
        display_Clear();
        println_Msg(F("Reading Rom..."));
        display_Update();
        rgb.setColor(0, 0, 0);
        goto readn64rom;
        break;

      case 2:
        // Return to N64 menu
        break;

      case 3:
        // Reset
        asm volatile ("  jmp 0");
        break;
    }
  }
  display_Update();
}

//******************************************
// End of File
//******************************************
