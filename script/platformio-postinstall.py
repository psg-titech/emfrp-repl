#!/usr/local/bin/python

# @file   platformio-postinstall.py
# @brief  For platform i/o, executes packcc
# @author Go Suzuki <puyogo.suzuki@gmail.com>
# @date   2023/8/27

import os
import shutil

curDir = os.path.dirname(__file__)
projDir = os.path.join(curDir, '..')
parserPeg = os.path.join(projDir, 'src', 'parser.peg')
emfrpParserC = os.path.join(projDir, 'src', 'emfrp_parser.c')
emfrpParserH = os.path.join(projDir, 'include', 'emfrp_parser.h')
if os.name == 'nt' or os.name == 'ce':
  print(os.path.join(curDir, 'execPackcc.bat') + ' ' + os.path.join(projDir, 'packcc', 'build', 'msvc') + ' ' + parserPeg + ' ' + emfrpParserC + ' ' + emfrpParserH)
  os.system(os.path.join(curDir, 'execPackcc.bat') + ' ' + os.path.join(projDir, 'packcc', 'build', 'msvc') + ' ' + parserPeg + ' ' + emfrpParserC + ' ' + emfrpParserH)
else:
  buildDir = os.path.join(projDir, 'packcc', 'build', 'gcc')
  if shutil.which('gcc') == None:
    buildDir = os.path.join(projDir, 'packcc', 'build', 'clang')
  os.system(os.path.join(curDir, 'execPackcc.sh') + ' ' + buildDir + ' ' + parserPeg + ' ' + emfrpParserC + ' ' + emfrpParserH)

if not os.path.exists(emfrpParserC):
  exit(1)