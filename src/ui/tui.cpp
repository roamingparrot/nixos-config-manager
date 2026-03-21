#include "ui/tui.h"
#include "core/packageSearcher.h"
#include "core/packageInserter.h"
#include "core/configEditor.h"
#include "core/rebuildManager.h"
#include <ncurses.h>
#include <sstream>
#include <cstring>
#include <cstdlib>

using Ms = std::chrono::milliseconds;

// ── constructor ───────────────────────────────────────────────────────────────

TUI::TUI()
    : mode(MODE_LIST), listCursor(0), resultCursor(0), moduleCursor(0),
      searchPending(false), isSearching(false) {}

void TUI::initialize(const std::vector<PackageEntry>& pkgs,
                     const std::vector<InstallTarget>& targets) {
    installed      = pkgs;
    installTargets = targets;
}

// ── box drawing ───────────────────────────────────────────────────────────────

void TUI::drawBox(int y, int x, int h, int w) {
    // Corners
    mvaddch(y,         x,         ACS_ULCORNER);
    mvaddch(y,         x + w - 1, ACS_URCORNER);
    mvaddch(y + h - 1, x,         ACS_LLCORNER);
    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);
    // Horizontal edges
    mvhline(y,         x + 1, ACS_HLINE, w - 2);
    mvhline(y + h - 1, x + 1, ACS_HLINE, w - 2);
    // Vertical edges
    mvvline(y + 1, x,         ACS_VLINE, h - 2);
    mvvline(y + 1, x + w - 1, ACS_VLINE, h - 2);
}

// ── helpers ───────────────────────────────────────────────────────────────────

static std::string trunc(const std::string& s, int max) {
    if (max <= 0) return "";
    if ((int)s.size() <= max) return s;
    return s.substr(0, max - 1) + "~";
}


// Print a status bar with left-aligned status and right-aligned hints
static void printStatusBar(int row, const std::string& status, const std::string& hints, int cols) {
    int statusLen = (int)status.size();
    int hintsLen = (int)hints.size();
    int gap = 2;
    
    std::string displayHints = hints;
    if (statusLen + gap + hintsLen > cols) {
        int availableHints = cols - statusLen - gap;
        if (availableHints > 3) {
            displayHints = trunc(hints, availableHints);
        } else {
            displayHints = "";
        }
        hintsLen = (int)displayHints.size();
    }
    
    int padding = cols - statusLen - hintsLen;
    if (padding < gap) padding = gap;
    
    mvprintw(row, 0, "%s%*s", status.c_str(), padding + hintsLen, displayHints.c_str());
}

// ── MODE_LIST ─────────────────────────────────────────────────────────────────

void TUI::drawList() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Title bar
    attron(A_REVERSE | A_BOLD);
    mvprintw(0, 0, "%-*s", cols, " nixedit – NixOS package manager");
    attroff(A_REVERSE | A_BOLD);

    // Outer box  (row 1 … rows-4)
    int boxH = rows - 4;
    drawBox(1, 0, boxH, cols);

    // Fixed column widths that always fit inside the box
    int pkgW  = 24;                       // package name column
    int delW  = 5;                        // "[x]" + 2 spaces
    int fileW = cols - pkgW - delW - 6;   // remainder (- borders - gaps)
    if (fileW < 8) fileW = 8;

    // Column headers inside box
    attron(A_BOLD);
    mvprintw(2, 2, "%-*s  %-*s  %s",
        pkgW, "Package", fileW, "Source file", "Del");
    attroff(A_BOLD);
    mvhline(3, 1, ACS_HLINE, cols - 2);
    mvaddch(3, 0,        ACS_LTEE);
    mvaddch(3, cols - 1, ACS_RTEE);

    // Package rows
    int listH  = boxH - 4;
    int offset = 0;
    if (listCursor >= listH) offset = listCursor - listH + 1;

    for (int i = 0; i < listH && (i + offset) < (int)installed.size(); ++i) {
        int idx = i + offset;
        const PackageEntry& pkg = installed[idx];
        bool selected = (idx == listCursor);
        bool marked   = pkg.markedForDeletion;

        if (selected) attron(A_REVERSE);
        if (marked)   attron(A_BOLD);

        mvprintw(4 + i, 2, "%-*s  %-*s  %s",
            pkgW, trunc(pkg.name,     pkgW).c_str(),
            fileW, trunc(pkg.filePath, fileW).c_str(),
            marked ? "[x]" : "[ ]");

        if (marked)   attroff(A_BOLD);
        if (selected) attroff(A_REVERSE);
    }

    // Status bar
    int marked = 0;
    for (auto& p : installed) if (p.markedForDeletion) marked++;

    std::string status = "(" + std::to_string(installed.size()) + " packages  " 
                       + std::to_string(marked) + " marked)";
    std::string hints = "j/k move   d mark   a add   w save+rebuild   q quit";
    printStatusBar(rows - 1, status, hints, cols);
}

