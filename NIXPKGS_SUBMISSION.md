# nixpkgs Submission Guide

This document outlines how to submit nixedit to nixpkgs.

## Package Structure for nixpkgs

For nixpkgs submission, the package should go in `pkgs/by-name/ni/nixedit/`:

```
pkgs/by-name/ni/nixedit/
├── package.nix
└── (optional) patches/
```

## Steps to Submit

### 1. Fork nixpkgs

```bash
# Fork nixpkgs on GitHub first, then:
git clone https://github.com/YOUR_USERNAME/nixpkgs
cd nixpkgs
git checkout -b add-nixedit
```

### 2. Create Package Directory

```bash
mkdir -p pkgs/by-name/ni/nixedit
```

### 3. Copy package.nix

Copy the `default.nix` from this repository to `pkgs/by-name/ni/nixedit/package.nix`:

```bash
cp /path/to/nix-os-manager/default.nix pkgs/by-name/ni/nixedit/package.nix
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
  repo = "nixedit";
  rev = "v${version}";
  hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
};
```

### 5. Test the Package

```bash
# Build the package
nix-build -A nixedit

# Test installation
nix-env -f . -iA nixedit
```

### 6. Run nixpkgs-review

```bash
nix-shell -p nixpkgs-review --run "nixpkgs-review pr --build"
```

### 7. Commit and Push

```bash
git add pkgs/by-name/ni/nixedit/
git commit -m "nixedit: init at 0.1.0

A TUI for managing NixOS packages by editing configuration files."
git push origin add-nixedit
```

### 8. Open Pull Request

Open a PR on https://github.com/NixOS/nixpkgs with:

**Title:** `nixedit: init at 0.1.0`

**Description:**
```
# Description

nixedit is a terminal-based user interface for managing NixOS packages
by directly editing declarative configuration files. It provides:

- Multi-file configuration support (follows imports)
- Package search using nix-env -qaP
- Smart syntax detection (pkgs. references)
- Safe package removal preserving formatting
- Automatic nixos-rebuild integration

# Motivation

Managing packages across multiple NixOS configuration files can be
tedious. nixedit provides a unified TUI view of all packages and
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

- [ ] Binary name matches directory: `nixedit`
- [ ] `meta.description` is set
- [ ] `meta.longDescription` is set
- [ ] `meta.license` is set (MIT)
- [ ] `meta.platforms` is set (platforms.linux)
- [ ] `meta.mainProgram` is set ("nixedit")
- [ ] `meta.homepage` points to repository
- [ ] No network calls during build
- [ ] Package builds with `nix-build -A nixedit`
- [ ] All dependencies explicitly declared

## References

- nixpkgs by-name README: https://github.com/NixOS/nixpkgs/blob/master/pkgs/by-name/README.md
- Contributing guide: https://github.com/NixOS/nixpkgs/blob/master/CONTRIBUTING.md
