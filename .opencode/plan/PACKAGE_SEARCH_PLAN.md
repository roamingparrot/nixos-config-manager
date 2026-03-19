# Package Search & Installation - Implementation Plan

## Overview

Adding the ability to **search** packages using `nix search` and **install** them by modifying configuration files.

**Core Philosophy**: This tool is a *config editor with package awareness*, not a package manager. It modifies NixOS declarative configs and runs `nixos-rebuild switch`.

---

## Phase 0: Understanding the Nix Search Ecosystem

### Challenge 0.1: Learn `nix search` behavior
- **Task**: Document `nix search nixpkgs <query> --json` output format
- **Expected Output**:
  ```json
  {
    "nixpkgs#neovim": {
      "version": "0.9.5",
      "description": "Hyperextensible Vim-based text editor",
      "pname": "neovim",
      "attrName": "neovim"
    }
  }
  ```
- **Key Insight**: Attribute path is `nixpkgs#neovim` → need to extract `pkgs.neovim`

### Challenge 0.2: Understand Nix attribute naming
- **Task**: Learn that Nix attributes ≠ package names
- **Examples**:
  - `google-chrome` package → `pkgs.google-chrome`
  - `nodejs` package → `pkgs.nodejs`
  - `requests` (Python) → `pkgs.python311Packages.requests`
- **Challenge**: Must use the attribute path, not just the name

---

## Phase 1: Data Model Extensions

### Challenge 1.1: Create SearchResult model
```cpp
// models/searchResult.h
struct SearchResult {
    std::string attributePath;    // "nixpkgs#neovim"
    std::string packageName;      // "neovim"
    std::string version;          // "0.9.5"
    std::string description;      // "Hyperextensible Vim-based text editor"
};
```

### Challenge 1.2: Create InstallTarget model
```cpp
// models/installTarget.h
struct InstallTarget {
    std::string filePath;         // Absolute path: "/etc/nixos/modules/coding.nix"
    bool usesWithPkgs;            // true if file has "with pkgs;" before systemPackages
    int insertLine;              // Line number to insert after
    std::string indentation;     // "      " (preserve existing indentation)
};
```

### Challenge 1.3: Extend PackageEntry
```cpp
// Add to models/packageEntry.h
struct PackageEntry {
    // ... existing fields ...
    std::string nixAttribute;     // "pkgs.neovim" or "neovim"
};
```

---

## Phase 2: Package Search Component

### Challenge 2.1: Create PackageSearcher class
```cpp
// core/packageSearcher.h
class PackageSearcher {
private:
    std::vector<SearchResult> lastResults;
    std::string lastQuery;

    std::vector<SearchResult> parseJsonOutput(const std::string& json);
    std::string executeSearch(const std::string& query);

public:
    std::vector<SearchResult> search(const std::string& query);
    std::vector<SearchResult> getResults() const;
    std::string getLastQuery() const;
};
```

### Challenge 2.2: Implement JSON parsing
- **Approach**: Manual parsing (no external JSON library)
- **Strategy**:
  1. Find all `"nixpkgs#<attr>"` patterns
  2. Extract nested `version`, `description` fields
  3. Use regex for simple extraction

### Challenge 2.3: Implement search execution
- **Command**: `nix search nixpkgs <query> --json`
- **Error Handling**: Check if `nix` command exists, handle timeout
- **Performance**: Cache results for fuzzy search (future enhancement)

### Challenge 2.4: Extract correct Nix attribute
- **Challenge**: From `nixpkgs#neovim`, extract `pkgs.neovim`
- **Solution**: Replace `nixpkgs#` with `pkgs.` (works for most nixpkgs packages)
- **Edge Case**: Nested packages like `nixpkgs#python311Packages.requests`
  - Solution: `pkgs.python311Packages.requests`

---

## Phase 3: Detect File Syntax (with pkgs;)

### Challenge 3.1: Create FileSyntaxDetector class
```cpp
// core/fileSyntaxDetector.h
class FileSyntaxDetector {
public:
    InstallTarget analyzeFile(const std::string& filePath);

private:
    bool hasWithPkgs(const std::string& content, size_t systemPackagesPos);
    int findPackageListStart(const std::string& content, size_t systemPackagesPos);
    std::string detectIndentation(const std::string& content, int lineNumber);
};
```

### Challenge 3.2: Implement with-pkgs detection
- **Algorithm**:
  1. Find `environment.systemPackages =`
  2. Search backwards (within ~10 lines) for `with pkgs;`
  3. If found before the `=`, use simple name insertion

### Challenge 3.3: Handle package list insertion point
- **Challenge**: Where exactly to insert in `[ git neovim ]`?
- **Solution**: Insert *after* the opening bracket `[` but *before* first package
- **Formatting**: Preserve existing indentation (detected from file)

