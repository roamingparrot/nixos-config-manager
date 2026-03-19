#ifndef REBUILD_MANAGER_H
#define REBUILD_MANAGER_H

#include <string>
#include <vector>

/**
 * @brief Executes nixos-rebuild and reports results
 * 
 * This class handles the execution of nixos-rebuild switch
 * and captures its output for display in the TUI.
 */
class RebuildManager {
private:
    std::string lastOutput;  // Last rebuild output
    bool isRebuilding;       // Rebuild in progress flag
    
public:
    /**
     * @brief Constructor
     */
    RebuildManager();
    
    /**
     * @brief Run nixos-rebuild switch
     * @return True if successful, false otherwise
     */
    bool rebuild();
    
    /**
     * @brief Get the output from the last rebuild
     * @return Output string
     */
    std::string getOutput() const;
    
    /**
     * @brief Check if a rebuild is currently in progress
     * @return True if rebuilding, false otherwise
     */
    bool isRunning() const;
};

#endif // REBUILD_MANAGER_H