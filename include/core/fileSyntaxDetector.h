#ifndef FILE_SYNTAX_DETECTOR_H
#define FILE_SYNTAX_DETECTOR_H

#include <string>
#include "models/installTarget.h"

/**
 * @brief Detects syntax patterns in Nix configuration files
 * 
 * This class analyzes Nix files to determine if they use
 * "with pkgs;" syntax and find appropriate insertion points.
 */
class FileSyntaxDetector {
private:
    /**
     * @brief Check if "with pkgs;" appears before systemPackages
     * @param content File content
     * @param systemPackagesPos Position of environment.systemPackages
     * @return true if "with pkgs;" is used
     */
    bool hasWithPkgs(const std::string& content, size_t systemPackagesPos);
    
    /**
     * @brief Find the opening bracket of the package list
     * @param content File content
     * @param systemPackagesPos Position of environment.systemPackages
     * @return Position of '[' or -1 if not found
     */
    int findPackageListStart(const std::string& content, size_t systemPackagesPos);
    
    /**
     * @brief Detect indentation used in file
     * @param content File content
     * @param lineNumber Line number to check
     * @return Indentation string (spaces/tabs)
     */
    std::string detectIndentation(const std::string& content, int lineNumber);
    
    /**
     * @brief Read file content
     * @param filePath Path to file
     * @return File content
     */
    std::string readFile(const std::string& filePath);

public:
    /**
     * @brief Constructor
     */
    FileSyntaxDetector();
    
    /**
     * @brief Analyze file and create InstallTarget
     * @param filePath Path to Nix configuration file
     * @return InstallTarget with syntax information
     */
    InstallTarget analyzeFile(const std::string& filePath);
};

#endif // FILE_SYNTAX_DETECTOR_H
