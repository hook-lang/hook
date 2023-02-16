#!/usr/bin/env bash

#------------------------------------------------------------------------------
# This script is used to build a release version of Hook and pack it into a
# tarball.
#
# Usage:
#
#   pack.sh [with-extensions]
#
# Examples:
#
#   pack.sh
#   pack.sh with-extensions
#------------------------------------------------------------------------------

with_extensions="$1"

source scripts/utils.sh

cmake_build_and_pack Release $with_extensions
