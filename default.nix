{
  lib,
  stdenv,
  cmake,
  ncurses,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "nixedit";
  version = "0.1.0";

  src = lib.cleanSourceWith {
    src = ./.;
    filter =
      path: type:
      let
        baseName = baseNameOf path;
      in
      # Exclude build artifacts and git
      baseName != "build" && baseName != ".git" && baseName != "result";
  };

  nativeBuildInputs = [ cmake ];
  buildInputs = [ ncurses ];

  preConfigure = ''
    cd src
  '';

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  installPhase = ''
    mkdir -p $out/bin
    cp nixedit $out/bin/
  '';

  meta = with lib; {
    description = "A TUI for managing NixOS packages";
    longDescription = ''
      dotman is a terminal-based user interface for searching, installing,
      and removing NixOS packages by directly editing your Nix configuration files.
      It provides an intuitive way to manage your system packages without
      manually editing configuration files.
    '';
    homepage = "https://github.com/anomalyco/dotman";
    license = licenses.mit;
    maintainers = with lib.maintainers; [ roamingparrot ];
    mainProgram = "nixedit";
    platforms = lib.platforms.linux;
  };
})
