#!/bin/sh

set -e

: ${ARDUINO_APP:=/Users/enami/Applications/Arduino-1.7.10.app}
TOOLDIR=${ARDUINO_APP}/Contents/Java/hardware/tools
ARDUINODIR=${ARDUINO_APP}/Contents/Java/hardware/arduino

OPENOCDDIR=${TOOLDIR}/OpenOCD-0.9.0-arduino
SAMDDIR=${ARDUINODIR}/samd
OPENOCD_CFGFILE=${SAMDDIR}/variants/arduino_zero/openocd_scripts/arduino_zero.cfg
BOOTLOADER_FILE=${1:-${SAMDDIR}/bootloaders/zero/Bootloader_D21.hex}

test -f ${OPENOCD_CFGFILE} || { echo ${OPENOCD_CFGFILE} not found 1>&2; exit 2; }
test -f ${BOOTLOADER_FILE} || { echo ${BOOTLOADER_FILE} not found 1>&2; exit 2; }

${OPENOCDDIR}/bin/openocd \
    -s ${OPENOCDDIR}/share/openocd/scripts/ \
    -f ${OPENOCD_CFGFILE} \
    -c init -c halt \
    -c "at91samd bootloader 0" \
    -c "program ${BOOTLOADER_FILE} verify" \
    -c "at91samd bootloader 16384" \
    -c "at91samd bootloader" \
    -c reset -c exit
