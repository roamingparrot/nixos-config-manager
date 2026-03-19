#include "configParser.h"
#include <iostream>
#include <regex>
#include <sstream>

ConfigParser::ConfigParser() {}

std::vector<PackageEntry> ConfigParser::extractPackages(const std::string& fileContent,
                                                      const std::string& filePath) {
    std::vector<PackageEntry> packages;
    
    // Regex to match environment.systemPackages = with pkgs;
    std::regex packageBlockRegex(R"(environment\.systemPackages\s*=\s*with\s+pkgs\s*;)", 
                               std::regex_constants::icase);
    std::sregex_iterator iter(fileContent.begin(), fileContent.end(), packageBlockRegex);
    std::sregex_iterator end;
    
    // For each match, find the package block
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        size_t startPos = match.position() + match.length();
        
        // Find the opening bracket
        size_t openBracketPos = fileContent.find('[', startPos);
        if (openBracketPos == std::string::npos) {
            continue;
        }
        
        // Find the matching closing bracket
        int closeBracketPos = findMatchingBracket(fileContent, openBracketPos);
        if (closeBracketPos == -1) {
            continue;
        }
        
        // Extract the content between brackets
        std::string blockContent = fileContent.substr(openBracketPos + 1, 
                                                     closeBracketPos - openBracketPos - 1);
        
        // Calculate line numbers
        int startLine = 1;
        for (size_t i = 0; i < openBracketPos; ++i) {
            if (fileContent[i] == '\n') {
                startLine++;
            }
        }
        
        // Extract package names from the block
        std::vector<std::string> packageNames = extractPackageNames(blockContent, startLine);
        
        // Create PackageEntry for each package
        for (const std::string& packageName : packageNames) {
            // For now, we'll set line numbers to startLine + 1 for each package
            // In a more sophisticated implementation, we'd track exact line numbers
            packages.emplace_back(packageName, filePath, startLine + 1, startLine + 1);
        }
    }
    
    return packages;
}

int ConfigParser::findMatchingBracket(const std::string& content, int start) {
    int bracketCount = 1;
    for (size_t i = start + 1; i < content.length(); ++i) {
        if (content[i] == '[') {
            bracketCount++;
        } else if (content[i] == ']') {
            bracketCount--;
            if (bracketCount == 0) {
                return i;
            }
        }
    }
    return -1; // Not found
}

std::vector<std::string> ConfigParser::extractPackageNames(const std::string& block, int startLine) {
    (void)startLine; // Unused for now
    std::vector<std::string> packages;
    
    // Regex to match package names (word-only lines)
    std::regex packageRegex(R"(^\s*([a-zA-Z][a-zA-Z0-9_-]*)\s*$)");
    std::vector<std::string> lines = splitLines(block);
    
    for (const std::string& line : lines) {
        std::smatch match;
        if (std::regex_match(line, match, packageRegex)) {
            packages.push_back(match[1].str());
        }
    }
    
    return packages;
}

std::vector<std::string> ConfigParser::splitLines(const std::string& str) {
    std::vector<std::string> lines;
    std::stringstream ss(str);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    
    return lines;
}