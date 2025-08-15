#!/bin/bash
set -e

# This script sets up dependencies, builds the cb-mpc library, and runs the TDH2 demo.
# It assumes it is executed from the root of a freshly cloned repository on a Debian/Ubuntu system.

# Install build dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake curl rsync git

# Initialize submodules (required for vendored dependencies)
git submodule update --init --recursive

# Build a minimal static OpenSSL used by the library
make openssl-linux

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

