#include "tui.h"
#include <ncurses.h>

TUI::TUI() 
    : currentMode(MODE_LIST), listCursor(0), searchCursor(0), 
      moduleCursor(0), shouldSave(false) {}

void TUI::initialize(const std::vector<PackageEntry>& packages) {
    installedPackages = packages;
}

void TUI::setSearchResults(const std::vector<SearchResult>& results) {
    searchResults = results;
    searchCursor = 0;
}

void TUI::setInstallTargets(const std::vector<InstallTarget>& targets) {
    installTargets = targets;
    moduleCursor = 0;
}

SearchResult TUI::getSelectedSearchResult() const {
    return selectedSearchResult;
}

InstallTarget TUI::getSelectedTarget() const {
    return selectedTarget;
}

std::string TUI::getSearchQuery() const {
    return searchQuery;
}

std::vector<PackageEntry> TUI::getPackages() const {
    return installedPackages;
}

void TUI::drawListMode() {
    attron(COLOR_PAIR(1));
    mvprintw(0, 0, "=== INSTALLED PACKAGES ===");
    attroff(COLOR_PAIR(1));
    mvprintw(1, 0, "Press 'a' to ADD new package | 'd' to mark for deletion | 'w' to save | 'q' to quit");
    
    int markedCount = 0;
    for (const auto& pkg : installedPackages) {
        if (pkg.markedForDeletion) markedCount++;
    }
    
    int startLine = 3;
    for (size_t i = 0; i < installedPackages.size() && startLine + i < LINES - 3; ++i) {
        const PackageEntry& pkg = installedPackages[i];
        
        if (i == (size_t)listCursor) attron(A_REVERSE);
        if (pkg.markedForDeletion) attron(COLOR_PAIR(4));
        
        mvprintw(startLine + i, 0, "%s %s", 
                (i == (size_t)listCursor) ? ">" : " ",
                pkg.name.c_str());
        mvprintw(startLine + i, 30, "(%s)", pkg.filePath.c_str());
        mvprintw(startLine + i, 70, "%s", pkg.markedForDeletion ? "[X]" : "[ ]");
        
        attroff(COLOR_PAIR(4));
        if (i == (size_t)listCursor) attroff(A_REVERSE);
    }
    
    mvprintw(LINES - 1, 0, "%zu packages | %d marked", installedPackages.size(), markedCount);
}

void TUI::drawSearchInputMode() {
    attron(COLOR_PAIR(2));
    mvprintw(0, 0, "=== SEARCH FOR PACKAGES TO INSTALL ===");
    attroff(COLOR_PAIR(2));
    mvprintw(2, 0, "Search query: %s_", searchQuery.c_str());
    mvprintw(4, 0, "Press ENTER to search | ESC to cancel");
}

void TUI::drawSearchResultsMode() {
    attron(COLOR_PAIR(2));
    mvprintw(0, 0, "=== SEARCH RESULTS (Available for Installation) ===");
    attroff(COLOR_PAIR(2));
    mvprintw(1, 0, "Query: '%s' | Press ENTER to select | ESC to cancel", searchQuery.c_str());
    
    int startLine = 3;
    for (size_t i = 0; i < searchResults.size() && startLine + i < LINES - 2; ++i) {
        const SearchResult& result = searchResults[i];
        
        if (i == (size_t)searchCursor) {
            attron(A_REVERSE);
        }
        
        mvprintw(startLine + i, 0, "%s %s", 
                (i == (size_t)searchCursor) ? ">" : " ",
                result.packageName.c_str());
        mvprintw(startLine + i, 30, "v%s", result.version.c_str());
        mvprintw(startLine + i, 40, "- %s", result.description.c_str());
        
        if (i == (size_t)searchCursor) {
            attroff(A_REVERSE);
        }
    }
    
    mvprintw(LINES - 1, 0, "%zu results", searchResults.size());
}

