#!/usr/bin/env bash

set -e

echo "Building NixOS Configuration Manager..."

# Check if nix-shell is available
if ! command -v nix-shell &> /dev/null; then
    echo "Error: nix-shell not found. Please install Nix."
    exit 1
fi

# Build with nix-shell
nix-shell -p cmake ncurses --run "
    cd src
    mkdir -p build
    cd build
    cmake ..
    make
    echo ''
    echo 'Build complete!'
    echo 'Executable: src/build/nixedit'
    echo ''
    echo 'Run with: sudo ./src/build/nixedit'
"
