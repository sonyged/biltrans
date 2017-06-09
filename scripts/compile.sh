#!/bin/sh
#
# Copyright (c) 2017 Sony Global Education, Inc.
#

cd $(dirname $0) || exit 20

BUILDDIR=${1:-/tmp/build}
test -d "${BUILDDIR}" || exit 21

SKETCH_CPP=${2:-sketch_mar07b.cpp}

(
 if [ -n "${ENABLE_INTERP}" ]; then
     echo '#define ENABLE_INTERP 1';
 fi
 if [ -n "${INCLUDE_TRANSLATED_C}" ]; then
     echo '#define INCLUDE_TRANSLATED_C 1';
 fi
 cat ../code/firmata_base.cpp;
 cat ../code/firmata_base.h;
 echo '#define FIRMATA_BASE';
 cat ../code/trouble_shooting.h;
 if [ -n "${ENABLE_FUNCTION_TEST_01}" ]; then
     echo '#define FUNCTION_TEST_01';
     echo '#if defined(FUNCTION_TEST_01)';
     cat ../code/function_test_01.h;
     echo '#endif /* defined(FUNCTION_TEST_01) */';
 fi
 cat ../code/koov.c;
 cat ../code/listlib.h;
 if [ -n "${ENABLE_INTERP}" ]; then
     cat ../code/keyword_dict.h;
     cat ../code/insn_dict.h;
     cat ../code/interp.h;
     cat ../code/interp.c;
     cat ../code/interp_glue.c;
 fi
 cat) > ${BUILDDIR}/${SKETCH_CPP}

cp ../code/koov_version.h "${BUILDDIR}/"
sh ../scripts/build_firmata.sh "${BUILDDIR}" "${SKETCH_CPP}" || exit 22

SKETCH_BASE="${BUILDDIR}/${SKETCH_CPP}"
if [ -n "${OUTPUT_JSON}" ]; then
    cat <<EOF
{
  "sketch":
     "$(cat ${SKETCH_BASE}.hex | sed 's/$/\\n/' | tr -d '\r\n')",
  "btpin_offset":
     $(grep ^.btpin.data ${SKETCH_BASE}.map | awk '{print $2}')
}
EOF
else
    cat ${SKETCH_BASE}.hex
fi
