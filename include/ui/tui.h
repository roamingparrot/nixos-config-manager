#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>
#include "models/packageEntry.h"
#include "models/searchResult.h"
#include "models/installTarget.h"

/**
 * @brief TUI mode states
 */
enum TUIMode {
    MODE_LIST,           // View installed packages, mark for deletion
    MODE_SEARCH_INPUT,   // Enter search query
    MODE_SEARCH_RESULTS, // View search results
    MODE_SELECT_MODULE,  // Choose installation target
    MODE_CONFIRM        // Confirm action
};

/**
 * @brief Terminal User Interface for package management
 */
class TUI {
private:
    // State
    TUIMode currentMode;
    std::vector<PackageEntry> installedPackages;
    std::vector<SearchResult> searchResults;
    std::vector<InstallTarget> installTargets;
    
    // Cursor positions for each mode
    int listCursor;
    int searchCursor;
    int moduleCursor;
    
    // User input
    std::string searchQuery;
    bool shouldSave;
    
    // Selected items
    SearchResult selectedSearchResult;
    InstallTarget selectedTarget;
    
    // Drawing methods
    void drawListMode();
    void drawSearchInputMode();
    void drawSearchResultsMode();
    void drawModuleSelectMode();

public:
    TUI();
    
    void initialize(const std::vector<PackageEntry>& packages);
    bool run();
    std::vector<PackageEntry> getPackages() const;
    
    // Mode-specific setters
    void setSearchResults(const std::vector<SearchResult>& results);
    void setInstallTargets(const std::vector<InstallTarget>& targets);
    
    // Get selected items
    SearchResult getSelectedSearchResult() const;
    InstallTarget getSelectedTarget() const;
    std::string getSearchQuery() const;
};

#endif // TUI_H
