#!/usr/bin/env bash

cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_EXTENSIONS=On
cmake --build build
