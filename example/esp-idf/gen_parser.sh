#!/bin/sh
PROJDIR=`dirname $0`/../..
if ! type clang > /dev/null; then
  PACKCC_BUILD_DIR=$PROJDIR/packcc/build/clang/
else
  PACKCC_BUILD_DIR=$PROJDIR/packcc/build/gcc/
fi
$PROJDIR/project/common/execPackcc.sh $PACKCC_BUILD_DIR  $PROJDIR/src/parser.peg $PROJDIR/src/emfrp_parser.c $PROJDIR/include/emfrp_parser.h



