#!/bin/sh
#
# Copyright (c) 2017 Sony Global Education, Inc.
#

set -e

: ${ARDUINO_APP:=${HOME}/Applications/Arduino-1.7.10.app}
: ${ARDUINO_ROOT:=${ARDUINO_APP}/Contents/Java}
: ${TOOLDIR:=${ARDUINO_ROOT}/hardware/tools}
: ${FIRMATA_DIR:=${HOME}/Documents/Arduino/libraries/Firmata}
ARDUINODIR=${ARDUINO_ROOT}/hardware/arduino
CROSS=${TOOLDIR}/gcc-arm-none-eabi-4.8.3-2014q1/bin/arm-none-eabi-


BUILDDIR=${1:-/tmp/build}

for v in BUILDDIR ARDUINO_APP; do
    eval "dir=\${$v}"
    test -d "${dir}" ||
	{ echo No such direcotry: ${dir} 1>&2; exit 2; }
done

mkdir -p ${BUILDDIR}/Servo/avr
mkdir -p ${BUILDDIR}/Servo/sam
mkdir -p ${BUILDDIR}/Servo/samd
mkdir -p ${BUILDDIR}/Wire
mkdir -p ${BUILDDIR}/MMA8653
mkdir -p ${BUILDDIR}/Firmata/utility

SKETCH_CPP=${BUILDDIR}/${2:-sketch_mar07b.cpp}
VARIANT_CPP=${ARDUINODIR}/samd/variants/arduino_zero/variant.cpp

#ECHO=echo
CXX="${ECHO} ${CROSS}g++"
CC="${ECHO} ${CROSS}gcc"
AR="${ECHO} ${CROSS}ar"
OBJCOPY="${ECHO} ${CROSS}objcopy"

INCLUDES="-I${TOOLDIR}/CMSIS/CMSIS/Include/
 -I${TOOLDIR}/CMSIS/Device/ATMEL/ -I${ARDUINODIR}/samd/cores/arduino
 -I${ARDUINODIR}/samd/variants/arduino_zero"

EXTRA_INCLUDES="-I${ARDUINO_ROOT}/libraries/Servo/src
 -I${ARDUINO_ROOT}/libraries/MMA8653
 -I${ARDUINODIR}/samd/libraries/Wire
 -I${FIRMATA_DIR}"

WIRE_INCLUDES=-I${ARDUINODIR}/samd/libraries/Wire/utility

FIRMATA_INCLUDES=-I${FIRMATA_DIR}/utility

USB_MANUFACTURER='Sony Corp.'
USB_PRODUCT='KOOV'
USB_VID=0x054c
USB_PID=0x0be6

DEFINES='-Dprintf=iprintf -mcpu=cortex-m0plus -DF_CPU=48000000L
 -DARDUINO=10708 -DARDUINO_SAM_ZERO -DARDUINO_ARCH_SAMD -DKOOV
 -D__SAMD21G18A__ -mthumb -DUSB_VID=${USB_VID} -DUSB_PID=${USB_PID} -DUSBCON
 -DUSB_MANUFACTURER="\"${USB_MANUFACTURER}\""
 -DUSB_PRODUCT="\"${USB_PRODUCT}\""'

CXXFLAGS="-g -Os -w -ffunction-sections -fdata-sections -nostdlib
 --param max-inline-insns-single=500 -fno-rtti -fno-exceptions
 ${DEFINES}"

CFLAGS="-g -Os -w -ffunction-sections -fdata-sections -nostdlib
 --param max-inline-insns-single=500 ${DEFINES}"

LDSCRIPTSDIR=${ARDUINODIR}/samd/variants/arduino_zero/linker_scripts

LDFLAGS='-Os -Wl,--gc-sections -save-temps -mcpu=cortex-m0plus
 -T${LDSCRIPTSDIR}/gcc/flash_with_bootloader.ld
 -Wl,-Map,${SKETCH_CPP}.map -o ${SKETCH_CPP}.elf
 --specs=nano.specs -L${BUILDDIR} -Wl,--start-group -lm -lgcc
 -Wl,--end-group -mthumb -Wl,--cref -Wl,--check-sections
 -Wl,--gc-sections -Wl,--entry=Reset_Handler
 -Wl,--unresolved-symbols=report-all -Wl,--warn-common
 -Wl,--warn-section-align -Wl,--warn-unresolved-symbols'

SERVO_SRCS="
 ${ARDUINO_ROOT}/libraries/Servo/src/avr/Servo.cpp
 ${ARDUINO_ROOT}/libraries/Servo/src/sam/Servo.cpp
 ${ARDUINO_ROOT}/libraries/Servo/src/samd/Servo.cpp"
SERVO_OBJS="
 ${BUILDDIR}/Servo/avr/Servo.cpp.o
 ${BUILDDIR}/Servo/sam/Servo.cpp.o
 ${BUILDDIR}/Servo/samd/Servo.cpp.o"

MMA8653_SRCS="
 ${ARDUINO_ROOT}/libraries/MMA8653/MMA8653.cpp"
MMA8653_OBJS="
 ${BUILDDIR}/MMA8653/MMA8653.cpp.o"

WIRE_SRCS="
 ${ARDUINODIR}/samd/libraries/Wire/Wire.cpp"
WIRE_OBJS="
 ${BUILDDIR}/Wire/Wire.cpp.o"

FIRMATA_SRCS="
 ${FIRMATA_DIR}/Firmata.cpp
 ${FIRMATA_DIR}/utility/EthernetClientStream.cpp"
FIRMATA_OBJS="
 ${BUILDDIR}/Firmata/Firmata.cpp.o
 ${BUILDDIR}/Firmata/utility/EthernetClientStream.cpp.o"

