#!/usr/bin/env bash

FILENAME="src/hk_version.h"
REVISION="$(git rev-parse --short HEAD)"

echo "#ifndef HK_VERSION_H" > $FILENAME
echo "#define HK_VERSION_H" >> $FILENAME
echo "#define HK_VERSION \"0.1.0\"" >> $FILENAME
echo "#define HK_REVISION \"($REVISION)\"" >> $FILENAME
echo "#endif" >> $FILENAME

cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_EXTENSIONS=On
cmake --build build
cpack --config build/CPackConfig.cmake

git checkout -- src/hk_version.h
