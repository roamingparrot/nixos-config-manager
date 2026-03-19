# NixOS TUI Configuration Editor

A terminal-based (TUI) application written in C++ for managing system packages on NixOS by editing declarative configuration files across multiple modules.

Unlike simple tools that modify a single `configuration.nix`, this project is designed to work with real-world setups where configuration is split across multiple imported files.

## Features

- **Multi-File Support** - Parse `imports`, resolve relative paths, and recursively load all modules
- **Package Discovery** - Detect all `environment.systemPackages` blocks and extract package names
- **TUI Interface** - View all packages aggregated in one place with their source file
- **Safe Editing** - Remove packages from their source file, preserving formatting
- **Rebuild Integration** - Run `nixos-rebuild switch` after changes

## Installation

### Quick Build (Recommended)

```bash
./build.sh
```

### Manual Build

```bash
cd src
mkdir build && cd build
cmake ..
make
```

## Usage

Run the TUI to manage packages:

```bash
sudo ./src/build/dotman
```

**Note:** Root privileges are required to edit `/etc/nixos/` files and run `nixos-rebuild`.

## Key Concept

NixOS configurations are modular. This tool follows imports to build a unified view of all packages across your configuration files, then edits only the correct source file when removing a package.

**Example structure:**
```nix
# configuration.nix
{ imports = [ ./hardware-configuration.nix ./packages.nix ]; }

# packages.nix
{ environment.systemPackages = with pkgs; [ git neovim ]; }
```

## Controls

| Key   | Action                   |
|-------|--------------------------|
| j / ↓ | Move down                |
| k / ↑ | Move up                  |
| d     | Mark package for deletion |
| w     | Save changes and rebuild |
| q     | Quit without saving      |

**Note:** Nix handles configuration backups via generations. You can rollback using `nixos-rebuild --rollback`.

## Parsing Limitations

This is **not** a full Nix parser. It uses pattern-based extraction for the common form:

```nix
environment.systemPackages = with pkgs; [
  git
  neovim
];
```

Dynamic expressions like `environment.systemPackages = someFunction pkgs;` are not detected.

## License

MIT
