@echo off

pushd %~dp0
clang sf_main.cpp -O2 -mfma -mavx2 -Wall -Wno-unused-function -Wno-missing-braces -fno-exceptions -fdiagnostics-absolute-paths -Wno-deprecated-declarations -g -o main.exe -Wl,user32.lib
popd