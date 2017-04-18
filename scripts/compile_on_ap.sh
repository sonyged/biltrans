#!/bin/sh
#
# Copyright (c) 2017 Sony Global Education, Inc.
#

[ $# -gt 0 ] || exit 10
test -f "$1" || exit 11

case "$1" in
/*) JSON="$1";;
*) JSON=$(pwd)/"$1"
esac

cd $(dirname $0) || exit 12

# Linux default values.
: ${ARDUINO_ROOT=/usr/local/arduino-osx/Contents/Java}
: ${TOOLDIR=/usr/local/arduino-1.7.10-linux64/hardware/tools}
: ${FIRMATA_DIR=/usr/local/firmata-arduino/Firmata}
: ${ARDUINO_APP=/usr/local/arduino-1.7.10-linux64}

export ARDUINO_APP
export ARDUINO_ROOT
export TOOLDIR
export FIRMATA_DIR

# make sure to use recent node.
PATH=/usr/local/bin:$PATH

BUILDDIR=/tmp/build.$$
rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}"

test -d "${BUILDDIR}" || exit 13

exec 2>> "${BUILDDIR}/stderr.log"
node --version >>"${BUILDDIR}"/node.log
cp -p "${JSON}" "${BUILDDIR}/koov.json"

TRANSLATED="${BUILDDIR}/translated.c"
# If version does not match, embed.js exits with 15.
node ./embed.js "${JSON}" 2>>"${BUILDDIR}"/node.log >"${TRANSLATED}" ||
    exit $?
test -s "${TRANSLATED}" || exit 14

# Don't include function_test_01
export ENABLE_FUNCTION_TEST_01=
# Include interpreter
export ENABLE_INTERP=yes
cp ../code/interp_insns.h ${BUILDDIR}/
cat ../code/interp_main.c |
    sh -x ./compile.sh "${BUILDDIR}"
