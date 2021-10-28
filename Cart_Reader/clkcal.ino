//******************************************
// Clock Calibration Module
//******************************************

#ifdef clockgen_calibration
#include <FreqCount.h>
#include "snes_clk.h"
#include "SdFat.h"

/******************************************
   Variables
 *****************************************/
int32_t cal_factor = 0;
int32_t old_cal = 0;
int32_t cal_offset = 100;


/******************************************
   Clock Calibration
 *****************************************/
void clkcal()   {
  // Adafruit Clock Generator
  // last number is the clock correction factor which is custom for each clock generator
  cal_factor = readClockOffset();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Read correction: ");
  display.println(cal_factor);
  display.display();
  delay(500);

  if (cal_factor > INT32_MIN) {
    clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, cal_factor);
  } else {
    clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    cal_factor = 0;
  }
  //clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
  //clockgen.pll_reset(SI5351_PLLA);
  //clockgen.pll_reset(SI5351_PLLB);
  clockgen.set_freq(400000000ULL, SI5351_CLK0);
  clockgen.set_freq(100000000ULL, SI5351_CLK1);
  clockgen.set_freq(307200000ULL, SI5351_CLK2);
  clockgen.output_enable(SI5351_CLK1, 1);
  clockgen.output_enable(SI5351_CLK2, 1);
  clockgen.output_enable(SI5351_CLK0, 1);

  // Frequency Counter
  delay(500);
  FreqCount.begin(1000);
  while (1) 
  {
    if (old_cal != cal_factor) {
      display_Clear();
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F("     Adjusting"));
      display_Update();
      clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
      clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
      clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
      clockgen.pll_reset(SI5351_PLLA);
      clockgen.pll_reset(SI5351_PLLB);
      clockgen.set_freq(400000000ULL, SI5351_CLK0);
      clockgen.set_freq(100000000ULL, SI5351_CLK1);
      clockgen.set_freq(307200000ULL, SI5351_CLK2);
      old_cal = cal_factor;
      delay(500);
    }
    else {
      clockgen.update_status();
      while (clockgen.dev_status.SYS_INIT == 1) {
      }
  
      if (FreqCount.available()) {
        float count = FreqCount.read();
        display_Clear();
        println_Msg(F("Clock Calibration"));
        println_Msg(F(""));
        print_Msg(F("Freq:   "));
        print_Msg(count);
        println_Msg(F("Hz"));
        print_Msg(F("Correction:"));
        print_right(cal_factor);
        print_Msg(F("Adjustment:"));
        print_right(cal_offset);
#ifdef enable_Button2
        println_Msg(F("(Hold button to save)"));
        println_Msg(F(""));
        println_Msg(F("Decrease     Increase"));
#else
  #ifdef enable_rotary
        println_Msg(F("Rotate to adjust"));
  #else
        println_Msg(F("Click/dbl to adjust"));
  #endif
#endif
        display_Update();
      }
  #ifdef enable_Button2
      // get input button
      int a = checkButton1();
      int b = checkButton2();
  
      // if the cart readers input button is pressed shortly
      if (a == 1) {
        old_cal = cal_factor;
        cal_factor -= cal_offset;
      }
      if (b == 1) {
        old_cal = cal_factor;
        cal_factor += cal_offset;
      }
  
      // if the cart readers input buttons is double clicked
      if (a == 2) {
        cal_offset /= 10ULL;
        if (cal_offset < 1)
        {
          cal_offset = 100000000ULL;
        }
      }
      if (b == 2) {
        cal_offset *= 10ULL;
        if (cal_offset > 100000000ULL)
        {
          cal_offset = 1;
        }
      }
  
      // if the cart readers input button is pressed long
      if (a == 3) {
        savetofile();
      }
      if (b == 3) {
        savetofile();
      }
#else
    //Handle inputs for either rotary encoder or single button interface.
    int a = checkButton();
    
    if (a == 1) { //clockwise rotation or single click
      old_cal = cal_factor;
      cal_factor += cal_offset;
    }

    if (a == 2) {  //counterclockwise rotation or double click
      old_cal = cal_factor;
      cal_factor -= cal_offset;
    }

    if (a == 3) { //button short hold
       cal_offset *= 10ULL;
        if (cal_offset > 100000000ULL)
        {
          cal_offset = 1;
        }
    }

    if (a == 4) { //button long hold
      savetofile();
    }
#endif
    }
  }
}

void print_right(int32_t number)
{
  int32_t abs_number = number;
  if (abs_number < 0)
    abs_number *= -1;
  else
    print_Msg(F(" "));

  if (abs_number == 0)
    abs_number = 1;
  while(abs_number < 100000000ULL)
  {
    print_Msg(F(" "));
    abs_number *= 10ULL;
  }
  println_Msg(number);
}

void savetofile() {
  display_Clear();
  println_Msg(F("Saving..."));
  println_Msg(cal_factor);
  display_Update();
  delay(2000);

  if (!myFile.open("/snes_clk.txt", O_WRITE | O_CREAT | O_TRUNC)) {
    print_Error(F("SD Error"), true);
  }
  // Write calibration factor to file
  myFile.print(cal_factor);

  // Close the file:
  myFile.close();
  println_Msg(F("Done"));
  display_Update();
  delay(1000);
  resetArduino();
}

#endif
//******************************************
// End of File
//******************************************
