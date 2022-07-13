### Important: The CIC adapters are completely untested and still a work-in-progress, so the PCB files might contain errors or not work at all.    

##

The snescic_adapter is for using the original CIC chip either out of a cartridge(D411) or out of a SNES console(F411). They will be region locked and you can only dump SA1 games from the region of the CIC chip. Furthermore it has not been tested if this works at all, so right now it's just an unproven idea to save people the trouble of programming a PIC microcontroller.    

##

The snespic_adapter is for using either the PIC12F629 DIP or SMD(flashed with snescic-lock-resync.hex) or PIC16F630 DIP(flashed with supercic-lock_p16f630.hex) microcontrollers. They are region-free so they can dump SA1 cartridges from any country.     

Hexfiles:     
- [snescic-lock-resync.hex](https://github.com/mrehkopf/sd2snes/tree/develop/cic)    
- [supercic-lock_p16f630.hex](https://sd2snes.de/blog/cool-stuff/supercic)      

More Info:   
- [Flashing the snesCIC](https://github.com/sanni/cartreader/wiki/Flashing-the-snesCIC)    

