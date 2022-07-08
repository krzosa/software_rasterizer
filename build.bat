@echo off

pushd %~dp0
clang main.cpp -O2  -mavx2 -Wall -Wno-unused-function -Wno-missing-braces -fno-exceptions -fdiagnostics-absolute-paths -g -o main.exe -Wl,user32.lib
popd