---

## Phase 4: Module Selection UI

### Challenge 4.1: Create ModuleSelector class
```cpp
// core/moduleSelector.h
class ModuleSelector {
private:
    std::vector<InstallTarget> availableTargets;
    std::vector<InstallTarget> discoverTargets();

public:
    std::vector<InstallTarget> getTargets() const;
    InstallTarget selectTarget(int index);
};
```

### Challenge 4.2: Discover installation targets
- **Algorithm**:
  1. Load all modules (already done by ModuleResolver)
  2. For each module, check if it contains `environment.systemPackages`
  3. Analyze syntax using FileSyntaxDetector
  4. Return list of valid targets

### Challenge 4.3: Display module selection in TUI
- **New TUI State**: Mode = `SELECT_MODULE`
- **Display**:
  ```
  Select installation target:
  
  [1] /etc/nixos/modules/coding.nix        (uses: with pkgs;)
  [2] /etc/nixos/modules/other-pkgs.nix   (uses: with pkgs;)
  [3] /etc/nixos/packages/global.nix       (uses: pkgs.)
  
  [j/k] Navigate  [Enter] Select  [q] Cancel
  ```

---

## Phase 5: Package Inserter Component

### Challenge 5.1: Create PackageInserter class
```cpp
// core/packageInserter.h
class PackageInserter {
public:
    bool insertPackage(const InstallTarget& target, const std::string& package);

private:
    std::string readFile(const std::string& filePath);
    int findInsertPosition(const std::string& content, const InstallTarget& target);
    void writeFile(const std::string& filePath, const std::string& content);
};
```

### Challenge 5.2: Implement smart insertion
- **Challenge**: Insert in the right format
- **Input**: `environment.systemPackages = with pkgs; [ git ]`
- **Action**: Insert `neovim` to get `environment.systemPackages = with pkgs; [ git neovim ]`
- **Key Logic**:
  - If target uses `with pkgs;`: insert simple name
  - If target doesn't: insert `pkgs.name`
  - Preserve formatting and indentation

### Challenge 5.3: Handle empty package lists
- **Case**: `environment.systemPackages = with pkgs; [ ]`
- **Action**: Insert between `[` and `]`: `environment.systemPackages = with pkgs; [ neovim ]`

### Challenge 5.4: Preserve file formatting
- **Challenge**: Don't break existing indentation or style
- **Solution**: Detect indentation from surrounding code, apply same style

---

## Phase 6: TUI Integration

### Challenge 6.1: Extend TUI state machine
```cpp
// Add to ui/tui.h
enum TUIMode {
    MODE_LIST,           // View packages, mark for deletion
    MODE_SEARCH,         // Search for packages
    MODE_SEARCH_RESULTS, // View search results
    MODE_SELECT_MODULE,  // Choose install target
    MODE_REBUILDING     // Show rebuild progress
};
```

### Challenge 6.2: Add search mode UI
- **Trigger**: Press `/` to enter search mode
- **Display**:
  ```
  Search packages: nvim▋
  
  Results:
  > neovim        0.9.5     Hyperextensible Vim-based text editor
    neovim-0.9    0.9.0     Neovim 0.9.x legacy version
    neovim-qt     0.2.4    Neovim Qt GUI
  
  [Enter] Install  [d] Add description  [q] Cancel
  ```

### Challenge 6.3: Add search results mode
- **Trigger**: After entering search query and pressing Enter
- **Display**: List of SearchResult items
- **Navigation**: j/k to move, Enter to select for installation

### Challenge 6.4: Implement search-as-you-type
- **Challenge**: Update results as user types
- **Solution**: 
  - Debounce input (wait 300ms after last keystroke)
  - Execute `nix search` command
  - Update display

### Challenge 6.5: Connect search → module selection → insert → rebuild
- **Flow**:
  1. User searches for "neovim"
  2. User selects "neovim" from results
  3. TUI switches to module selection mode
  4. User picks target file
  5. PackageInserter adds package
  6. RebuildManager runs rebuild

---

## Phase 7: UX Enhancements

### Challenge 7.1: Detect duplicate packages
- **Challenge**: Prevent adding same package twice
- **Solution**:
  1. Before inserting, check if package exists in any module
  2. If exists, show: "Package 'neovim' already in /etc/nixos/modules/coding.nix"
  3. Offer to navigate there instead

### Challenge 7.2: Show package attributes
- **Challenge**: Display full attribute path for clarity
- **Display**: `pkgs.neovim` vs just `neovim`
- **User Benefit**: Shows exactly what will be written to config

