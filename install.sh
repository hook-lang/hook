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

home_dir="/opt/hook"

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

echo "export HOOK_HOME=$home_dir" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc

#------------------------------------------------------------------------------
# End
#------------------------------------------------------------------------------

sudo chmod +x "$home_dir/bin/hook"

info "Install successful."
info "To start using Hook, run: 'hook --help'"

exec bash
