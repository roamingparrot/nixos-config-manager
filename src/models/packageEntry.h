#ifndef PACKAGE_ENTRY_H
#define PACKAGE_ENTRY_H

#include <string>

/**
 * @brief Represents a single package from a configuration file
 * 
 * This struct holds information about a package including its name,
 * source file location, and line numbers for precise editing.
 */
struct PackageEntry {
    std::string name;        // Package name (e.g., "git")
    std::string filePath;    // Absolute path to source file
    int startLine;          // Line where package is defined
    int endLine;            // End line (usually same as start)
    bool markedForDeletion; // User marked for removal
    
    /**
     * @brief Constructor with default values
     */
    PackageEntry() : startLine(0), endLine(0), markedForDeletion(false) {}
    
    /**
     * @brief Constructor with all values
     * @param name Package name
     * @param filePath Source file path
     * @param startLine Starting line number
     * @param endLine Ending line number
     */
    PackageEntry(const std::string& name, const std::string& filePath, 
                int startLine, int endLine) 
        : name(name), filePath(filePath), startLine(startLine), 
          endLine(endLine), markedForDeletion(false) {}
};

#endif // PACKAGE_ENTRY_H