void TUI::drawModuleSelectMode() {
    attron(COLOR_PAIR(3));
    mvprintw(0, 0, "=== SELECT INSTALLATION TARGET ===");
    attroff(COLOR_PAIR(3));
    mvprintw(1, 0, "Package: %s", selectedSearchResult.packageName.c_str());
    mvprintw(2, 0, "Choose where to install:");
    
    int startLine = 4;
    for (size_t i = 0; i < installTargets.size() && startLine + i < LINES - 2; ++i) {
        const InstallTarget& target = installTargets[i];
        
        if (i == (size_t)moduleCursor) {
            attron(A_REVERSE);
        }
        
        mvprintw(startLine + i, 0, "%s [%d] %s", 
                (i == (size_t)moduleCursor) ? ">" : " ",
                (int)i + 1,
                target.fileName.c_str());
        mvprintw(startLine + i, 40, "(uses: %s)", 
                target.usesWithPkgs ? "with pkgs;" : "pkgs.");
        
        if (i == (size_t)moduleCursor) {
            attroff(A_REVERSE);
        }
    }
    
    mvprintw(LINES - 1, 0, "Press ENTER to install | ESC to cancel");
}

bool TUI::run() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_CYAN, COLOR_BLACK);
        init_pair(4, COLOR_RED, COLOR_BLACK);
    }
    
    bool running = true;
    while (running) {
        clear();
        
        switch (currentMode) {
            case MODE_LIST:
                drawListMode();
                break;
            case MODE_SEARCH_INPUT:
                drawSearchInputMode();
                break;
            case MODE_SEARCH_RESULTS:
                drawSearchResultsMode();
                break;
            case MODE_SELECT_MODULE:
                drawModuleSelectMode();
                break;
            case MODE_CONFIRM:
                break;
        }
        
        refresh();
        int ch = getch();
        
        if (currentMode == MODE_LIST) {
            if (ch == 'q' || ch == 'Q') {
                shouldSave = false;
                running = false;
            } else if (ch == 'w' || ch == 'W') {
                shouldSave = true;
                running = false;
            } else if (ch == 'a' || ch == 'A') {
                currentMode = MODE_SEARCH_INPUT;
                searchQuery.clear();
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (listCursor < (int)installedPackages.size() - 1) listCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (listCursor > 0) listCursor--;
            } else if (ch == 'd' || ch == 'D') {
                if (!installedPackages.empty()) {
                    installedPackages[listCursor].markedForDeletion = 
                        !installedPackages[listCursor].markedForDeletion;
                }
            }
        } else if (currentMode == MODE_SEARCH_INPUT) {
            if (ch == 27) {  // ESC
                currentMode = MODE_LIST;
            } else if (ch == 10 || ch == KEY_ENTER) {  // ENTER
                running = false;
                shouldSave = false;
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (!searchQuery.empty()) {
                    searchQuery.pop_back();
                }
            } else if (ch >= 32 && ch < 127) {
                searchQuery += (char)ch;
            }
        } else if (currentMode == MODE_SEARCH_RESULTS) {
            if (ch == 27) {  // ESC
                currentMode = MODE_LIST;
            } else if (ch == 10 || ch == KEY_ENTER) {  // ENTER
                if (!searchResults.empty()) {
                    selectedSearchResult = searchResults[searchCursor];
                    currentMode = MODE_SELECT_MODULE;
                }
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (searchCursor < (int)searchResults.size() - 1) searchCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (searchCursor > 0) searchCursor--;
            }
        } else if (currentMode == MODE_SELECT_MODULE) {
            if (ch == 27) {  // ESC
                currentMode = MODE_SEARCH_RESULTS;
            } else if (ch == 10 || ch == KEY_ENTER) {  // ENTER
                if (!installTargets.empty()) {
                    selectedTarget = installTargets[moduleCursor];
                    running = false;
                    shouldSave = false;
                }
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (moduleCursor < (int)installTargets.size() - 1) moduleCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (moduleCursor > 0) moduleCursor--;
            }
        }
    }
    
    endwin();
    return shouldSave;
}
