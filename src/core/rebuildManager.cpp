#include "core/rebuildManager.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>

// External declaration for ncurses endwin - we'll use system() instead to avoid header issues
// The key issue is that ncurses has the terminal in a special state during popen()

RebuildManager::RebuildManager() : isRebuilding(false) {}

bool RebuildManager::rebuild() {
    isRebuilding = true;
    lastOutput.clear();
    
    try {
        // Since we call endwin() before this, we can use a simple command
        // Timeout after 10 minutes to prevent hanging forever
        std::string command = "timeout 600 nixos-rebuild switch --show-trace 2>&1";
        
        lastOutput = "Running nixos-rebuild switch...\n";
        lastOutput += "This may take several minutes...\n\n";
        
        std::cerr << "DEBUG: Executing: " << command << std::endl;
        
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            lastOutput += "\nError: Failed to execute nixos-rebuild\n";
            isRebuilding = false;
            return false;
        }
        
        // Read output in real-time
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            lastOutput += buffer;
            // Also print to stderr for debugging
            std::cerr << buffer;
        }
        
        int result = pclose(pipe);
        isRebuilding = false;
        
        std::cerr << "DEBUG: nixos-rebuild exit code: " << result << std::endl;
        
        // Check result (timeout returns 124, shifted by 8 bits = 31744)
        if (WEXITSTATUS(result) == 124) {
            lastOutput += "\n\nError: nixos-rebuild timed out after 10 minutes\n";
            return false;
        } else if (result == 127 || WEXITSTATUS(result) == 127) {
            lastOutput += "\n\nError: nixos-rebuild command not found\n";
            return false;
        } else if (result != 0) {
            lastOutput += "\n\nError: nixos-rebuild failed with exit code: " + std::to_string(WEXITSTATUS(result)) + "\n";
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
