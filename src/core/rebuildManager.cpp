#include "core/rebuildManager.h"
#include <iostream>
#include <memory>
#include <stdexcept>

RebuildManager::RebuildManager() : isRebuilding(false) {}

bool RebuildManager::rebuild() {
    isRebuilding = true;
    lastOutput.clear();
    
    try {
        // Execute nixos-rebuild switch and capture output
        FILE* pipe = popen("nixos-rebuild switch --show-trace 2>&1", "r");
        if (!pipe) {
            throw std::runtime_error("Failed to execute nixos-rebuild");
        }
        
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            lastOutput += buffer;
        }
        
        int result = pclose(pipe);
        isRebuilding = false;
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