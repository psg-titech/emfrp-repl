#!/bin/sh

# @file   execPackcc.sh
# @brief  execPackcc.sh <packcc/build/{gcc, clang}> <src/parser.peg> <dest. of emfrp_parser.c> <dest. of emfrp_parser.h>
# @author Go Suzuki <puyogo.suzuki@gmail.com>
# @date   2023/8/13

oldpath=`pwd`
if [ ! -f $1/release/bin/packcc ]; then
    echo "Packcc has not already built. Build it."
    cd $1
    if type "gmake" > /dev/null; then
        gmake
    else
        make
    fi
    cd $oldpath
    if [ ! -f $1/release/bin/packcc ]; then
        echo "Packcc compilation failed. Aborting."
        exit 1
    else
        echo "Packcc compilation succeeded."
    fi
fi

$1/release/bin/packcc -o emfrp_parser $2
mv emfrp_parser.c $3
mv emfrp_parser.h $4
