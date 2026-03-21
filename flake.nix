{
  description = "nixedit - A TUI for managing NixOS packages";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
  };

  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; };
    in
    {
      packages.x86_64-linux.default = pkgs.callPackage ./default.nix { };

      devShells.x86_64-linux.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          ncurses
          gcc
          gnumake
        ];
      };

      formatter.x86_64-linux = pkgs.nixfmt;
    };
}
