@echo off
set PROJDIR=%~dp0..\..
%PROJDIR%\project\common\execPackcc.bat %PROJDIR%\packcc\build\msvc\x64\Release\packcc  %PROJDIR%\src\parser.peg %PROJDIR%\src\emfrp_parser.c %PROJDIR%\include\emfrp_parser.h


