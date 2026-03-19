#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>
#include "models/packageEntry.h"
#include "models/searchResult.h"
#include "models/installTarget.h"

enum TUIMode {
    MODE_LIST,          // Browse installed packages
    MODE_SEARCH,        // Live search + results in one view
    MODE_SELECT_MODULE  // Pick which file to install into
};

/**
 * @brief Self-contained TUI. Owns all search/install logic so it
 *        never needs to exit back to main to perform an action.
 */
class TUI {
private:
    // ── state ─────────────────────────────────────────────
    TUIMode mode;
    std::vector<PackageEntry>  installed;
    std::vector<SearchResult>  searchResults;
    std::vector<InstallTarget> installTargets;

    int  listCursor;
    int  resultCursor;
    int  moduleCursor;

    std::string searchQuery;
    std::string statusMsg;   // one-line feedback at bottom

    SearchResult  pendingResult;  // chosen from search before module pick
    bool          actionDone;     // set true once install/remove finished

    // ── drawing ───────────────────────────────────────────
    void drawBorder(int row, int cols);
    void drawList();
    void drawSearch();
    void drawModuleSelect();
    void setStatus(const std::string& msg);

    // ── search helpers ────────────────────────────────────
    void runSearch();                      // calls nix-env, updates searchResults
    void doInstall(const InstallTarget&);  // insert + rebuild
    void doRemove();                       // remove marked + rebuild
    void showRebuildOutput(const std::string& out, bool ok);

public:
    TUI();
    void initialize(const std::vector<PackageEntry>& pkgs,
                    const std::vector<InstallTarget>& targets);
    void run();
};

#endif // TUI_H