// ── MODE_SEARCH ───────────────────────────────────────────────────────────────

void TUI::drawSearch() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Title bar
    attron(A_REVERSE | A_BOLD);
    mvprintw(0, 0, "%-*s", cols, " nixedit – Add package");
    attroff(A_REVERSE | A_BOLD);

    // Search input box (rows 1-3)
    drawBox(1, 0, 3, cols);
    mvprintw(2, 2, "Search: %s", searchQuery.c_str());
    // Cursor
    attron(A_BOLD);
    mvprintw(2, 10 + (int)searchQuery.size(), "_");
    attroff(A_BOLD);
    // Status hint inside search box (right side)
    const char* hint = isSearching  ? "  searching…"
                     : searchPending ? "  …"
                     : "";
    mvprintw(2, cols - (int)strlen(hint) - 2, "%s", hint);

    // Results box (row 4 … rows-4)
    int resBoxH = rows - 7;
    drawBox(4, 0, resBoxH, cols);

    // Column headers
    attron(A_BOLD);
    mvprintw(5, 2, "%-28s %s", "Package", "Version");
    attroff(A_BOLD);
    mvhline(6, 1, ACS_HLINE, cols - 2);
    mvaddch(6, 0,        ACS_LTEE);
    mvaddch(6, cols - 1, ACS_RTEE);

    // Results
    int listH  = resBoxH - 4;
    int offset = 0;
    if (resultCursor >= listH) offset = resultCursor - listH + 1;

    if (searchQuery.empty()) {
        mvprintw(7, 2, "Type a package name to search nixpkgs…");
    } else if (!isSearching && searchResults.empty()) {
        mvprintw(7, 2, "No results for \"%s\"", searchQuery.c_str());
    } else {
        for (int i = 0; i < listH && (i + offset) < (int)searchResults.size(); ++i) {
            int idx = i + offset;
            const SearchResult& r = searchResults[idx];
            bool sel = (idx == resultCursor);
            if (sel) attron(A_REVERSE);
            mvprintw(7 + i, 2, "%-28s %s",
                trunc(r.packageName, 28).c_str(),
                trunc(r.version, cols - 34).c_str());
            if (sel) attroff(A_REVERSE);
        }
    }

    // Status bar
    std::string status = searchQuery.empty() ? "Type query, press Enter" 
                       : "(" + std::to_string(searchResults.size()) + " results)";
    std::string hints = searchResults.empty() ? "esc back" 
                      : "j/k move   enter select   esc back";
    printStatusBar(rows - 1, status, hints, cols);
}

// ── MODE_SELECT_MODULE ────────────────────────────────────────────────────────

