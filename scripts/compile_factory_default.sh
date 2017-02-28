#!/bin/sh

export ENABLE_FUNCTION_TEST_01=yes
cd $(dirname $0)

node ../compile.js ../example/empty.json | sh ./compile.sh
