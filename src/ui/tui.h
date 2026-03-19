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
    bool shouldSave;
    
    /**
     * @brief Draw the TUI interface
     */
    void draw();
    
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
     * @return True if user wants to save, false otherwise
     */
    bool run();
    
    /**
     * @brief Get packages (potentially modified)
     * @return Vector of packages
     */
    std::vector<PackageEntry> getPackages() const;
};

#endif // TUI_H