void TUI::drawModuleSelect() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    attron(A_REVERSE | A_BOLD);
    mvprintw(0, 0, "%-*s", cols, " nixedit – Select installation target");
    attroff(A_REVERSE | A_BOLD);

    // Info box (rows 1-5)
    drawBox(1, 0, 4, cols);
    mvprintw(2, 2, "Package : %s  (%s)",
        pendingResult.packageName.c_str(),
        pendingResult.version.c_str());
    mvprintw(3, 2, "Select the .nix file to add this package to:");

    // Targets box
    int tboxH = rows - 9;
    drawBox(5, 0, tboxH, cols);
    attron(A_BOLD);
    mvprintw(6, 2, "%-36s %s", "File", "Insertion style");
    attroff(A_BOLD);
    mvhline(7, 1, ACS_HLINE, cols - 2);
    mvaddch(7, 0,        ACS_LTEE);
    mvaddch(7, cols - 1, ACS_RTEE);

    int styleW  = 26;                        // "with pkgs;  →  bare name"
    int fileColW = cols - styleW - 6;        // remainder
    if (fileColW < 12) fileColW = 12;

    for (int i = 0; i < (int)installTargets.size() && i < tboxH - 4; ++i) {
        const InstallTarget& t = installTargets[i];
        bool sel = (i == moduleCursor);
        if (sel) attron(A_REVERSE);
        const char* style = t.usesWithPkgs ? "with pkgs → bare name"
                                           : "no with   → pkgs.<name>";
        mvprintw(8 + i, 2, "%-*s  %s",
            fileColW, trunc(t.filePath, fileColW).c_str(), style);
        if (sel) attroff(A_REVERSE);
    }

    // Status bar
    std::string status = statusMsg.empty() 
                       ? "Installing: " + pendingResult.packageName 
                       : statusMsg;
    std::string hints = "j/k move   enter install   esc back";
    printStatusBar(rows - 1, status, hints, cols);
}

// ── rebuild output ────────────────────────────────────────────────────────────

void TUI::drawRebuildOutput(const std::string& out, bool ok) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    clear();

    attron(A_REVERSE | A_BOLD);
    mvprintw(0, 0, "%-*s", cols,
        ok ? " nixedit – Rebuild succeeded" : " nixedit – Rebuild FAILED");
    attroff(A_REVERSE | A_BOLD);

    drawBox(1, 0, rows - 3, cols);

    std::istringstream ss(out);
    std::string line;
    int row = 2;
    while (std::getline(ss, line) && row < rows - 4)
        mvprintw(row++, 2, "%s", trunc(line, cols - 4).c_str());

    mvhline(rows - 2, 0, ACS_HLINE, cols);
    mvprintw(rows - 1, 2, "press any key to continue");
    refresh();
    getch();
}

// ── search ────────────────────────────────────────────────────────────────────

void TUI::triggerSearch() {
    isSearching = true;
    searchPending = false;
    // Redraw to show "searching…" before the blocking call
    clear();
    drawSearch();
    refresh();

    PackageSearcher searcher;
    searchResults = searcher.search(searchQuery);
    resultCursor  = 0;
    isSearching   = false;
}

// ── install / remove ──────────────────────────────────────────────────────────

void TUI::doInstall(const InstallTarget& target) {
    // Phase 7: duplicate detection
    for (const auto& existing : installed) {
        if (existing.name == pendingResult.packageName) {
            statusMsg = "Already installed: " + pendingResult.packageName
                      + "  (" + existing.filePath + ")";
            return;
        }
    }

    std::string pkg = target.usesWithPkgs
        ? pendingResult.packageName
        : pendingResult.getPkgsAttribute();

    statusMsg = "Installing " + pendingResult.packageName + "...";
    clear();
    drawModuleSelect();
    refresh();

    PackageInserter inserter;
    if (!inserter.insertPackage(target, pkg)) {
        statusMsg = "Error: could not write to file.";
        return;
    }

    statusMsg = "Package written, starting rebuild...";
    clear();
    drawModuleSelect();
    refresh();

    // CRITICAL: Exit ncurses mode before running rebuild
    endwin();
    
    RebuildManager rebuild;
    bool ok = rebuild.rebuild();
    
    // Re-initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    drawRebuildOutput(rebuild.getOutput(), ok);
    
    // Clear status message after rebuild
    statusMsg.clear();
}

