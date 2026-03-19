#ifndef MODULE_INFO_H
#define MODULE_INFO_H

#include <string>
#include <vector>

/**
 * @brief Represents a NixOS configuration module file
 * 
 * This struct holds information about a NixOS configuration module
 * including its path, content, and imported modules.
 */
struct ModuleInfo {
    std::string absolutePath;           // Absolute path to the module file
    std::string content;                // Full content of the file
    std::vector<std::string> imports;   // Imported file paths
    
    /**
     * @brief Constructor with default values
     */
    ModuleInfo() {}
    
    /**
     * @brief Constructor with path and content
     * @param path Absolute path to the module file
     * @param fileContent Full content of the file
     */
    ModuleInfo(const std::string& path, const std::string& fileContent) 
        : absolutePath(path), content(fileContent) {}
};

#endif // MODULE_INFO_H