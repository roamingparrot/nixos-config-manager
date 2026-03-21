{ pkgs }:

pkgs.mkShell {
  buildInputs = [
    pkgs.cmake
    pkgs.ncurses
    pkgs.gcc
    pkgs.gnumake
  ];

  shellHook = ''
    echo "NixOS Configuration Manager Development Environment"
    echo "=================================================="
    echo ""
    echo "Available commands:"
    echo "  ./build.sh          - Build the project"
    echo "  sudo ./src/build/nixedit - Run the application"
    echo ""
  '';
}
