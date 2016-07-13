#!/bin/sh

cd $(dirname $0) || exit 1

export ARDUINO_ROOT=/usr/local/arduino-osx/Contents/Java
export TOOLDIR=/usr/local/arduino-1.7.10-linux64/hardware/tools
export EXTRA_LIBDIR=/usr/local/firmata-arduino
export FIRMATA_DIR=/usr/local/firmata-arduino/Firmata
export ARDUINO_APP=/usr/local/arduino-1.7.10-linux64

BUILDDIR=/tmp/build.$$
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"

test -f "$1" || exit 1
node ../compile.js "$1" | sh ./compile.sh "${BUILDDIR}"