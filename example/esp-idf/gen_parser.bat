@echo off
set PROJDIR=%~dp0..\..
%PROJDIR%\script\execPackcc.bat %PROJDIR%\packcc\build\msvc\  %PROJDIR%\src\parser.peg %PROJDIR%\src\emfrp_parser.c %PROJDIR%\include\emfrp_parser.h


