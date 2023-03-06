@echo off

set FILENAME=src\version.h
set REVISION=git rev-parse --short HEAD

echo "#ifndef VERSION_H" > $FILENAME
echo "#define VERSION_H" >> $FILENAME
echo "#define VERSION ""0.1.0""" >> $FILENAME
echo "#define REVISION ""(%REVISION%)\""" >> $FILENAME
echo "#endif" >> $FILENAME

cmake -B build
cmake --build build --config Release
cpack --config build\CPackConfig.cmake

git checkout -- src\version.h