OBJS="${BUILDDIR}/syscalls.c.o ${SKETCH_CPP}.o
 ${SERVO_OBJS} ${MMA8653_OBJS} ${WIRE_OBJS} ${FIRMATA_OBJS}
 ${BUILDDIR}/variant.cpp.o ${BUILDDIR}/core.a"

# Using library Servo in folder: ${ARDUINO_ROOT}/libraries/Servo 
# Using library Wire in folder: ${ARDUINODIR}/samd/libraries/Wire (legacy)
# Using library Firmata in folder: ${FIRMATA_DIR} 

${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} \
 ${ARDUINO_ROOT}/libraries/Servo/src/avr/Servo.cpp \
 -o ${BUILDDIR}/Servo/avr/Servo.cpp.o 
${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} \
 ${ARDUINO_ROOT}/libraries/Servo/src/sam/Servo.cpp \
 -o ${BUILDDIR}/Servo/sam/Servo.cpp.o 
${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} \
 ${ARDUINO_ROOT}/libraries/Servo/src/samd/Servo.cpp \
 -o ${BUILDDIR}/Servo/samd/Servo.cpp.o 
${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} \
 ${ARDUINO_ROOT}/libraries/MMA8653/MMA8653.cpp \
 -o ${BUILDDIR}/MMA8653/MMA8653.cpp.o 
${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} ${WIRE_INCLUDES} \
 ${ARDUINODIR}/samd/libraries/Wire/Wire.cpp -o ${BUILDDIR}/Wire/Wire.cpp.o 
${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} ${FIRMATA_INCLUDES} \
 ${FIRMATA_DIR}/Firmata.cpp -o ${BUILDDIR}/Firmata/Firmata.cpp.o 
${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} ${FIRMATA_INCLUDES} \
 ${FIRMATA_DIR}/utility/EthernetClientStream.cpp \
 -o ${BUILDDIR}/Firmata/utility/EthernetClientStream.cpp.o 

LIBSRCS="${ARDUINODIR}/samd/cores/arduino/avr/dtostrf.c
 ${ARDUINODIR}/samd/cores/arduino/delay.c
 ${ARDUINODIR}/samd/cores/arduino/hooks.c
 ${ARDUINODIR}/samd/cores/arduino/itoa.c
 ${ARDUINODIR}/samd/cores/arduino/startup.c
 ${ARDUINODIR}/samd/cores/arduino/syscalls.c
 ${ARDUINODIR}/samd/cores/arduino/USB/samd21_host.c
 ${ARDUINODIR}/samd/cores/arduino/WInterrupts.c
 ${ARDUINODIR}/samd/cores/arduino/wiring.c
 ${ARDUINODIR}/samd/cores/arduino/wiring_analog.c
 ${ARDUINODIR}/samd/cores/arduino/wiring_digital.c
 ${ARDUINODIR}/samd/cores/arduino/wiring_shift.c
 ${ARDUINODIR}/samd/cores/arduino/IPAddress.cpp
 ${ARDUINODIR}/samd/cores/arduino/main.cpp
 ${ARDUINODIR}/samd/cores/arduino/Print.cpp
 ${ARDUINODIR}/samd/cores/arduino/Reset.cpp
 ${ARDUINODIR}/samd/cores/arduino/RingBuffer.cpp
 ${ARDUINODIR}/samd/cores/arduino/SERCOM.cpp
 ${ARDUINODIR}/samd/cores/arduino/Stream.cpp
 ${ARDUINODIR}/samd/cores/arduino/Tone.cpp
 ${ARDUINODIR}/samd/cores/arduino/Uart.cpp
 ${ARDUINODIR}/samd/cores/arduino/USB/CDC.cpp
 ${ARDUINODIR}/samd/cores/arduino/USB/HID.cpp
 ${ARDUINODIR}/samd/cores/arduino/USB/USBCore.cpp
 ${ARDUINODIR}/samd/cores/arduino/wiring_pulse.cpp
 ${ARDUINODIR}/samd/cores/arduino/WMath.cpp
 ${ARDUINODIR}/samd/cores/arduino/WString.cpp"

#
# Uncomment following line when compiled with 1.7.8
#
#LIBSRCS="${LIBSRCS}
# ${ARDUINODIR}/samd/cores/arduino/USB/samd21_device.c"

${CXX} -c ${CXXFLAGS} ${INCLUDES} ${EXTRA_INCLUDES} ${SKETCH_CPP} -o \
    ${SKETCH_CPP}.o

for file in ${VARIANT_CPP} ${LIBSRCS}; do
    case $file in
    *.cpp) eval ${CXX} -c ${CXXFLAGS} ${INCLUDES} $file \
		  -o ${BUILDDIR}/$(basename $file).o;;
    *.c) eval ${CC} -c ${CFLAGS} ${INCLUDES} $file \
	       -o ${BUILDDIR}/$(basename $file).o;;
    esac
done

for file in ${LIBSRCS}; do
    ${AR} rcs ${BUILDDIR}/core.a ${BUILDDIR}/$(basename $file).o
done

eval ${CXX} ${LDFLAGS} -Wl,--start-group ${OBJS} -Wl,--end-group \
    -Wl,--section-start=.text=0x4000 2> ${BUILDDIR}/linker.err
grep -q 'undefined reference' ${BUILDDIR}/linker.err && exit 2

${OBJCOPY} -O binary ${SKETCH_CPP}.elf ${SKETCH_CPP}.bin 
${OBJCOPY} -O ihex -R .eeprom ${SKETCH_CPP}.elf ${SKETCH_CPP}.hex 
