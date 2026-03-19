# NixOS TUI Configuration Editor

## Overview

A terminal-based (TUI) application written in C++ for managing system packages on NixOS by editing declarative configuration files across multiple modules.

Unlike simple tools that modify a single `configuration.nix`, this project is designed to work with real-world setups where configuration is split across multiple imported files.

---

## Goals

### Current Scope (MVP)

- Discover and read all NixOS configuration modules
- Resolve `imports` recursively across files
- Locate all definitions of `environment.systemPackages`
- Aggregate and display all system packages in a single TUI
- Show which file each package is defined in
- Allow users to remove a package from its source file
- Safely write changes back to the correct file
- Trigger `nixos-rebuild switch`

---

## Key Concept

NixOS configurations are modular.

**configuration.nix**
```nix
{
  imports = [
    ./hardware-configuration.nix
    ./packages.nix
  ];
}
```

**packages.nix**
```nix
{
  environment.systemPackages = with pkgs; [
    git
    neovim
  ];
}
```

This tool must follow imports, build a unified view, and edit the correct source file.

---

## Features

### Multi-File Support

- Parse `imports = [ ... ]`
- Resolve relative paths
- Recursively load modules
- Avoid duplicate processing

### Package Discovery

- Detect all `environment.systemPackages` blocks
- Extract package names
- Track origin file path

### TUI Interface

Display aggregated packages:
```
> git        (/etc/nixos/packages.nix)
  firefox    (/etc/nixos/configuration.nix)
  neovim     (/etc/nixos/dev/tools.nix)
```

### Editing

When removing a package:
- Identify its source file
- Modify only that file
- Preserve formatting as much as possible

### Rebuild Integration

After changes, run `nixos-rebuild switch`:
- Show progress
- Display errors if rebuild fails

---

## Architecture

### 1. File Discovery

Responsible for starting at `/etc/nixos/configuration.nix` and finding all imported files.

```cpp
class ModuleResolver {
public:
    std::vector<std::string> resolveAllModules(const std::string& entryFile);
};
```

### 2. Config Parser

Extracts package definitions from each file.

```cpp
struct PackageEntry {
    std::string name;
    std::string filePath;
};

class ConfigParser {
public:
    std::vector<PackageEntry> extractPackages(const std::string& fileContent,
                                              const std::string& filePath);
};
```

### 3. Editor

Handles safe modification of files.

```cpp
class ConfigEditor {
public:
    bool removePackage(const PackageEntry& entry);
};
```

### 4. Rebuild Manager

```cpp
class RebuildManager {
public:
    bool rebuild();
};
```

### 5. TUI Layer

Displays aggregated package list and handles navigation and actions.

---

## Parsing Strategy

### Important Constraint

This project does **not implement a full Nix parser**.

Instead, it uses:
- Pattern-based extraction
- Bracket matching for `[ ... ]`
- Line-based manipulation

### Supported Pattern

```nix
environment.systemPackages = with pkgs; [
  git
  neovim
];
```

### Limitations

- Dynamic expressions may not be detected:
  ```nix
  environment.systemPackages = someFunction pkgs;
  ```
- Packages generated via functions may be invisible
- Complex abstractions are out of scope for MVP

---

## Safety Features

### Backups

Before modifying any file, create `<file>.bak`

### Minimal Edits

- Only modify the exact package list
- Do not reformat entire files
- Preserve comments where possible

### Failure Handling

If rebuild fails:
- Show error output
- Do not delete backups

---

## Permissions

Required for:
- Editing `/etc/nixos/*`
- Running rebuild

Recommended usage:
```
sudo ./dotman
```

---

## UI Controls

| Key   | Action                  |
|-------|-------------------------|
| j / ↓ | Move down               |
| k / ↑ | Move up                 |
| d     | Remove selected package |
| q     | Quit                    |
| /     | Search                  |

---

## Project Structure

```
src/
 ├── core/
 │    ├── moduleResolver.cpp
 │    ├── configParser.cpp
 │    ├── configEditor.cpp
 │    └── rebuildManager.cpp
 ├── ui/
 │    └── tui.cpp
 ├── main.cpp
 └── CMakeLists.txt
```

---

## Building

```bash
mkdir build && cd build
cmake ..
make
```

---

## License

MIT
