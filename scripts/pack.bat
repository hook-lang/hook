@echo off

set FILENAME=src\hk_version.h
set REVISION=git rev-parse --short HEAD

echo "#ifndef HK_VERSION_H" > $FILENAME
echo "#define HK_VERSION_H" >> $FILENAME
echo "#define HK_VERSION ""0.1.0""" >> $FILENAME
echo "#define HK_REVISION ""(%REVISION%)\""" >> $FILENAME
echo "#endif" >> $FILENAME

cmake -B build
cmake --build build --config Release
cpack --config build\CPackConfig.cmake

git checkout -- src\hk_version.h
