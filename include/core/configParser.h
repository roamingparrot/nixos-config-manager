#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <string>
#include <vector>
#include "models/packageEntry.h"

/**
 * @brief Extracts package definitions from parsed files
 * 
 * This class parses Nix configuration files to extract 
 * environment.systemPackages declarations.
 */
class ConfigParser {
private:
    /**
     * @brief Find matching closing bracket
     * @param content File content
     * @param start Position of opening bracket
     * @return Position of matching closing bracket
     */
    int findMatchingBracket(const std::string& content, int start);
    
    /**
     * @brief Extract package names from a package block
     * @param block Content between brackets
     * @param startLine Starting line number of block
     * @return Vector of package names
     */
    std::vector<std::string> extractPackageNames(const std::string& block, int startLine);
    
    /**
     * @brief Split string by newline
     * @param str String to split
     * @return Vector of lines
     */
    std::vector<std::string> splitLines(const std::string& str);

public:
    /**
     * @brief Constructor
     */
    ConfigParser();
    
    /**
     * @brief Extract packages from file content
     * @param fileContent Content of the configuration file
     * @param filePath Path to the configuration file
     * @return Vector of PackageEntry structs
     */
    std::vector<PackageEntry> extractPackages(const std::string& fileContent,
                                             const std::string& filePath);
};

#endif // CONFIG_PARSER_H