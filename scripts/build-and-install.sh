#!/usr/bin/env bash

#------------------------------------------------------------------------------
# This script is used to build Hook and install it into the system.
#
# Usage:
#
#   build-and-install.sh [build_type] [with_extensions] [install_prefix]
#
# Examples:
#
#   build-and-install.sh
#   build-and-install.sh Release
#   build-and-install.sh Release with-extensions
#   build-and-install.sh Release without-extensions /usr/local
#------------------------------------------------------------------------------

build_type="$1"
with_extensions="$2"
install_prefix="$3"

source scripts/utils.sh

cmake_build_and_install "$build_type" "$with_extensions" "$install_prefix"
