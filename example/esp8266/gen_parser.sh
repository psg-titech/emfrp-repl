#!/bin/sh
PROJDIR=`dirname $0`/../..
if [ -e $PROJDIR/packcc/build/clang/release/bin/packcc ]; then
  PACKCC_BUILD_DIR=$PROJDIR/packcc/build/clang/release
else
  PACKCC_BUILD_DIR=$PROJDIR/packcc/build/gcc/release
fi
$PROJDIR/project/common/execPackcc.sh $PACKCC_BUILD_DIR/bin/packcc  $PROJDIR/src/parser.peg $PROJDIR/src/emfrp_parser.c $PROJDIR/include/emfrp_parser.h



