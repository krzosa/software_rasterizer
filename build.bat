@echo off

pushd %~dp0
clang main.cpp -O2 -mfma -mavx2 -Wall -Wno-unused-function -Wno-missing-braces -fno-exceptions -fdiagnostics-absolute-paths -I".." -g -o main.exe -Wl,user32.lib
popd