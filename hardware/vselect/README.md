### Important: The Vselect module PCB is currently untested and may require design revisions to function.

##

The Vselect module automates voltage selection on HW5. It is to be soldered in place of the voltage selection switch. You cannot use the Vselect module and a power LED at the same time.

## Ordering the PCB and Installing the components

The Vselect module requires SMD/SMC (surface-mounted devices/components) soldering. The pads have been extended slightly to aid in hand-soldering. However, because many people are uncomfortable with hand-soldering such small components automated assembly files suitable for use with JLCPCB have been included. This does significantly increase the cost of the module, however. To use assembly on JLCPCB, upload the gerber as normal and then select to include assembly. On the next page upload `VSelect_bom.csv` and `VSelect-top-pos.csv`. Ensure the part placement looks sane. You may need to manually rotate components. Make sure the dots line up on the chip as well. It should look like this:

![](https://i.imgur.com/SJk2NCl.png)

## Installation

To install the module place it over the voltage switch contacts with the component side facing up and solder it in place. You can use stripped wires or pins to bridge between the boards. Once soldered in place you must jump 2 wires off of the board. JP1 should go to pin D38 on the Arduino. The nearest point to jump to is where the resistor for the power LED normally would be installed. Use the rightmost pin next to the screw terminal. After that, you must connect JP2 to ground. There are many places you could tap into ground but the closest is the screw terminal, just be careful not to close the hole which would get in the way of installing a screw. If you want an easier location, the unused screw terminal under the Arduino is probably the easiest.

## Configuration

The module requires code to function properly. The OSCR should still power on even without the code changes but the voltage will not automatically toggle. To enable automatic toggling, locate this bit of code:

```
//******************************************
// HW CONFIGS
//******************************************
#if (defined(HW4) || defined(HW5))
// #define enable_vselect
#define enable_LCD
```

...and change it to:

```
//******************************************
// HW CONFIGS
//******************************************
#if (defined(HW4) || defined(HW5))
#define enable_vselect
#define enable_LCD
```

After that simply update/program/flash your Arduino/OSCR as normal. Make sure you enable this every time you update. You can test that voltage switching is occurring with a multimeter by checking between ground and VCC.

## Notes
The module starts in 5V mode and switches to 3V after the Arduino finishes booting. Because of this, you should not install 3V carts before powering the system on. Once on the main menu, the system should be running on 3V.

The flash function does not currently have a menu to select a voltage. One should be added at a later time.

## Troubleshooting
If the 3.3V rail is not powered (i.e. you forgot to solder the jumper on the LCD) then when the module tries to switch to that it will power off the VCC rail. This will appear as a reboot loop where you'll see it briefly power on and then immediately power off only to immediately power on again. This is because the module starts in 5V mode and switches to 3V after the system boots and tells it to do so.
