@echo off

cmake -B build
cmake --build build --config Release
cpack --config build\CPackConfig.cmake

