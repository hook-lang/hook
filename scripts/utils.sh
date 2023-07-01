#!/usr/bin/env bash

DEFAULT_BUILD_TYPE="Debug"

cmake_build() {
  build_type="$1"
  with_extensions="$2"
  install_prefix="$3"

  if [ -z "$build_type" ]; then
    build_type="$DEFAULT_BUILD_TYPE"
  fi
  if [ "$with_extensions" == "with-extensions" ]; then
    with_extensions="-DBUILD_EXTENSIONS=1"
  else
    with_extensions=""
  fi

  if [ -z "$install_prefix" ]; then
    cmake -B build -DCMAKE_BUILD_TYPE=$build_type $with_extensions
  else
    cmake -B build -DCMAKE_BUILD_TYPE=$build_type $with_extensions -DCMAKE_INSTALL_PREFIX=$install_prefix
  fi

  cmake --build build
}

cmake_build_and_install() {
  build_type="$1"
  with_extensions="$2"
  install_prefix="$3"
  cmake_build $build_type $with_extensions $install_prefix
  cmake --install build
}

cmake_build_and_pack() {
  build_type="$1"
  with_extensions="$2"
  cmake_build $build_type $with_extensions
  cpack --config build/CPackConfig.cmake
}
