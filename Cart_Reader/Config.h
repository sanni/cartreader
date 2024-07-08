/********************************************************************
                    Open Source Cartridge Reader
********************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

/***** FIRMWARE CONFIGURATION **************************************

    Add or remove the "//" in front of items to toggle them.

    Disabled:
      //#define HW5

    Enabled:
      #define HW5

    Things in ** blocks like this are comments. Changing them doesn't
    affect the firmware that is flashed to your OSCR.

    If you only get a blank screen or "Press Button" message after
    flashing you have enabled too many modules.

********************************************************************/

/*==== HARDWARE VERSION ===========================================*/

/*
    Choose your hardware version:
*/

//#define HW5
//#define HW4
//#define HW3
//#define HW2
//#define HW1
//#define SERIAL_MONITOR

/****/

/*==== HARDWARE MODULES ===========================================*/

/* [ Automatic Voltage Selection ---------------------------------- ]
    Enable this if you have the VSELECT module.
*/

//#define ENABLE_VSELECT

/****/

/* [ Clock Generator ---------------------------------------------- ]
    Enable this if you have the clock generator module. This will
    automatically be enabled if you selected HW2 or newer above.
*/

//#define ENABLE_CLOCKGEN

/****/

/* [ Real Time Clock ---------------------------------------------- ]
    Enable this if you have the RTC module. You can configure the
    type later in this file.
*/

//#define ENABLE_RTC

/****/

/*==== GAME SYSTEM CORES ==========================================*/

/* [ Atari 2600 --------------------------------------------------- ]
*/

//#define ENABLE_2600

/****/

/* [ Atari 5200 --------------------------------------------------- ]
*/

//#define ENABLE_5200

/****/

/* [ Atari 7800 --------------------------------------------------- ]
*/

//#define ENABLE_7800

/****/

/* [ Atari LYNX --------------------------------------------------- ]
*/

//#define ENABLE_LYNX

/****/

/* [ Atari 8-bit -------------------------------------------------- ]
*/

//#define ENABLE_ATARI8

/****/

/* [ Bally Astrocade ---------------------------------------------- ]
*/

//#define ENABLE_BALLY

/****/

/* [ Bandai Little Jammer ----------------------------------------- ]
*/

//#define ENABLE_LJ

/****/

/* [ Bandai Little Jammer Pro ------------------------------------- ]
*/

//#define ENABLE_LJPRO

/****/

/* [ Benesse Pocket Challenge W ----------------------------------- ]
*/

//#define ENABLE_PCW

/****/

/* [ Casio PV-1000 ------------------------------------------------ ]
*/

//#define ENABLE_PV1000

/****/

/* [ ColecoVision ------------------------------------------------- ]
*/

//#define ENABLE_COLV

/****/

/* [ Commodore 64 ------------------------------------------------- ]
*/

//#define ENABLE_C64

/****/

/* [ Commodore VIC-20 --------------------------------------------- ]
*/

//#define ENABLE_VIC20

/****/

/* [ Emerson Arcadia 2001 ----------------------------------------- ]
*/

//#define ENABLE_ARC

/****/

/* [ Fairchild Channel F ------------------------------------------ ]
*/

//#define ENABLE_FAIRCHILD

/****/

/* [ Flashrom Programmer for repro carts -------------------------- ]
*/

#define ENABLE_FLASH
//#define ENABLE_FLASH16

/****/

/* [ Game Boy (Color) and Advance --------------------------------- ]
*/

#define ENABLE_GBX

/****/

/* [ Intellivision ------------------------------------------------ ]
*/

//#define ENABLE_INTV

/****/

/* [ LeapFrog Leapster -------------------------------------------- ]
*/

//#define ENABLE_LEAP

/****/

/* [ Neo Geo Pocket ----------------------------------------------- ]
*/

//#define ENABLE_NGP

/****/

/* [ Nintendo 64 -------------------------------------------------- ]
*/

#define ENABLE_N64

/****/

/* [ Nintendo Entertainment System/Family Computer ---------------- ]
*/

