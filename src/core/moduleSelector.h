#ifndef MODULE_SELECTOR_H
#define MODULE_SELECTOR_H

#include <string>
#include <vector>
#include "../models/installTarget.h"

/**
 * @brief Selects target modules for package installation
 * 
 * This class discovers all modules with environment.systemPackages
 * and allows the user to select a target for installation.
 */
class ModuleSelector {
private:
    std::vector<InstallTarget> availableTargets;
    
    /**
     * @brief Check if a module has environment.systemPackages
     * @param filePath Path to module file
     * @return true if module has systemPackages
     */
    bool hasSystemPackages(const std::string& filePath);

public:
    /**
     * @brief Constructor
     */
    ModuleSelector();
    
    /**
     * @brief Discover all modules with systemPackages
     * @param modules List of all module paths from ModuleResolver
     * @return Vector of install targets
     */
    std::vector<InstallTarget> discoverTargets(const std::vector<std::string>& modules);
    
    /**
     * @brief Get available installation targets
     * @return Vector of install targets
     */
    std::vector<InstallTarget> getTargets() const;
    
    /**
     * @brief Select target by index
     * @param index Index in availableTargets vector
     * @return Selected InstallTarget
     */
    InstallTarget selectTarget(int index);
};

#endif // MODULE_SELECTOR_H
