@echo off


clang main.cpp -Wall -Wno-unused-function -Wno-missing-braces -fno-exceptions -fdiagnostics-absolute-paths -g -I".." -o main.exe -Wl,user32.lib