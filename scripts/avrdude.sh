#!/bin/sh

: ${ARDUINO_APP:=/Users/enami/Applications/Arduino-1.7.8.app}
TOOLDIR=${ARDUINO_APP}/Contents/Java/hardware/tools
AVRDIR=${TOOLDIR}/avr
HEX=${1:-/tmp/build/sketch_mar07b.cpp.hex}
DEVICE=${2:-/dev/tty.usbmodem0041}

${AVRDIR}/bin/avrdude -C${AVRDIR}/etc/avrdude.conf -v -v \
	-patmega2560 -cstk500v2 -P${DEVICE} -b57600 \
	-Uflash:w:${HEX}:i

exit 0

bash-3.2$ sh -x ./avrdude.sh
+ /Users/enami/Applications/Arduino-1.7.8.app/Contents/Java/hardware/tools/avr/bin/avrdude -C/Users/enami/Applications/Arduino-1.7.8.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf -v -v -patmega2560 -cstk500v2 -P/dev/tty.usbmodem0041 -b57600 -Uflash:w:/tmp/Blink.cpp.hex:i

avrdude: Version 6.0.1, compiled on Jan 15 2015 at 12:42:51
         Copyright (c) 2000-2005 Brian Dean, http://www.bdmicro.com/
         Copyright (c) 2007-2009 Joerg Wunsch

         System wide configuration file is "/Users/enami/Applications/Arduino-1.7.8.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf"
         User configuration file is "/Users/enami/.avrduderc"
         User configuration file does not exist or is not a regular file, skipping

         Using Port                    : /dev/tty.usbmodem0041
         Using Programmer              : stk500v2
         Overriding Baud Rate          : 57600
         AVR Part                      : ATmega2560
         Chip Erase delay              : 9000 us
         PAGEL                         : PD7
         BS2                           : PA0
         RESET disposition             : dedicated
         RETRY pulse                   : SCK
         serial program mode           : yes
         parallel program mode         : yes
         Timeout                       : 200
         StabDelay                     : 100
         CmdexeDelay                   : 25
         SyncLoops                     : 32
         ByteDelay                     : 0
         PollIndex                     : 3
         PollValue                     : 0x53
         Memory Detail                 :

                                  Block Poll               Page                       Polled
           Memory Type Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack
           ----------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------
           eeprom        65    10     8    0 no       4096    8      0  9000  9000 0x00 0x00
           flash         65    10   256    0 yes    262144  256   1024  4500  4500 0x00 0x00
           lfuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           hfuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           efuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           lock           0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
           calibration    0     0     0    0 no          1    0      0     0     0 0x00 0x00
           signature      0     0     0    0 no          3    0      0     0     0 0x00 0x00

         Programmer Type : STK500V2
         Description     : Atmel STK500 Version 2.x firmware
         Programmer Model: AVRISP
         Hardware Version: 3
         Firmware Version Master : 4.05
         Vtarget         : 0.0 V
         SCK period      : 0.1 us

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e9801
avrdude: safemode: lfuse reads as 0
avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: NOTE: "flash" memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: reading input file "/tmp/Blink.cpp.hex"
avrdude: writing flash (28684 bytes):

Writing | ################################################## | 100% 0.00s

avrdude: 28684 bytes of flash written
avrdude: verifying flash memory against /tmp/Blink.cpp.hex:
avrdude: load data flash data from input file /tmp/Blink.cpp.hex:
avrdude: input file /tmp/Blink.cpp.hex contains 28684 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 0.00s

avrdude: verifying ...
avrdude: 28684 bytes of flash verified

avrdude: safemode: lfuse reads as 0
avrdude: safemode: hfuse reads as 0
avrdude: safemode: efuse reads as 0
avrdude: safemode: Fuses OK (H:00, E:00, L:00)

avrdude done.  Thank you.

bash-3.2$
