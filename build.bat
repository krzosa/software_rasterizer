@echo off


rem clang assets.cpp -Wall -Wno-unused-function -Wno-missing-braces -fno-exceptions -fdiagnostics-absolute-paths -g -I".." -o assets.exe -Wl,user32.lib
rem assets.exe
rem tracy/TracyClient.cpp -DTRACY_ENABLE


clang main.cpp -mavx2 -Wall -Wno-unused-function -Wno-missing-braces -fno-exceptions -fdiagnostics-absolute-paths -g -I".." -o main.exe -Wl,user32.lib -Wl,optick\lib\x64\release\OptickCore.lib