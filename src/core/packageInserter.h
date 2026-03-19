#ifndef PACKAGE_INSERTER_H
#define PACKAGE_INSERTER_H

#include <string>
#include "../models/installTarget.h"

/**
 * @brief Inserts packages into Nix configuration files
 * 
 * This class handles the insertion of packages into the correct
 * location in a Nix file, preserving formatting and syntax.
 */
class PackageInserter {
private:
    /**
     * @brief Read file content
     * @param filePath Path to file
     * @return File content
     */
    std::string readFile(const std::string& filePath);
    
    /**
     * @brief Write content to file
     * @param filePath Path to file
     * @param content Content to write
     */
    void writeFile(const std::string& filePath, const std::string& content);
    
    /**
     * @brief Find insertion position in content
     * @param content File content
     * @param target Installation target with position info
     * @return Position to insert at, or -1 if not found
     */
    int findInsertPosition(const std::string& content, const InstallTarget& target);

public:
    /**
     * @brief Constructor
     */
    PackageInserter();
    
    /**
     * @brief Insert a package into a target file
     * @param target Installation target with file and syntax info
     * @param packageName Package to insert (e.g., "neovim" or "pkgs.neovim")
     * @return true if successful
     */
    bool insertPackage(const InstallTarget& target, const std::string& packageName);
};

#endif // PACKAGE_INSERTER_H
