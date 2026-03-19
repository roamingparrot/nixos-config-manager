#include "ui/tui.h"
#include "core/packageSearcher.h"
#include "core/packageInserter.h"
#include "core/configEditor.h"
#include "core/rebuildManager.h"
#include <ncurses.h>
#include <sstream>
#include <algorithm>

// ── helpers ───────────────────────────────────────────────────────────────────

static std::string truncate(const std::string& s, int max) {
    if ((int)s.size() <= max) return s;
    return s.substr(0, max - 1) + "~";
}

static void drawHLine(int row, int cols) {
    mvhline(row, 0, ACS_HLINE, cols);
}

// ── ctor / init ───────────────────────────────────────────────────────────────

TUI::TUI()
    : mode(MODE_LIST), listCursor(0), resultCursor(0),
      moduleCursor(0), actionDone(false) {}

void TUI::initialize(const std::vector<PackageEntry>& pkgs,
                     const std::vector<InstallTarget>& targets) {
    installed      = pkgs;
    installTargets = targets;
}

// ── status line ───────────────────────────────────────────────────────────────

void TUI::setStatus(const std::string& msg) { statusMsg = msg; }

// ── draw helpers ──────────────────────────────────────────────────────────────

void TUI::drawBorder(int row, int cols) { drawHLine(row, cols); }

void TUI::drawList() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Header
    attron(A_BOLD);
    mvprintw(0, 0, "dotman  –  NixOS package manager");
    attroff(A_BOLD);
    drawBorder(1, cols);

    // Column headings
    mvprintw(2, 2,  "Package");
    mvprintw(2, 28, "Source file");
    mvprintw(2, cols - 6, "Del?");
    drawBorder(3, cols);

    // Package rows
    int listRows = rows - 7;   // rows available for packages
    int startRow = 4;

    // Scroll offset so cursor stays visible
    int offset = 0;
    if (listCursor >= listRows) offset = listCursor - listRows + 1;

    for (int i = 0; i < listRows && (i + offset) < (int)installed.size(); ++i) {
        int idx = i + offset;
        const PackageEntry& pkg = installed[idx];
        bool selected = (idx == listCursor);

        if (selected) attron(A_REVERSE);
        if (pkg.markedForDeletion) attron(A_BOLD);

        mvprintw(startRow + i, 2,  "%s", truncate(pkg.name,     24).c_str());
        mvprintw(startRow + i, 28, "%s", truncate(pkg.filePath, cols - 36).c_str());
        mvprintw(startRow + i, cols - 5, "%s", pkg.markedForDeletion ? "[x]" : "   ");

        if (pkg.markedForDeletion) attroff(A_BOLD);
        if (selected) attroff(A_REVERSE);
    }

    // Footer
    drawBorder(rows - 3, cols);
    int marked = 0;
    for (auto& p : installed) if (p.markedForDeletion) marked++;
    mvprintw(rows - 2, 2, "%zu installed  |  %d marked for removal",
             installed.size(), marked);
    drawBorder(rows - 1, cols);
    mvprintw(rows - 1, 2,
             "j/k Move   d Mark/unmark   a Add package   w Save+rebuild   q Quit");
}

void TUI::drawSearch() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Header
    attron(A_BOLD);
    mvprintw(0, 0, "dotman  –  Add package");
    attroff(A_BOLD);
    drawBorder(1, cols);

    // Search bar
    mvprintw(2, 2, "Search: %s", searchQuery.c_str());
    // Draw blinking cursor character
    int cursorCol = 10 + (int)searchQuery.size();
    attron(A_BLINK | A_BOLD);
    mvprintw(2, cursorCol, "_");
    attroff(A_BLINK | A_BOLD);

    drawBorder(3, cols);

    // Column headings
    mvprintw(4, 2,  "Package");
    mvprintw(4, 28, "Version");
    drawBorder(5, cols);

    // Results list
    int listRows = rows - 9;
    int offset   = 0;
    if (resultCursor >= listRows) offset = resultCursor - listRows + 1;

    if (searchQuery.empty()) {
        mvprintw(6, 2, "Start typing to search nixpkgs...");
    } else if (searchResults.empty()) {
        mvprintw(6, 2, "No results for \"%s\"", searchQuery.c_str());
    } else {
        for (int i = 0; i < listRows && (i + offset) < (int)searchResults.size(); ++i) {
            int idx = i + offset;
            const SearchResult& r = searchResults[idx];
            bool selected = (idx == resultCursor);

            if (selected) attron(A_REVERSE);
            mvprintw(6 + i, 2,  "%s", truncate(r.packageName, 24).c_str());
            mvprintw(6 + i, 28, "%s", truncate(r.version, cols - 32).c_str());
            if (selected) attroff(A_REVERSE);
        }
    }

    // Footer
    drawBorder(rows - 3, cols);
    mvprintw(rows - 2, 2, "%zu results", searchResults.size());
    drawBorder(rows - 1, cols);
    mvprintw(rows - 1, 2,
             "Type to search   j/k Move   Enter Select   Esc Cancel");
}

