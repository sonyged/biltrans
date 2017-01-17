#!/bin/sh
#
# Copyright (c) 2017 Sony Global Education, Inc.
#

# OSX default values
: ${ARDUINO_APP:=${HOME}/Applications/Arduino-1.7.10.app}
: ${ARDUINO_ROOT:=${ARDUINO_APP}/Contents/Java}
: ${TOOLDIR:=${ARDUINO_ROOT}/hardware/tools}
: ${FIRMATA_DIR:=${HOME}/Documents/Arduino/libraries/Firmata}

export ARDUINO_APP
export ARDUINO_ROOT
export TOOLDIR
export FIRMATA_DIR

exec sh -x $(dirname $0)/compile_on_ap.sh "$1"