void TUI::doRemove() {
    bool any = false;
    for (auto& p : installed) if (p.markedForDeletion) { any = true; break; }
    if (!any) {
        statusMsg = "No packages marked for deletion";
        return;
    }
    
    statusMsg = "Removing packages...";
    clear();
    drawList();
    refresh();

    ConfigEditor editor;
    if (!editor.removePackages(installed)) {
        statusMsg = "Error: could not remove packages.";
        return;
    }
    
    statusMsg = "Starting rebuild...";
    clear();
    drawList();
    refresh();

    // CRITICAL: Exit ncurses mode before running rebuild
    endwin();
    
    RebuildManager rebuild;
    bool ok = rebuild.rebuild();
    
    // Re-initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    drawRebuildOutput(rebuild.getOutput(), ok);
    
    // Clear status message after rebuild
    statusMsg.clear();
}

// ── main loop ─────────────────────────────────────────────────────────────────

void TUI::run() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    // halfdelay: getch() returns ERR after N tenths of a second with no input
    // We use 1 tenth (100 ms) so we can poll for debounce expiry
    halfdelay(1);

    lastKeystroke  = Clock::now();
    searchPending  = false;
    isSearching    = false;

    while (true) {
        // Check debounce: 600 ms after last keystroke → fire search
        if (mode == MODE_SEARCH && searchPending && !isSearching) {
            auto elapsed = std::chrono::duration_cast<Ms>(
                Clock::now() - lastKeystroke).count();
            if (elapsed >= 600) {
                triggerSearch();
            }
        }

        clear();
        switch (mode) {
            case MODE_LIST:          drawList();         break;
            case MODE_SEARCH:        drawSearch();       break;
            case MODE_SELECT_MODULE: drawModuleSelect(); break;
        }
        refresh();

        int ch = getch();  // returns ERR on timeout

        if (mode == MODE_LIST) {
            if      (ch == 'q' || ch == 'Q')  { break; }
            else if (ch == 'w' || ch == 'W')  { doRemove(); }
            else if (ch == 'a' || ch == 'A')  {
                mode = MODE_SEARCH;
                searchQuery.clear();
                searchResults.clear();
                searchPending = false;
                isSearching   = false;
            }
            else if (ch == 'j' || ch == KEY_DOWN) {
                if (listCursor < (int)installed.size() - 1) listCursor++;
            }
            else if (ch == 'k' || ch == KEY_UP) {
                if (listCursor > 0) listCursor--;
            }
            else if ((ch == 'd' || ch == 'D') && !installed.empty()) {
                installed[listCursor].markedForDeletion =
                    !installed[listCursor].markedForDeletion;
            }

        } else if (mode == MODE_SEARCH) {
            if (ch == 27) {   // ESC
                nocbreak(); cbreak();  // restore blocking getch
                mode = MODE_LIST;
            } else if (ch == '\n' || ch == KEY_ENTER) {
                if (!searchResults.empty()) {
                    pendingResult = searchResults[resultCursor];
                    moduleCursor  = 0;
                    statusMsg.clear();
                    nocbreak(); cbreak();  // restore blocking getch
                    mode = MODE_SELECT_MODULE;
                }
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (!searchQuery.empty()) {
                    searchQuery.pop_back();
                    lastKeystroke = Clock::now();
                    searchPending = true;
                }
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (resultCursor < (int)searchResults.size() - 1) resultCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (resultCursor > 0) resultCursor--;
            } else if (ch != ERR && ch >= 32 && ch < 127) {
                searchQuery += (char)ch;
                lastKeystroke = Clock::now();
                searchPending = true;
            }

        } else if (mode == MODE_SELECT_MODULE) {
            if (ch == 27) {   // ESC
                halfdelay(1);
                mode = MODE_SEARCH;
            } else if (ch == '\n' || ch == KEY_ENTER) {
                if (!installTargets.empty()) {
                    doInstall(installTargets[moduleCursor]);
                    mode = MODE_LIST;
                }
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (moduleCursor < (int)installTargets.size() - 1) moduleCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (moduleCursor > 0) moduleCursor--;
            }
        }
    }

    endwin();
}
