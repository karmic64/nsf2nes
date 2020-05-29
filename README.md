# nsf2nes
No-frills NSF rip to NES ROM converter.


## Usage
First, know the limitations of the NSFs you can use.
- The NSF may not use bankswitching.
- The NSF may not use expansion sound.
- There must be at least one free page of memory for the 6502 driver code.
If any of these conditions are not satisfied, an error message will be printed and no ROM will be written.

Usage is nothing more than `nsf2nes nsffile`. If everything went well, an NES ROM will be created with the same filename as the NSF with `.nes` appended to it.

When you load the ROM, use the D-Pad to step through songs and any other button to restart the current one.


## Build info
If you want to modify the driver, it's in [64tass](http://tass64.sourceforge.net) syntax. `64tass -f -o drv.bin drv.asm` to assemble it. Keep it under 250 bytes.

The method I use of including the driver in the executable might be incompatible with some compilers. If you have any problems, please contact me.


## To-do
- Support optimized NSFs which don't actually use bankswitching for anything but reducing the filesize
- Detection of bloated rips that simply fill all of $8000-$FFFF, placing driver in an unused page
- Mapper 31-specific driver for full NSF bankswitching (maybe)