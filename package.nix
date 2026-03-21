{
  lib,
  stdenv,
  fetchFromGitHub,
  cmake,
  ncurses,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "nixedit";
  version = "0.1.0";

  src = fetchFromGitHub {
    owner = "roamingparrot";
    repo = "nixos-config-manager";
    rev = "v${version}";
    hash = "sha256-XtGmw6cDx7QQO8s0fXRsQKKM7oqRWwNzqCPoIbRTxdU=";
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
    homepage = "https://github.com/roamingparrot/nixos-config-manager";
    license = licenses.mit;
    maintainers = with lib.maintainers; [ roamingparrot ];
    mainProgram = "nixedit";
    platforms = lib.platforms.linux;
  };
})
