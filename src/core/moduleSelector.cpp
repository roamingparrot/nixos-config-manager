#include "moduleSelector.h"
#include "fileSyntaxDetector.h"
#include <fstream>
#include <iostream>

ModuleSelector::ModuleSelector() {}

std::vector<InstallTarget> ModuleSelector::discoverTargets(
    const std::vector<std::string>& modules) {
    
    availableTargets.clear();
    FileSyntaxDetector detector;
    
    for (const std::string& modulePath : modules) {
        if (hasSystemPackages(modulePath)) {
            InstallTarget target = detector.analyzeFile(modulePath);
            availableTargets.push_back(target);
        }
    }
    
    return availableTargets;
}

bool ModuleSelector::hasSystemPackages(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("environment.systemPackages") != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

std::vector<InstallTarget> ModuleSelector::getTargets() const {
    return availableTargets;
}

InstallTarget ModuleSelector::selectTarget(int index) {
    if (index >= 0 && index < static_cast<int>(availableTargets.size())) {
        return availableTargets[index];
    }
    return InstallTarget();
}
