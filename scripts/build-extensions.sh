#!/usr/bin/env bash

cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXTENSIONS=On
cmake --build build
