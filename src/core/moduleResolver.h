#ifndef MODULE_RESOLVER_H
#define MODULE_RESOLVER_H

#include <string>
#include <vector>
#include <set>
#include "../models/moduleInfo.h"

/**
 * @brief Discovers all NixOS configuration files by following imports
 * 
 * This class recursively resolves all imported modules starting from
 * a given entry point (typically /etc/nixos/configuration.nix).
 */
class ModuleResolver {
private:
    std::set<std::string> visitedFiles;  // Track visited files to avoid duplicates
    
    /**
     * @brief Parse imports from file content
     * @param content File content to parse
     * @return Vector of imported file paths
     */
    std::vector<std::string> parseImports(const std::string& content);
    
    /**
     * @brief Resolve relative path to absolute path
     * @param base Base directory path
     * @param relative Relative path to resolve
     * @return Absolute path
     */
    std::string resolvePath(const std::string& base, const std::string& relative);
    
    /**
     * @brief Read file content
     * @param filePath Path to file
     * @return File content as string
     */
    std::string readFile(const std::string& filePath);
    
    /**
     * @brief Get directory path from file path
     * @param filePath Full file path
     * @return Directory path
     */
    std::string getDirectory(const std::string& filePath);

public:
    /**
     * @brief Constructor
     */
    ModuleResolver();
    
    /**
     * @brief Resolve all modules starting from entry file
     * @param entryFile Entry point configuration file
     * @return Vector of all resolved module paths
     */
    std::vector<std::string> resolveAllModules(const std::string& entryFile);
    
    /**
     * @brief Load module information for a file
     * @param filePath Path to module file
     * @return ModuleInfo struct with file information
     */
    ModuleInfo loadModule(const std::string& filePath);
};

#endif // MODULE_RESOLVER_H