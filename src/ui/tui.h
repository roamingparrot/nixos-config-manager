#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>
#include "../models/packageEntry.h"

/**
 * @brief Terminal User Interface for package management
 * 
 * This class handles the display and interaction with the user
 * through a terminal-based interface.
 */
class TUI {
private:
    std::vector<PackageEntry> packages;
    int cursorPosition;
    bool searchMode;
    std::string searchQuery;
    
    /**
     * @brief Draw the TUI interface
     */
    void draw();
    
    /**
     * @brief Handle user input
     */
    void handleInput();
    
public:
    /**
     * @brief Constructor
     */
    TUI();
    
    /**
     * @brief Initialize the TUI with packages
     * @param packages Vector of packages to display
     */
    void initialize(const std::vector<PackageEntry>& packages);
    
    /**
     * @brief Run the TUI main loop
     */
    void run();
    
    /**
     * @brief Display a message to the user
     * @param message Message to display
     */
    void showMessage(const std::string& message);
};

#endif // TUI_H