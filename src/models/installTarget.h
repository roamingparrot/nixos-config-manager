#ifndef INSTALL_TARGET_H
#define INSTALL_TARGET_H

#include <string>

/**
 * @brief Represents a target location for package installation
 * 
 * This struct contains information about where and how to insert
 * a package into a NixOS configuration file.
 */
struct InstallTarget {
    std::string filePath;         // Absolute path: "/etc/nixos/modules/coding.nix"
    std::string fileName;         // Short name: "coding.nix"
    bool usesWithPkgs;            // true if file has "with pkgs;" before systemPackages
    int insertLine;               // Line number to insert at (0 = not determined)
    std::string indentation;      // "      " (preserve existing indentation)
    
    /**
     * @brief Constructor with default values
     */
    InstallTarget() : usesWithPkgs(false), insertLine(0), indentation("  ") {}
    
    /**
     * @brief Constructor with path
     */
    InstallTarget(const std::string& path, bool withPkgs = false)
        : filePath(path), usesWithPkgs(withPkgs), insertLine(0), indentation("  ") {
        // Extract filename from path
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            fileName = path.substr(pos + 1);
        } else {
            fileName = path;
        }
    }
};

#endif // INSTALL_TARGET_H
