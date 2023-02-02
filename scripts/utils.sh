#!/usr/bin/env bash

VERSION_FILENAME="src/hk_version.h"
DEFAULT_BUILD_TYPE="Debug"
DEFAULT_INSTALL_PREFIX="/opt/hook"

info() {
  echo "$@"
}

warning() {
  echo "$@" >&2
}

fatal_error() {
  warning "$@"
  exit 1
}

update_revision() {
  revision="$(git rev-parse --short HEAD)"
  echo "#ifndef HK_VERSION_H" > $VERSION_FILENAME
  echo "#define HK_VERSION_H" >> $VERSION_FILENAME
  echo "#define HK_VERSION \"0.1.0\"" >> $VERSION_FILENAME
  echo "#define HK_REVISION \"($revision)\"" >> $VERSION_FILENAME
  echo "#endif" >> $VERSION_FILENAME
}

discard_changes() {
  git checkout -- "$VERSION_FILENAME"
}

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
    install_prefix="$DEFAULT_INSTALL_PREFIX"
  fi

  update_revision

  cmake -B build -DCMAKE_BUILD_TYPE="$build_type" "$with_extensions" -DCMAKE_INSTALL_PREFIX="$install_prefix" 
  cmake --build build

  discard_changes "$FILENAME"
}

cmake_build_and_install() {
  build_type="$1"
  with_extensions="$2"
  install_prefix="$3"
  cmake_build "$build_type" "$with_extensions" "$install_prefix"
  cmake --install build
}

cmake_build_and_pack() {
  build_type="$1"
  with_extensions="$2"
  cmake_build "$build_type" "$with_extensions"
  cpack --config build/CPackConfig.cmake
}
