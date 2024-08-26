# VSELECT: Automatic Voltage Selection Module

The VSELECT module automates voltage selection on HW5 by allowing the firmware to set the voltage. The module version is intended to be soldered in place of the voltage selection switch. You cannot use the VSELECT module and a power LED at the same time.

## Ordering the PCB and Installing the components

The VSELECT module requires SMD/SMC (surface-mounted devices/components) soldering. The pads have been extended slightly to aid in hand-soldering. While files suitable for use with JLCPCB's PCBA service have been included, these are unsuitable for most people to use due to the minimum order quantity. If you want fewer than 10 modules it will be cheaper to order them from someone selling pre-built ones.

## Installation

Instructions to build as well as to install the module have moved [to the wiki](https://github.com/sanni/cartreader/wiki/Automatic-Voltage-Selection).

## Configuration

The module requires code to function properly. The OSCR should still power on even without the code changes but the voltage will not automatically toggle. To enable automatic toggling, locate this bit of code:

```
/*==== HARDWARE MODULES ===========================================*/

/* [ Automatic Voltage Selection ---------------------------------- ]
    Enable this if you have the VSELECT module.
*/

//#define ENABLE_VSELECT
```

...and change it to:

```
/*==== HARDWARE MODULES ===========================================*/

/* [ Automatic Voltage Selection ---------------------------------- ]
    Enable this if you have the VSELECT module.
*/

#define ENABLE_VSELECT
```

After that simply update/program/flash your Arduino/OSCR as normal. Make sure you enable this every time you update. You can test that voltage switching is occurring with a multimeter by checking between ground and VCC.

## Updating
When the OSCR is using VSELECT-enabled firmware there are a few extra steps you need to take when flashing firmware. First, before connecting the OSCR to a computer, remove the SD card. Next, connect the OSCR to your computer and wait. Once it prompts you, press the button to set the voltage to 5V. Although you can flash the ATmega2560 while running at 3.3V it does not always work and seems more likely to cause the firmware to randomly be corrupt (sometimes right away, sometimes not for a few days). Although you can just reflash the firmware to it to resolve it, having to deal with it is rather inconvenient.

## Troubleshooting
If the 3.3V rail is not powered (i.e. you forgot to solder the jumper on the LCD) then when the module tries to switch to that it will power off the VCC rail. This will appear as a reboot loop where you'll see it briefly power on and then immediately power off only to immediately power on again. This is because the module starts in 5V mode and switches to 3.3V after the system boots and tells it to do so.
