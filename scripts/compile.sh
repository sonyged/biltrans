#!/bin/sh

cd $(dirname $0) || exit 20

BUILDDIR=${1:-/tmp/build}
test -d "${BUILDDIR}" || exit 21

SKETCH_CPP=${2:-sketch_mar07b.cpp}

(cat ../code/firmata_base.cpp;
 cat ../code/firmata_base.h;
 echo '#define FIRMATA_BASE';
 cat ../code/trouble_shooting.h;
 cat ../code/koov.c;
 cat ../code/listlib.h;
 cat) > ${BUILDDIR}/${SKETCH_CPP}

sh ../scripts/build_firmata.sh "${BUILDDIR}" "${SKETCH_CPP}" || exit 22
cat ${BUILDDIR}/${SKETCH_CPP}.hex
