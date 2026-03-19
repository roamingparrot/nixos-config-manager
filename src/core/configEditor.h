#ifndef CONFIG_EDITOR_H
#define CONFIG_EDITOR_H

#include <string>
#include <vector>
#include "../models/packageEntry.h"

/**
 * @brief Safely modifies Nix configuration files
 * 
 * This class handles the removal of packages from their source files
 * while preserving formatting and comments.
 */
class ConfigEditor {
private:
    /**
     * @brief Read file content
     * @param filePath Path to file
     * @return File content as string
     */
    std::string readFile(const std::string& filePath);
    
    /**
     * @brief Write content to file
     * @param filePath Path to file
     * @param content Content to write
     */
    void writeFile(const std::string& filePath, const std::string& content);

public:
    /**
     * @brief Constructor
     */
    ConfigEditor();
    
    /**
     * @brief Remove a single package from its source file
     * @param entry Package entry to remove
     * @return True if successful, false otherwise
     */
    bool removePackage(const PackageEntry& entry);
    
    /**
     * @brief Remove multiple packages from their source files
     * @param packages Vector of package entries to remove
     * @return True if all successful, false otherwise
     */
    bool removePackages(const std::vector<PackageEntry>& packages);
};

#endif // CONFIG_EDITOR_H