#define ENABLE_NES

/****/

/* [ Magnavox Odyssey 2 ------------------------------------------- ]
*/

//#define ENABLE_ODY2

/****/

/* [ MSX ---------------------------------------------------------- ]
*/

//#define ENABLE_MSX

/****/

/* [ PC Engine/TurboGrafx 16 -------------------------------------- ]
*/

//#define ENABLE_PCE

/****/

/* [ Pokemon Mini ------------------------------------------------- ]
*/

//#define ENABLE_POKE

/****/

/* [ RCA Studio II ------------------------------------------------ ]
*/

//#define ENABLE_RCA

/****/

/* [ Sega Master System/Mark III/Game Gear/SG-1000 ---------------- ]
*/

#define ENABLE_SMS

/****/

/* [ Sega Mega Drive/Genesis -------------------------------------- ]
*/

#define ENABLE_MD

/****/

/* [ Super Famicom SF Memory Cassette ----------------------------- ]
*/

#define ENABLE_SFM

/****/

/* [ Super Famicom Satellaview ------------------------------------ ]
*/

//#define ENABLE_SV

/****/

/* [ Super Famicom Sufami Turbo ----------------------------------- ]
*/

//#define ENABLE_ST

/****/

/* [ Super Famicom Game Processor RAM Cassette -------------------- ]
*/

//#define ENABLE_GPC

/****/

/* [ Super Nintendo ----------------------------------------------- ]
*/

#define ENABLE_SNES

/****/

/* [ Texas Instruments TI-99 -------------------------------------- ]
*/

//#define ENABLE_TI99

/****/

/* [ Tomy Pyuuta -------------------------------------------------- ]
*/

//#define ENABLE_PYUUTA

/****/

/* [ TRS-80 Color Computer ---------------------------------------- ]
*/

//#define ENABLE_TRS80

/****/

/* [ Vectrex ------------------------------------------------------ ]
*/

//#define ENABLE_VECTREX

/****/

/* [ Virtual Boy -------------------------------------------------- ]
*/

//#define ENABLE_VBOY

/****/

/* [ Vtech V.Smile ------------------------------------------------ ]
*/

//#define ENABLE_VSMILE

/****/

/* [ Watara Supervision ------------------------------------------- ]
*/

//#define ENABLE_WSV

/****/

/* [ WonderSwan and Benesse Pocket Challenge v2 ------------------- ]
*/

//#define ENABLE_WS

/****/

/* [ Super A'can -------------------------------------------------- ]
*/

//#define ENABLE_SUPRACAN

/****/

/* [ Casio Loopy -------------------------------------------------- ]
*/

//#define ENABLE_LOOPY

/****/

/*==== FIRMWARE OPTIONS ===========================================*/

/* [ Config File -------------------------------------------------- ]
    Allow changing some configuration values via a config file. You
    generally can only use the config to set options or disable
    certain featuress. It cannot be used to toggle firmware options
    on, only off.

    Note For Developers: See OSCR.* for info.

    Filename: config.txt
*/

//#define ENABLE_CONFIG

/****/

/* [ LCD: Background Color ---------------------------------------- ]
    Set the backlight color of the LCD if you have one.

    Can be set using config:
      lcd.confColor=1
      lcd.red=0
      lcd.green=0
      lcd.blue=0

    PARAMETERS:
      Green, Red, Blue
*/

#define OPTION_LCD_BG_COLOR 100, 0, 0

/****/

/* [ 3.3V Stability Fix (3V3FIX) ---------------------------------- ]
    Enable this if you are having stability issues when using 3.3V,
    works best with VSELECT.

    If not using VSELECT, always turn the cart reader on with the
    voltage switch set to 5V and switch to 5V before selecting a
    cartridge from the menu.
*/

//#define ENABLE_3V3FIX

/****/

/* [ Updater ------------------------------------------------------ ]
    Disable this if you don't plan to/want to use the firmware
    updater utility. This setting is ignored on hardware versions
    other than HW5 and HW3.
*/

