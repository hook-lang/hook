#!/usr/bin/env bash

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cpack --config build/CPackConfig.cmake
