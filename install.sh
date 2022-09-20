#!/bin/sh -e

#------------------------------------------------------------------------------
# Installation script for Hook
#------------------------------------------------------------------------------

BASE_URL="https://github.com/fabiosvm/hook-lang/releases/download"
VERSION="0.1.0"

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

detect_arch() {
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
}

detect_os() {
  os="unix"
  case "$(uname)" in
    [Ll]inux)
      os="linux";;
    [Dd]arwin)
      os="macos";;
    [Ff]ree[Bb][Ss][Dd])
      os="freebsd";;
    *)
      info "Warning: unable to detect OS, assuming generic unix ($os)"
  esac
}

detect_platform() {
  detect_arch
  detect_os
  platform="$os-$arch"
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
base_name="hook-$VERSION-$platform"
dist_url="$BASE_URL/$VERSION/$base_name.tar.gz"

temp_dir="$(mktemp -d 2>/dev/null || mktemp -d -t hook)"
temp_file="$temp_dir/hook-dist.tar.gz"

info "Downloading: $dist_url"

download "$dist_url" "$temp_file"

info "Unpacking.."

if ! tar -xzf "$temp_file"; then
  error "Extraction failed."
fi

home_dir="/usr/local/hook"

info "Installing to: $home_dir"
info "Need to use 'sudo' for install"

sudo -k
sudo mv "$base_name" "$home_dir"

info "Cleaning up.."

rm -rf "$temp_dir"

#------------------------------------------------------------------------------
# Setup the environment variable
#------------------------------------------------------------------------------

info "Setting the environment variable: HOOK_HOME"

if [ "$os" = "linux" ]; then
  rc=".bashrc"
else
  if [ "$os" = "macos" ]; then
    sh="$(env sh -c 'basename $(ps -o comm= -p $(ps -o ppid= -p $$))')"
    if [ "$sh" = "bash" ]; then
      rc=".bash_profile"
    else
      rc=".zshrc"
    fi
  fi
fi

if [ -z ${rc+x} ]; then
  warning "Please add $home_dir/bin to your PATH environment variable."
else
  echo "export HOOK_HOME=$home_dir" >> ~/$rc
  echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/$rc
fi

#------------------------------------------------------------------------------
# End
#------------------------------------------------------------------------------

sudo chmod +x "$home_dir/bin/hook"

info "Install successful."
info "Hook is now available in your PATH, but you may need to restart your shell."
info "You can check the installation by running 'hook --version'."
info "Enjoy!"