#define ENABLE_UPDATER

/****/

/* [ Self Test ---------------------------------------------------- ]
    Tests for shorts and other issues in your OSCR build.
*/

#define ENABLE_SELFTEST

/****/

/* [ Logging ------------------------------------------------------ ]
    Write all info to OSCR_LOG.txt in root dir

    Can be toggled off using config:
      oscr.logging=0
*/

#define ENABLE_GLOBAL_LOG

/****/

/* [ RTC: IC Type ------------------------------------------------- ]
    When the RTC module is installed, choose the type here. This
    setting is ignored if the RTC option is not enabled.
*/

#define DS3231
//#define DS1307

/****/

/* [ SNES Core/CLOCKGEN: Read Clock Generator Calibration Data ---- ]
    Toggle clock calibration menu and whether or not to use calibration data from snes_clk.txt
*/

//#define OPTION_CLOCKGEN_CALIBRATION
//#define OPTION_CLOCKGEN_USE_CALIBRATION

/****/

/* [ MegaDrive/Genesis Core: Compatibility Settings --------------- ]
    Allows you to create a text file on the SD card called
    "mdconf.txt" which you should place the following into:

    [segaSram16bit] N

    Where N is:
      0: Output each byte once. Not supported by emulators. (default)
      1: Duplicate each byte. Usable by Kega Fusion.
      2: Same as 1 + pad with 0xFF so that the file size is 64KB.

    **
    ** DEPRECATED: Use the config file instead. See below.
    **
*/

//#define use_md_conf

/*
    Configure how the MD core saves are formatted.

    Can be set using config:
      md.saveType=0

    If config is enabled, this option does nothing -- use the config.

    Options:
      0: Output each byte once. Not supported by emulators. (default)
      1: Duplicate each byte. Usable by Kega Fusion.
      2: Same as 1 + pad with 0xFF so that the file size is 64KB.
*/

//#define OPTION_MD_DEFAULT_SAVE_TYPE 0

/****/

/* [ N64 Core: Fast CRC ------------------------------------------- ]
    Toggle so the CRC for N64 Roms will be calculated during dumping
    from memory instead of after dumping from SD card, not compatible
    with all Cart Readers
*/

//#define OPTION_N64_FASTCRC

/****/

/* [ N64 Core: Log Summary ---------------------------------------- ]
    Enable to save a n64log.txt file with rom info in /N64/ROM
*/

//#define OPTION_N64_SAVESUMMARY

/****/

/*==== PROCESSING =================================================*/

/*
              You probably shouldn't change this stuff!
*/

#if defined(ENABLE_CONFIG)
#define CONFIG_FILE "config.txt"
// Define the max length of the key=value pairs
// Do your best not to have to increase these.
#define CONFIG_KEY_MAX 32
#define CONFIG_VALUE_MAX 32
#endif

#if (defined(HW4) || defined(HW5))
#define ENABLE_LCD
#define ENABLE_NEOPIXEL
#define ENABLE_ROTARY
//# define rotate_counter_clockwise
#define ENABLE_CLOCKGEN
#define OPTION_N64_FASTCRC
#define OPTION_WS_ADAPTER_V2
#endif

#if (defined(HW2) || defined(HW3))
#define ENABLE_OLED
#define ENABLE_BUTTON2
#define ENABLE_CLOCKGEN
#define ENABLE_CA_LED
#define OPTION_N64_FASTCRC
#endif

#if defined(HW1)
#define ENABLE_OLED
//#define ENABLE_CLOCKGEN
//#define OPTION_N64_FASTCRC
#endif

#if defined(SERIAL_MONITOR)
#define ENABLE_SERIAL
//#define ENABLE_CLOCKGEN
//#define OPTION_N64_FASTCRC
#endif

/* Firmware updater only works with HW3 and HW5 */
#if !(defined(HW5) || defined(HW3))
#undef ENABLE_UPDATER
#endif

/* End of settings */

#endif /* CONFIG_H_ */
