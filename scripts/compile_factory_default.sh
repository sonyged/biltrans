#!/bin/sh

export ENABLE_FUNCTION_TEST_01=yes
export ENABLE_INTERP=yes
cd $(dirname $0)

BUILDDIR=${1:-/tmp/build}
SKETCH_CPP=${2:-factory-default.cpp}

cp ../code/interp_insns.h ${BUILDDIR}/
cat ../code/interp_main.c |
    sh ./compile.sh ${BUILDDIR} ${SKETCH_CPP}
