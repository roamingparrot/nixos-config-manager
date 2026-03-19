#include "tui.h"
#include <ncurses.h>
#include <iostream>

TUI::TUI() : cursorPosition(0), searchMode(false), shouldSave(false) {}

void TUI::initialize(const std::vector<PackageEntry>& packages) {
    this->packages = packages;
}

bool TUI::run() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    // Main loop
    bool running = true;
    while (running) {
        draw();
        int ch = getch();
        
        switch (ch) {
            case 'q':
            case 'Q':
                // Quit without saving
                shouldSave = false;
                running = false;
                break;
            case 'w':
            case 'W':
                // Save and rebuild
                shouldSave = true;
                running = false;
                break;
            case 'j':
            case KEY_DOWN:
                // Move cursor down
                if (cursorPosition < (int)packages.size() - 1) {
                    cursorPosition++;
                }
                break;
            case 'k':
            case KEY_UP:
                // Move cursor up
                if (cursorPosition > 0) {
                    cursorPosition--;
                }
                break;
            case 'd':
            case 'D':
                // Toggle deletion mark
                if (!packages.empty()) {
                    packages[cursorPosition].markedForDeletion = 
                        !packages[cursorPosition].markedForDeletion;
                }
                break;
            default:
                // Ignore other keys for now
                break;
        }
    }
    
    // Clean up ncurses
    endwin();
    
    return shouldSave;
}

void TUI::draw() {
    // Clear screen
    clear();
    
    // Draw header
    mvprintw(0, 0, "NixOS Package Manager");
    mvprintw(1, 0, "==========================================");
    
    // Count marked packages
    int markedCount = 0;
    for (const auto& pkg : packages) {
        if (pkg.markedForDeletion) markedCount++;
    }
    
    // Draw packages
    int startLine = 3;
    for (size_t i = 0; i < packages.size(); ++i) {
        const PackageEntry& package = packages[i];
        std::string marker = package.markedForDeletion ? "[D]" : "[ ]";
        mvprintw(startLine + i, 0, "%s %s", 
                (i == (size_t)cursorPosition) ? ">" : " ",
                package.name.c_str());
        mvprintw(startLine + i, 30, "(%s)", package.filePath.c_str());
        mvprintw(startLine + i, 80, "%s", marker.c_str());
    }
    
    // Draw footer
    mvprintw(LINES - 2, 0, "==========================================");
    mvprintw(LINES - 1, 0, "%d packages | %d marked | [d] Mark | [w] Save & Rebuild | [q] Quit", 
             (int)packages.size(), markedCount);
    
    // Refresh screen
    refresh();
}

std::vector<PackageEntry> TUI::getPackages() const {
    return packages;
}