void TUI::drawModuleSelect() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    attron(A_BOLD);
    mvprintw(0, 0, "dotman  –  Select installation target");
    attroff(A_BOLD);
    drawBorder(1, cols);

    mvprintw(2, 2, "Installing: %s", pendingResult.packageName.c_str());
    mvprintw(3, 2, "Choose which file to add it to:");
    drawBorder(4, cols);

    mvprintw(5, 2,  "File");
    mvprintw(5, 40, "Syntax");
    drawBorder(6, cols);

    int listRows = rows - 10;
    for (int i = 0; i < (int)installTargets.size() && i < listRows; ++i) {
        const InstallTarget& t = installTargets[i];
        bool selected = (i == moduleCursor);

        if (selected) attron(A_REVERSE);
        mvprintw(7 + i, 2,  "%s", truncate(t.fileName, 36).c_str());
        mvprintw(7 + i, 40, "%s", t.usesWithPkgs ? "with pkgs;  →  pkgname"
                                                  : "without with  →  pkgs.pkgname");
        if (selected) attroff(A_REVERSE);
    }

    drawBorder(rows - 3, cols);
    if (!statusMsg.empty()) mvprintw(rows - 2, 2, "%s", statusMsg.c_str());
    drawBorder(rows - 1, cols);
    mvprintw(rows - 1, 2, "j/k Move   Enter Install here   Esc Back");
}

// ── action handlers ───────────────────────────────────────────────────────────

void TUI::runSearch() {
    if (searchQuery.empty()) {
        searchResults.clear();
        return;
    }
    PackageSearcher searcher;
    searchResults = searcher.search(searchQuery);
    resultCursor  = 0;
}

void TUI::doInstall(const InstallTarget& target) {
    std::string pkgToInsert = target.usesWithPkgs
        ? pendingResult.packageName
        : pendingResult.getPkgsAttribute();

    PackageInserter inserter;
    if (!inserter.insertPackage(target, pkgToInsert)) {
        setStatus("Error: could not insert package into file.");
        return;
    }

    setStatus("Running nixos-rebuild switch...");
    RebuildManager rebuild;
    bool ok = rebuild.rebuild();
    showRebuildOutput(rebuild.getOutput(), ok);
}

void TUI::doRemove() {
    ConfigEditor editor;
    if (!editor.removePackages(installed)) {
        setStatus("Error: could not remove packages.");
        return;
    }

    setStatus("Running nixos-rebuild switch...");
    RebuildManager rebuild;
    bool ok = rebuild.rebuild();
    showRebuildOutput(rebuild.getOutput(), ok);
}

void TUI::showRebuildOutput(const std::string& out, bool ok) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    clear();

    attron(A_BOLD);
    mvprintw(0, 0, ok ? "dotman  –  Rebuild succeeded"
                      : "dotman  –  Rebuild FAILED");
    attroff(A_BOLD);
    mvhline(1, 0, ACS_HLINE, cols);

    // Print as many lines of output as fit
    std::istringstream stream(out);
    std::string line;
    int row = 2;
    while (std::getline(stream, line) && row < rows - 3) {
        mvprintw(row++, 2, "%s", truncate(line, cols - 4).c_str());
    }

    mvhline(rows - 2, 0, ACS_HLINE, cols);
    mvprintw(rows - 1, 2, "Press any key to continue...");
    refresh();
    getch();
}

// ── main loop ─────────────────────────────────────────────────────────────────

void TUI::run() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    std::string lastQuery;

    while (true) {
        // Live search: re-run whenever query changes
        if (mode == MODE_SEARCH && searchQuery != lastQuery) {
            lastQuery = searchQuery;
            runSearch();
        }

        clear();
        switch (mode) {
            case MODE_LIST:         drawList();         break;
            case MODE_SEARCH:       drawSearch();       break;
            case MODE_SELECT_MODULE: drawModuleSelect(); break;
        }
        refresh();

        int ch = getch();

        // ── MODE_LIST ────────────────────────────────────────────────────────
        if (mode == MODE_LIST) {
            if (ch == 'q' || ch == 'Q') {
                break;
            } else if (ch == 'w' || ch == 'W') {
                doRemove();
            } else if (ch == 'a' || ch == 'A') {
                mode = MODE_SEARCH;
                searchQuery.clear();
                searchResults.clear();
                lastQuery = "\xFF"; // force refresh
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (listCursor < (int)installed.size() - 1) listCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (listCursor > 0) listCursor--;
            } else if (ch == 'd' || ch == 'D') {
                if (!installed.empty())
                    installed[listCursor].markedForDeletion =
                        !installed[listCursor].markedForDeletion;
            }

        // ── MODE_SEARCH ──────────────────────────────────────────────────────
        } else if (mode == MODE_SEARCH) {
            if (ch == 27) {   // ESC
                mode = MODE_LIST;
            } else if (ch == '\n' || ch == KEY_ENTER) {
                if (!searchResults.empty()) {
                    pendingResult = searchResults[resultCursor];
                    moduleCursor  = 0;
                    mode = MODE_SELECT_MODULE;
                }
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (!searchQuery.empty()) searchQuery.pop_back();
            } else if (ch == 'j' || ch == KEY_DOWN) {
                if (resultCursor < (int)searchResults.size() - 1) resultCursor++;
            } else if (ch == 'k' || ch == KEY_UP) {
                if (resultCursor > 0) resultCursor--;
            } else if (ch >= 32 && ch < 127) {
                searchQuery += (char)ch;
            }

        // ── MODE_SELECT_MODULE ───────────────────────────────────────────────
        } else if (mode == MODE_SELECT_MODULE) {
            if (ch == 27) {   // ESC – back to search
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
