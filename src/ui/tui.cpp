#include "tui.h"
#include <ncurses.h>
#include <iostream>

TUI::TUI() : cursorPosition(0), searchMode(false) {}

void TUI::initialize(const std::vector<PackageEntry>& packages) {
    this->packages = packages;
}

void TUI::run() {
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
                // Quit
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
}

void TUI::draw() {
    // Clear screen
    clear();
    
    // Draw header
    mvprintw(0, 0, "NixOS Package Manager");
    
    // Draw packages
    for (size_t i = 0; i < packages.size(); ++i) {
        const PackageEntry& package = packages[i];
        std::string marker = (package.markedForDeletion) ? "[D]" : "[ ]";
        mvprintw(i + 2, 0, "%s %s (%s)", 
                (i == cursorPosition) ? ">" : " ",
                package.name.c_str(),
                package.filePath.c_str());
        mvprintw(i + 2, 50, "%s", marker.c_str());
    }
    
    // Draw footer
    mvprintw(LINES - 1, 0, "[q] Quit | [d] Mark/Unmark | [w] Save & Rebuild");
    
    // Refresh screen
    refresh();
}

void TUI::showMessage(const std::string& message) {
    // For now, just print to stderr
    std::cerr << message << std::endl;
}