#!/bin/sh

export ENABLE_FUNCTION_TEST_01=
export ENABLE_INTERP=yes
cd $(dirname $0)

BUILDDIR=${1:-/tmp/build}
SKETCH_CPP=${2:-interp.cpp}

#node ../compile.js ../example/empty.json |
#node ../embed_json.js ../example/empty.json |
cat ../code/interp_main.c |
    sh ./compile.sh ${BUILDDIR} ${SKETCH_CPP}
