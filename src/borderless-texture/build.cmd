@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0"
if not exist build mkdir build
cl /nologo /std:c++17 /EHsc /W4 /WX /O2 /MT /DUNICODE /D_UNICODE /LD d3d9_proxy.cpp /Fobuild\proxy.obj /link /DEF:d3d9.def /OUT:build\d3d9.dll /IMPLIB:build\d3d9.lib user32.lib bcrypt.lib
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /WX /O2 /MT /DUNICODE /D_UNICODE tests.cpp /Fobuild\tests.obj /Febuild\tests.exe /link user32.lib bcrypt.lib
if errorlevel 1 exit /b 1
build\tests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /WX /O2 /MT engine-tests.cpp /Fobuild\engine-tests.obj /Febuild\engine-tests.exe /link bcrypt.lib
if errorlevel 1 exit /b 1
build\engine-tests.exe
if errorlevel 1 exit /b 1
dumpbin /exports build\d3d9.dll >build\exports.txt
dumpbin /headers build\d3d9.dll >build\headers.txt
cl /nologo /std:c++17 /EHsc /W4 /WX /O2 /MT smoke.cpp /Fobuild\smoke.obj /Febuild\smoke.exe
if errorlevel 1 exit /b 1
build\smoke.exe "%~dp0build\d3d9.dll"
if errorlevel 1 exit /b 1
