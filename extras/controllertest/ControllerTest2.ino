/**********************************************************************************
                  Nintendo 64 Controller Test for Arduino Mega

   Author:           sanni
   Date:             2016-04-15
   Version:          V2

   OLED  lib:        http://www.rinkydinkelectronics.com/library.php?id=79

   Thanks to:
   Andrew Brown/Peter Den Hartog - N64 send/get functions

**********************************************************************************/

#include <OLED_I2C.h>
extern uint8_t SmallFont[];

// define LCD pins
OLED myOLED(SDA, SCL, 8);

//define LED pin
int ledPin = 10;

// These two macros toggle the eepDataPin/ControllerDataPin between input and output
// External 1K pull-up resistor from eepDataPin to VCC required
// 0x10 = 00010000 -> Port H Pin 4
#define N64_HIGH DDRH &= ~0x10
#define N64_LOW DDRH |= 0x10
// Read the current state(0/1) of the eepDataPin
#define N64_QUERY (PINH & 0x10)

// received Controller data
char N64_raw_dump[33]; // 1 received bit per byte
String rawStr = ""; // above char array read into a string
struct {
  char stick_x;
  char stick_y;
}
N64_status;

// on which screens do we start
int startscreen = 0;
int mode = 0;
int test = 1;

//stings that hold the buttons
String button = "N/A";
String lastbutton = "N/A";

//name of the current displayed result
String anastick = "";

// Graph
int xax = 22 + 24; // midpoint x
int yax = 24; // midpoint y
int zax = 24; // size

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

void N64_send(unsigned char *buffer, char length);
void N64_get();

void setup()
{
  // Communication with controller on this pin
  // Don't remove these lines, we don't want to push +5V to the controller
  // Output a low signal
  PORTH &= ~(1 << 4);
  // Set Controller Data Pin(PH4) to Input
  DDRH &= ~(1 << 4);

  // Led
  pinMode(ledPin, OUTPUT);

  // OLED
  myOLED.begin();
  myOLED.setFont(SmallFont);
}

// This sends the given byte sequence to the controller
// length must be at least 1
// Oh, it destroys the buffer passed in as it writes it

