#!/usr/bin/env bash

#------------------------------------------------------------------------------
# This script is used to build Hook.
#
# Usage:
#
#   build.sh [build_type] [with_extensions] [install_prefix]
#
# Examples:
#
#   build.sh
#   build.sh Release
#   build.sh Release with-extensions
#   build.sh Release without-extensions /usr/local
#------------------------------------------------------------------------------

build_type="$1"
with_extensions="$2"
install_prefix="$3"

source scripts/utils.sh

cmake_build $build_type $with_extensions $install_prefix
