#!/bin/sh
$1 -o parser $2
mv parser.c $3
mv parser.h $4