void N64_send(unsigned char *buffer, char length)
{
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

void N64_get()
{
  // listen for the expected 8 bytes of data back from the controller and
  // blast it out to the N64_raw_dump array, one bit per byte for extra speed.
  // Afterwards, call translate_raw_data() to interpret the raw data and pack
  // it into the N64_status struct.
  asm volatile (";Starting to listen");
  unsigned char timeout;
  char bitcount = 32;
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
  // read in data and dump it to N64_raw_dump
  N64_get();
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
    digitalWrite(ledPin, LOW);
  }
  else
  {
    digitalWrite(ledPin, HIGH);
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

void printSTR(String st, int x, int y)
{
  char buf[st.length() + 1];

  st.toCharArray(buf, st.length() + 1);
  myOLED.print(buf, x, y);
}

void nextscreen()
{
  if (button == "Press a button" && lastbutton == "START")
  {
    // reset button
    lastbutton = "N/A";

    myOLED.clrScr();
    if (startscreen != 4)
      startscreen = startscreen + 1;
    else
    {
      startscreen = 1;
      test = 1;
    }
  }
}

void loop()
{
  // Get Button and analog stick
  get_button();

  switch (startscreen)
  {
    case 0:	// Logo Screen
      {
        myOLED.print("ControllerTest", CENTER, 8);
        myOLED.print("V1.0", CENTER, 18);
        myOLED.drawLine(22 + 0, 28, 22 + 84, 28);
        myOLED.print("2013 sanni", CENTER, 32);
        myOLED.update();

        delay(1500);
        startscreen = 1;
        myOLED.clrScr();
        break;
      }
    case 1:
      {
        myOLED.print("Button Test", CENTER, 0);
        myOLED.drawLine(22 + 0, 10, 22 + 84, 10);

        // Print Button
        printSTR("       " + button + "       ", CENTER, 20);

        // Print Stick X Value
        String stickx = String("X: " + String(N64_status.stick_x, DEC) + "   ");
        printSTR(stickx, 22 + 0, 38);

        // Print Stick Y Value
        String sticky = String("Y: " + String(N64_status.stick_y, DEC) + "   ");
        printSTR(sticky, 22 + 42, 38);

        //Update LCD
        myOLED.update();

        // go to next screen
        nextscreen();
        break;
      }
    case 2:
      {
        myOLED.print("Range", 22 + 52, 5);
        myOLED.print("Test", 22 + 52, 15);
        myOLED.drawRect(22 + 50, 0, 22 + 83, 25);

        // Print Stick X Value
        String stickx = String("X:" + String(N64_status.stick_x, DEC) + "   ");
        printSTR(stickx, 22 + 50, 28);

        // Print Stick Y Value
        String sticky = String("Y:" + String(N64_status.stick_y, DEC) + "   ");
        printSTR(sticky, 22 + 50, 38);

        // Draw Axis
        myOLED.drawLine(xax - zax, yax, xax + zax, yax);
        myOLED.drawLine(xax, yax - zax, xax, yax + zax);
        myOLED.clrPixel(xax, yax - 80 / 4);
        myOLED.clrPixel(xax, yax + 80 / 4);
        myOLED.clrPixel(xax + 80 / 4, yax);
        myOLED.clrPixel(xax - 80 / 4, yax);

        //Draw Analog Stick
        if (mode == 1)
        {
          myOLED.setPixel(xax + N64_status.stick_x / 4, yax - N64_status.stick_y / 4);
          //Update LCD
          myOLED.update();
        }
        else
        {
          myOLED.drawCircle(xax + N64_status.stick_x / 4, yax - N64_status.stick_y / 4, 2);
          //Update LCD
          myOLED.update();
          myOLED.clrScr();
        }

        // switch mode
        if (button == "Press a button" && lastbutton == "Z")
        {
          if (mode == 0)
          {
            mode = 1;
            myOLED.clrScr();
          }
          else
          {
            mode = 0;
            myOLED.clrScr();
          }
        }
        // go to next screen
        nextscreen();
        break;
      }
    case 3:
      {
        myOLED.print("Skipping Test", CENTER, 0);
        myOLED.drawLine(22 + 0, 10, 22 + 83, 10);
        myOLED.drawRect(22 + 0, 20, 22 + 83, 44);
        if (N64_status.stick_x > 0)
          myOLED.drawLine(22 + N64_status.stick_x, 20, 22 + N64_status.stick_x, 44);

        //Update LCD
        myOLED.update();

        if (button == "Press a button" && lastbutton == "Z")
        {
          // reset button
          lastbutton = "N/A";

          myOLED.clrScr();
        }
        // go to next screen
        nextscreen();
        break;
      }
    case 4:
      {
        switch ( test )
        {
          case 0:	// Display results
            {
              switch (results)
              {
                case 0:
                  {
                    anastick = "YOURS";
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

                    if (button == "Press a button" && lastbutton == "A")
                    {
                      // reset button
                      lastbutton = "N/A";
                      results = 1;
                    }

                    break;
                  }
                case 1:
                  {
                    anastick = "ORIG";
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

                    if (button == "Press a button" && lastbutton == "A")
                    {
                      // reset button
                      lastbutton = "N/A";
                      results = 0;
                    }
                    break;
                  }

              } //results
              myOLED.clrScr();

              printSTR(anastick, 22 + 50, 0);

              myOLED.print("U:", 22 + 50, 10);
              myOLED.printNumI(upy, RIGHT, 10);
              myOLED.print("D:", 22 + 50, 20);
              myOLED.printNumI(downy, RIGHT, 20);
              myOLED.print("L:", 22 + 50, 30);
              myOLED.printNumI(leftx, RIGHT, 30);
              myOLED.print("R:", 22 + 50, 40);
              myOLED.printNumI(rightx, RIGHT, 40);

              myOLED.drawLine(xax + upx / 4, yax - upy / 4, xax + uprightx / 4, yax - uprighty / 4);
              myOLED.drawLine(xax + uprightx / 4, yax - uprighty / 4, xax + rightx / 4, yax - righty / 4);
              myOLED.drawLine(xax + rightx / 4, yax - righty / 4, xax + downrightx / 4, yax - downrighty / 4);
              myOLED.drawLine(xax + downrightx / 4, yax - downrighty / 4, xax + downx / 4, yax - downy / 4);
              myOLED.drawLine(xax + downx / 4, yax - downy / 4, xax + downleftx / 4, yax - downlefty / 4);
              myOLED.drawLine(xax + downleftx / 4, yax - downlefty / 4, xax + leftx / 4, yax - lefty / 4);
              myOLED.drawLine(xax + leftx / 4, yax - lefty / 4, xax + upleftx / 4, yax - uplefty / 4);
              myOLED.drawLine(xax + upleftx / 4, yax - uplefty / 4, xax + upx / 4, yax - upy / 4);

              myOLED.setPixel(xax, yax);

              //Update LCD
              myOLED.update();
              break;
            } //display results

          case 1:// +y Up
            {
              myOLED.print("Hold Stick Up", CENTER, 18);
              myOLED.print("then press A", CENTER, 28);
              //myOLED.drawBitmap(110, 60, ana1);

              if (button == "Press a button" && lastbutton == "A")
              {
                bupx = N64_status.stick_x;
                bupy = N64_status.stick_y;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
                test = 2;
              }
              break;
            }

          case 2:// +y+x Up-Right
            {
              myOLED.print("Up-Right", CENTER, 22 );
              //myOLED.drawBitmap(110, 60, ana2);

              if (button == "Press a button" && lastbutton == "A")
              {
                buprightx = N64_status.stick_x;
                buprighty = N64_status.stick_y;
                test = 3;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }

          case 3:// +x Right
            {
              myOLED.print("Right", CENTER, 22 );
              //myOLED.drawBitmap(110, 60, ana3);

              if (button == "Press a button" && lastbutton == "A")
              {
                brightx = N64_status.stick_x;
                brighty = N64_status.stick_y;
                test = 4;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }

          case 4:// -y+x Down-Right
            {
              myOLED.print("Down-Right", CENTER, 22 );
              //myOLED.drawBitmap(110, 60, ana4);

              if (button == "Press a button" && lastbutton == "A")
              {
                bdownrightx = N64_status.stick_x;
                bdownrighty = N64_status.stick_y;
                test = 5;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }

          case 5:// -y Down
            {
              myOLED.print("Down", CENTER, 22 );
              //myOLED.drawBitmap(110, 60, ana5);

              if (button == "Press a button" && lastbutton == "A")
              {
                bdownx = N64_status.stick_x;
                bdowny = N64_status.stick_y;
                test = 6;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }

          case 6:// -y-x Down-Left
            {
              myOLED.print("Down-Left", CENTER, 22 );
              //myOLED.drawBitmap(110, 60, ana6);

              if (button == "Press a button" && lastbutton == "A")
              {
                bdownleftx = N64_status.stick_x;
                bdownlefty = N64_status.stick_y;
                test = 7;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }

          case 7:// -x Left
            {
              myOLED.print("Left", CENTER, 22 );
              //myOLED.drawBitmap(110, 60, ana7);

              if (button == "Press a button" && lastbutton == "A")
              {
                bleftx = N64_status.stick_x;
                blefty = N64_status.stick_y;
                test = 8;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }

          case 8:// +y+x Up-Left
            {
              myOLED.print("Up-Left", CENTER, 22);
              //myOLED.drawBitmap(110, 60, ana8);

              if (button == "Press a button" && lastbutton == "A")
              {
                bupleftx = N64_status.stick_x;
                buplefty = N64_status.stick_y;
                test = 0;
                // reset button
                lastbutton = "N/A";

                myOLED.clrScr();
              }
              break;
            }
        }
        if (test != 0)
        {
          myOLED.print("Benchmark", CENTER, 0);
          myOLED.drawLine(22 + 0, 9, 22 + 83, 9);
        }
        myOLED.update();
        // go to next screen
        nextscreen();
        break;
      }
  }
}

















