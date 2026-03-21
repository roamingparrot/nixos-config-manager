#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>
#include <chrono>
#include "models/packageEntry.h"
#include "models/searchResult.h"
#include "models/installTarget.h"

enum TUIMode {
    MODE_LIST,          // Browse installed packages
    MODE_SEARCH,        // Live search + results
    MODE_SELECT_MODULE,  // Pick target .nix file
    MODE_SETTINGS        // Settings menu
};

/**
 * @brief Self-contained TUI. Owns all search/install logic.
 *        Uses halfdelay() + timestamp debouncing so search only
 *        fires after the user pauses typing for ~600 ms.
 */
class TUI {
private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    // ── state ─────────────────────────────────────────────
    TUIMode mode;
    std::vector<PackageEntry>  installed;
    std::vector<SearchResult>  searchResults;
    std::vector<InstallTarget> installTargets;

    int listCursor;
    int resultCursor;
    int moduleCursor;

    std::string searchQuery;
    std::string statusMsg;
    std::string searchingMsg;  // "Searching…" indicator

    SearchResult pendingResult;

    // Debounce: fire search 600 ms after last keystroke
    TimePoint    lastKeystroke;
    bool         searchPending;  // query changed, search not yet run
    bool         isSearching;    // search currently running

    // Config path for reloading packages after install
    std::string configPath;

    // Settings
    struct Settings {
        std::string rebuildCommand = "nixos-rebuild switch";
        bool automaticRebuild = true;  // true = auto rebuild, false = manual
    } settings;
    int settingsCursor;

    // ── draw ──────────────────────────────────────────────
    void drawList();
    void drawSearch();
    void drawModuleSelect();
    void drawSettings();
    void drawBox(int y, int x, int h, int w);
    void drawRebuildOutput(const std::string& out, bool ok);

    // ── helpers ───────────────────────────────────────────
    void triggerSearch();
    void doInstall(const InstallTarget& t);
    void doRemove();
    void reloadPackages();  // Reload installed packages after rebuild

public:
    TUI();
    void initialize(const std::vector<PackageEntry>& pkgs,
                    const std::vector<InstallTarget>& targets,
                    const std::string& configPath = "/etc/nixos/configuration.nix");
    void run();
};

#endif // TUI_H
