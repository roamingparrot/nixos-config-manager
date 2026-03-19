{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    ncurses
    gcc
    gnumake
  ];

  shellHook = ''
    echo "NixOS Configuration Manager Development Environment"
    echo "=================================================="
    echo ""
    echo "Available commands:"
    echo "  ./build.sh          - Build the project"
    echo "  sudo ./src/build/dotman - Run the application"
    echo ""
  '';
}
