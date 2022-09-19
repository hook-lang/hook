cmake -B build -DCMAKE_TOOLCHAIN_FILE=%HOMEDRIVE%%HOMEPATH%\vcpkg\scripts\buildsystems\vcpkg.cmake -DBUILD_EXTENSIONS=On
cmake --build build
