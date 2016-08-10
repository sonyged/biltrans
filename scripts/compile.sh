#!/bin/sh

cd $(dirname $0) || exit 2

BUILDDIR=${1:-/tmp/build}
test -d "${BUILDDIR}" || exit 2

SKETCH_CPP=${2:-sketch_mar07b.cpp}

(cat ../code/firmata_base.cpp;
 cat ../code/firmata_base.h;
 cat ../code/koov.c;
 cat) > ${BUILDDIR}/${SKETCH_CPP}

sh ../scripts/build_firmata.sh "${BUILDDIR}" "${SKETCH_CPP}" || exit 2
cat ${BUILDDIR}/${SKETCH_CPP}.hex

