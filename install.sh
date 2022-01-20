#!/bin/bash

#
# Installation script for Hook
#

base_url="https://github.com/fabiosvm/hook-lang/releases/download"
version="0.1.0"

#
# Utility functions
#

info() {
  echo "$@"
}

warn() {
  echo "$@" >&2
}

stop() {
  warn $@
  exit 1
}

detect_osarch() {
  arch="$(uname -m)"
  case "$arch" in
    x86_64*|amd64*)
      arch="x64";;
    x86*|i[35678]86*)
      arch="x86";;
    arm64*|aarch64*|armv8*)   
      arch="arm64";;
    arm*)              
      arch="arm";;
    parisc*)
      arch="hppa";;          
  esac

  osarch="unix-$arch"
  case "$(uname)" in
    [Ll]inux)
      osarch="linux-$arch";;
    [Dd]arwin)
      osarch="macos-$arch";;
    [Ff]ree[Bb][Ss][Dd])
      osarch="unix-freebsd-$arch";;
    *)
      info "Warning: unable to detect OS, assuming generic unix ($osarch)"
  esac
}

has_command() {
  command -v "$1" > /dev/null 2>&1
}

make_temp_dir() {
  temp_dir="$(mktemp -d 2>/dev/null || mktemp -d -t hook)"
}

download_failed() {
  warn "Unable to download: $1"
  warn "It may be that there is no binary installer available for this platform: $osarch"
  warn "It's possible to build Hook from source: <https://github.com/fabiosvm/hook-lang>"
  stop ""
}

download() {
  if has_command curl ; then
    if ! curl -sS --proto =https --tlsv1.2 -f -L -o "$2" "$1"; then
      download_failed $1
    fi
  elif has_command wget ; then
    if ! wget -q --https-only "-O$2" "$1"; then
      download_failed $1
    fi
  else
    stop "Neither 'curl' nor 'wget' is available; install one to continue."
  fi
}

#
# Main process
#

detect_osarch
base_name="hook-$version-$osarch"
dist_source="$base_url/$version/$base_name.tar.gz"

make_temp_dir

temp_path="$temp_dir/hook-dist.tar.gz"

info "Downloading: $dist_source"

download "$dist_source" "$temp_path"

dist_dir="/opt/hook"
 
info "Unpacking and installing to: $dist_dir"

if ! tar -xzf "$temp_path"; then
  stop "Extraction failed."
fi

sudo mv "$base_name" "$dist_dir"

info "Cleaning up temporary directory.."

rm -rf "$temp_dir"

info "Setting the environment variable: HOOK_HOME"

echo "export HOOK_HOME=$dist_dir" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc

sudo chmod +x "$dist_dir/bin/hook"

exec bash
