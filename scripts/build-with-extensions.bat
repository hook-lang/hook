@echo off

cmake -B build -DCMAKE_TOOLCHAIN_FILE=%HOMEDRIVE%%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake -DBUILD_EXTENSIONS=1
cmake --build build --config Debug
