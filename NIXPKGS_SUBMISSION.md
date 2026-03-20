# nixpkgs Submission Guide

This document outlines how to submit dotman to nixpkgs.

## Package Structure for nixpkgs

For nixpkgs submission, the package should go in `pkgs/by-name/do/dotman/`:

```
pkgs/by-name/do/dotman/
├── package.nix
└── (optional) patches/
```

## Steps to Submit

### 1. Fork nixpkgs

```bash
# Fork nixpkgs on GitHub first, then:
git clone https://github.com/YOUR_USERNAME/nixpkgs
cd nixpkgs
git checkout -b add-dotman
```

### 2. Create Package Directory

```bash
mkdir -p pkgs/by-name/do/dotman
```

### 3. Copy package.nix

Copy the `default.nix` from this repository to `pkgs/by-name/do/dotman/package.nix`:

```bash
cp /path/to/nix-os-manager/default.nix pkgs/by-name/do/dotman/package.nix
```

### 4. Update Package Metadata

In `package.nix`, update:
- `src` to fetch from GitHub release or tarball
- `version` to match the release tag
- Add yourself to `maintainers`

Example src:

```nix
src = fetchFromGitHub {
  owner = "YOUR_GITHUB_USERNAME";
  repo = "dotman";
  rev = "v${version}";
  hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
};
```

### 5. Test the Package

```bash
# Build the package
nix-build -A dotman

# Test installation
nix-env -f . -iA dotman
```

### 6. Run nixpkgs-review

```bash
nix-shell -p nixpkgs-review --run "nixpkgs-review pr --build"
```

### 7. Commit and Push

```bash
git add pkgs/by-name/do/dotman/
git commit -m "dotman: init at 0.1.0

A TUI for managing NixOS packages by editing configuration files."
git push origin add-dotman
```

### 8. Open Pull Request

Open a PR on https://github.com/NixOS/nixpkgs with:

**Title:** `dotman: init at 0.1.0`

**Description:**
```
# Description

dotman is a terminal-based user interface for managing NixOS packages
by directly editing declarative configuration files. It provides:

- Multi-file configuration support (follows imports)
- Package search using nix-env -qaP
- Smart syntax detection (with pkgs; vs pkgs.)
- Safe package removal preserving formatting
- Automatic nixos-rebuild integration

# Motivation

Managing packages across multiple NixOS configuration files can be
tedious. dotman provides a unified TUI view of all packages and
handles the complexity of multi-file configurations.

# Checklist

- [x] Built locally
- [x] Tested on NixOS
- [x] No network access during build
- [x] All dependencies declared
- [x] Meta attributes set (description, license, platforms, mainProgram)
```

## Requirements Checklist

Before submitting, ensure:

- [ ] Binary name matches directory: `dotman`
- [ ] `meta.description` is set
- [ ] `meta.longDescription` is set
- [ ] `meta.license` is set (MIT)
- [ ] `meta.platforms` is set (platforms.linux)
- [ ] `meta.mainProgram` is set ("dotman")
- [ ] `meta.homepage` points to repository
- [ ] No network calls during build
- [ ] Package builds with `nix-build -A dotman`
- [ ] All dependencies explicitly declared

## References

- nixpkgs by-name README: https://github.com/NixOS/nixpkgs/blob/master/pkgs/by-name/README.md
- Contributing guide: https://github.com/NixOS/nixpkgs/blob/master/CONTRIBUTING.md
