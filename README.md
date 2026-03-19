# NixOS TUI Configuration Editor

A terminal-based (TUI) application written in C++ for managing system packages on NixOS by editing declarative configuration files across multiple modules.

Unlike simple tools that modify a single `configuration.nix`, this project is designed to work with real-world setups where configuration is split across multiple imported files.

## Features

- **Multi-File Support** - Parse `imports`, resolve relative paths, and recursively load all modules
- **Package Discovery** - Detect all `environment.systemPackages` blocks and extract package names
- **Package Search & Install** - Search nixpkgs and install new packages to your configuration
- **Smart Module Selection** - Choose which configuration file to install packages into
- **Syntax Detection** - Auto-detects `with pkgs;` vs `pkgs.` syntax and inserts correctly
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

## Workflow

### Installing a Package
1. Launch the TUI: `sudo ./src/build/dotman`
2. Press `a` to open package search
3. Type the package name (e.g., "firefox")
4. Press Enter to search
5. Navigate results with `j`/`k`, press Enter to select
6. Choose which module to install to (e.g., `packages.nix`)
7. Package is automatically added and system rebuilds

### Removing a Package
1. Launch the TUI: `sudo ./src/build/dotman`
2. Navigate to package with `j`/`k`
3. Press `d` to mark for deletion
4. Press `w` to save and rebuild
5. Package is removed from its source file and system rebuilds

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

### Main View (Installed Packages)
| Key   | Action                   |
|-------|--------------------------|
| a     | Add new package (search) |
| d     | Mark package for deletion |
| j / ↓ | Move down                |
| k / ↑ | Move up                  |
| w     | Save changes and rebuild |
| q     | Quit without saving      |

### Search Mode
| Key       | Action                |
|-----------|-----------------------|
| Type      | Enter search query    |
| Enter     | Execute search        |
| ESC       | Cancel                |

### Search Results
| Key       | Action                |
|-----------|-----------------------|
| j / ↓     | Move down             |
| k / ↑     | Move up               |
| Enter     | Select package        |
| ESC       | Back to main view     |

### Module Selection
| Key       | Action                |
|-----------|-----------------------|
| j / ↓     | Move down             |
| k / ↑     | Move up               |
| Enter     | Install to module     |
| ESC       | Back to search        |

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