### Challenge 7.3: Fuzzy search enhancement
- **Future**: Replace exact match with fuzzy matching
- **Implementation**: Use `nix search` then filter client-side

---

## Phase 8: Error Handling & Validation

### Challenge 8.1: Validate Nix syntax before save
- **Command**: `nix-instantiate --parse <file.nix>`
- **Challenge**: Ensure modified files are syntactically valid
- **Flow**:
  1. After inserting package, validate file
  2. If invalid, show error and don't proceed to rebuild

### Challenge 8.2: Handle nix search not found
- **Challenge**: `nix search` might not be installed
- **Error Message**: "Package 'nix-search' not found. Install with: nix-env -iA nixpkgs.nix-search"
- **Fallback**: Use `--dry-run` rebuild to catch errors

### Challenge 8.3: Handle network errors
- **Challenge**: `nix search` requires network
- **Error**: Show offline message if network unavailable

---

## Phase 9: Testing

### Challenge 9.1: Create test fixtures
```
/tmp/test-nixos-config/
├── configuration.nix          # imports: ./modules/base.nix, ./modules/dev.nix
├── modules/
│   ├── base.nix               # environment.systemPackages = with pkgs; [ git ]
│   └── dev.nix                # environment.systemPackages = pkgs. [ vim ]
└── test_search_results.json    # Sample nix search output
```

### Challenge 9.2: Test search functionality
- Mock `nix search` command with sample output
- Verify JSON parsing extracts correct fields
- Test error handling

### Challenge 9.3: Test insertion logic
- Test with `with pkgs;` syntax
- Test without `with pkgs;` syntax
- Test empty package lists
- Test duplicate prevention

### Challenge 9.4: Integration test
- Full flow: search → select module → insert → rebuild
- Use test fixtures instead of real system

---

## Implementation Order

### Week 1: Foundation
1. **Phase 1**: Extend data models (SearchResult, InstallTarget)
2. **Phase 2**: Create PackageSearcher with basic nix search execution
3. **Phase 2.2**: Implement JSON parsing (manual, no library)

### Week 2: Core Logic
4. **Phase 3**: Create FileSyntaxDetector
5. **Phase 3.2-3.3**: Implement with-pkgs detection and insertion point finding
6. **Phase 4**: Create ModuleSelector and discovery logic

### Week 3: TUI Integration
7. **Phase 6.1**: Extend TUI state machine
8. **Phase 6.2**: Add search mode UI
9. **Phase 6.3**: Add search results mode
10. **Phase 6.4**: Implement search-as-you-type (debounced)

### Week 4: Polish
11. **Phase 5**: Create PackageInserter and integrate
12. **Phase 6.5**: Connect full flow (search → select → insert → rebuild)
13. **Phase 7**: Add duplicate detection
14. **Phase 8**: Error handling and validation

---

## File Structure Updates

```
src/
├── main.cpp
├── CMakeLists.txt
├── core/
│   ├── moduleResolver.h/cpp         (existing)
│   ├── configParser.h/cpp          (existing)
│   ├── configEditor.h/cpp          (existing)
│   ├── rebuildManager.h/cpp        (existing)
│   ├── packageSearcher.h/cpp       (NEW)
│   ├── fileSyntaxDetector.h/cpp    (NEW)
│   ├── moduleSelector.h/cpp         (NEW)
│   └── packageInserter.h/cpp       (NEW)
├── models/
│   ├── packageEntry.h              (existing, extend)
│   ├── moduleInfo.h                (existing)
│   ├── searchResult.h              (NEW)
│   └── installTarget.h             (NEW)
└── ui/
    ├── tui.h/cpp                   (existing, extend)
    └── inputHandler.h/cpp          (existing)
```

---

## Key Design Decisions

1. **JSON Parsing**: Manual parsing (no nlohmann_json) to minimize dependencies
2. **Insertion Strategy**: Auto-detect `with pkgs;`, use simple names if detected
3. **Module Selection**: Always ask user to pick target module
4. **Validation**: Validate with `nix-instantiate --parse` before rebuild
5. **Error Handling**: Show clear errors, don't proceed on failure

---

## Success Criteria

✅ User can press `/` to search for packages  
✅ Search results display name, version, description  
✅ User can select a package and pick target module  
✅ Package is inserted in correct format (with/without pkgs.)  
✅ Duplicate packages are detected and prevented  
✅ `nixos-rebuild switch` runs after successful insertion  
✅ All changes are reversible via Nix generations  

---

## Future Enhancements (Out of Scope for MVP)

- Fuzzy search (client-side filtering)
- Package version pinning
- Package removal with dependency analysis
- NixOS option search
- Configuration diff before rebuild
