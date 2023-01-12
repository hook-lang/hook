@echo off

set FILENAME=src\hk_version.h
set REVISION=git rev-parse --short HEAD

echo "#ifndef HK_VERSION_H" > $FILENAME
echo "#define HK_VERSION_H" >> $FILENAME
echo "#define HK_VERSION ""0.1.0""" >> $FILENAME
echo "#define HK_REVISION ""(%REVISION%)\""" >> $FILENAME
echo "#endif" >> $FILENAME

cmake -B build -DCMAKE_TOOLCHAIN_FILE=%HOMEDRIVE%%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake -DBUILD_EXTENSIONS=On
cmake --build build --config Release

git checkout -- src\hk_version.h
