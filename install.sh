#!/bin/bash

#------------------------------------------------------------------------------
# Installation script for Hook
#------------------------------------------------------------------------------

base_url="https://github.com/fabiosvm/hook-lang/releases/download"
version="0.1.0"

#------------------------------------------------------------------------------
# Utility functions
#------------------------------------------------------------------------------

info() {
  echo "$@"
}

warning() {
  echo "$@" >&2
}

error() {
  warning $@
  exit 1
}

has_command() {
  command -v "$1" > /dev/null 2>&1
}

#------------------------------------------------------------------------------
# Detect the platform
#------------------------------------------------------------------------------

detect_platform() {
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
  platform="unix-$arch"
  case "$(uname)" in
    [Ll]inux)
      platform="linux-$arch";;
    [Dd]arwin)
      platform="macos-$arch";;
    [Ff]ree[Bb][Ss][Dd])
      platform="unix-freebsd-$arch";;
    *)
      info "Warning: unable to detect OS, assuming generic unix ($platform)"
  esac
}

#------------------------------------------------------------------------------
# Download the binary
#------------------------------------------------------------------------------

download_failed() {
  warning "Unable to download: $1"
  warning "It may be that there is no binary installer available for this platform: $platform"
  warning "It's possible to build Hook from source: <https://github.com/fabiosvm/hook-lang>"
  exit 1
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
    error "Neither 'curl' nor 'wget' is available; install one to continue."
  fi
}

#------------------------------------------------------------------------------
# Download and install
#------------------------------------------------------------------------------

detect_platform
base_name="hook-$version-$platform"
dist_source="$base_url/$version/$base_name.tar.gz"

temp_dir="$(mktemp -d 2>/dev/null || mktemp -d -t hook)"
temp_path="$temp_dir/hook-dist.tar.gz"

info "Downloading: $dist_source"

download "$dist_source" "$temp_path"

info "Unpacking.."

if ! tar -xzf "$temp_path"; then
  error "Extraction failed."
fi

dist_dir="/opt/hook"

info "Installing.."
info "Need to use 'sudo' for install at: $dist_dir"

sudo -k
sudo mv "$base_name" "$dist_dir"

info "Cleaning up temporary directory.."

rm -rf "$temp_dir"

info "Setting the environment variable: HOOK_HOME"

echo "export HOOK_HOME=$dist_dir" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc

sudo chmod +x "$dist_dir/bin/hook"

info "Install successful."
info "To start using Hook, run: 'hook --help'"

exec bash
