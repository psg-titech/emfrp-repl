REM @file   execPackcc.sh
REM @brief  execPackcc.sh <packcc/build/msvc> <src/parser.peg> <dest. of emfrp_parser.c> <dest. of emfrp_parser.h>
REM @author Go Suzuki <puyogo.suzuki@gmail.com>
REM @date   2023/8/13

set MSBUILD=msbuild
where %MSBUILD%
if %ERRORLEVEL% NEQ 0 (
  for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    set MSBUILD=%%i
  )
)

if not exist %1/x64/Release/packcc.exe (
   echo "Packcc has not already built. Build it."
   "%MSBUILD%" %1/msvc.sln /t:build /p:configuration=release /p:Platform="x64"
   if not exist %1/x64/Release/packcc.exe (
      echo "Packcc compilation failed. Aborting."
      exit 1
   ) else (
      echo "Packcc compilation succeeded."
   )
)
%1/x64/Release/packcc.exe -o emfrp_parser %2
move emfrp_parser.c %3
move emfrp_parser.h %4
