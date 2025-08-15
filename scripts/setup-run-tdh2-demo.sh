#!/bin/bash
set -e

# This script installs dependencies, builds the cb-mpc library and TDH2 demo,
# then runs the demo. It supports both Debian/Ubuntu (apt) and macOS (Homebrew)
# environments. It should be executed from the root of a freshly cloned repo.

OS_NAME=$(uname)
ARCH=$(uname -m)

if [[ $OS_NAME == "Darwin" ]]; then
  # macOS setup
  if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew is required but not installed. Visit https://brew.sh to install it." >&2
    exit 1
  fi
  brew update
  brew install cmake git rsync curl
  # Build minimal static OpenSSL
  if [[ $ARCH == "arm64" ]]; then
    scripts/openssl/build-static-openssl-macos-m1.sh
  else
    scripts/openssl/build-static-openssl-macos.sh
  fi
else
  # Debian/Ubuntu setup
  sudo apt-get update
  sudo apt-get install -y build-essential cmake curl rsync git
  make openssl-linux
fi

# Initialize submodules (required for vendored dependencies)
git submodule update --init --recursive

# Build the cb-mpc library (without running tests)
make build-no-test

# Install the library headers and archive into /usr/local/opt/cbmpc
sudo make install

# Build the TDH2 demo
cd demos-cpp/tdh2
cmake -B build
cmake --build build

# Run the demo binary
./build/mpc-demo-tdh2