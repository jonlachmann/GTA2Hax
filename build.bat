@echo off
reg import vscommondir.reg
call set_vc6
mkdir build_vc6
cd build_vc6
cmake.exe -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF -DSDL2_DIR=..\SDL2_Dev -DCMAKE_BUILD_TYPE=Release .. -G"NMake Makefiles JOM"
cmake --build . --target all
