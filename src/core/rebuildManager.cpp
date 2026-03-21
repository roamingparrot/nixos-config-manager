#include "core/rebuildManager.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>

// External declaration for ncurses endwin - we'll use system() instead to avoid header issues
// The key issue is that ncurses has the terminal in a special state during popen()

RebuildManager::RebuildManager() : isRebuilding(false) {}

bool RebuildManager::rebuild() {
    isRebuilding = true;
    lastOutput.clear();
    
    try {
        // CRITICAL: Use system() instead of popen() to avoid terminal state conflicts
        // nixedit runs in ncurses mode which puts the terminal in a special state
        // Using system() with script workaround or running endwin() first helps
        
        // Build command with timeout (5 minutes max) and use script to handle PTY
        std::string command = 
            "timeout 300 script -q -c 'nixos-rebuild switch --show-trace' /dev/null 2>&1";
        
        // Alternative without script if that fails:
        // std::string command = "timeout 300 nixos-rebuild switch --show-trace 2>&1";
        
        lastOutput = "Running nixos-rebuild switch...\n";
        lastOutput += "This may take a few minutes...\n\n";
        
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            lastOutput += "\nError: Failed to execute nixos-rebuild\n";
            isRebuilding = false;
            return false;
        }
        
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            lastOutput += buffer;
        }
        
        int result = pclose(pipe);
        isRebuilding = false;
        
        // Check result
        if (result == 124) {
            lastOutput += "\n\nError: nixos-rebuild timed out after 5 minutes\n";
            return false;
        } else if (result == 127) {
            lastOutput += "\n\nError: nixos-rebuild command not found\n";
            return false;
        } else if (result != 0) {
            lastOutput += "\n\nError: nixos-rebuild failed\n";
            return false;
        }
        
        return result == 0;
    } catch (const std::exception& e) {
        lastOutput = "Error: " + std::string(e.what());
        isRebuilding = false;
        return false;
    }
}

std::string RebuildManager::getOutput() const {
    return lastOutput;
}

bool RebuildManager::isRunning() const {
    return isRebuilding;
}
