#include "core/fileSyntaxDetector.h"
#include <fstream>
#include <sstream>
#include <iostream>

FileSyntaxDetector::FileSyntaxDetector() {}

InstallTarget FileSyntaxDetector::analyzeFile(const std::string& filePath) {
    InstallTarget target(filePath);
    
    try {
        std::string content = readFile(filePath);
        
        // Find environment.systemPackages
        size_t sysPackagesPos = content.find("environment.systemPackages");
        if (sysPackagesPos == std::string::npos) {
            return target;  // No systemPackages found
        }
        
        // Check if uses "with pkgs;"
        target.usesWithPkgs = hasWithPkgs(content, sysPackagesPos);
        
        // Find package list start position
        int listStart = findPackageListStart(content, sysPackagesPos);
        if (listStart != -1) {
            // Calculate line number
            int lineNum = 1;
            for (int i = 0; i < listStart; ++i) {
                if (content[i] == '\n') lineNum++;
            }
            target.insertLine = lineNum;
            
            // Detect indentation
            target.indentation = detectIndentation(content, lineNum);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error analyzing file " << filePath << ": " << e.what() << std::endl;
    }
    
    return target;
}

bool FileSyntaxDetector::hasWithPkgs(const std::string& content, size_t systemPackagesPos) {
    // Search backwards from systemPackagesPos
    size_t searchStart = (systemPackagesPos > 200) ? systemPackagesPos - 200 : 0;
    std::string searchArea = content.substr(searchStart, systemPackagesPos - searchStart);
    
    // Look for "with pkgs;"
    return searchArea.find("with pkgs;") != std::string::npos;
}

int FileSyntaxDetector::findPackageListStart(const std::string& content, 
                                            size_t systemPackagesPos) {
    // Find the opening bracket after systemPackages
    size_t bracketPos = content.find('[', systemPackagesPos);
    if (bracketPos == std::string::npos) {
        return -1;
    }
    
    return static_cast<int>(bracketPos);
}

std::string FileSyntaxDetector::detectIndentation(const std::string& content, int lineNumber) {
    // Find the start of the target line
    int currentLine = 1;
    size_t lineStart = 0;
    
    for (size_t i = 0; i < content.length() && currentLine < lineNumber; ++i) {
        if (content[i] == '\n') {
            currentLine++;
            lineStart = i + 1;
        }
    }
    
    // Count spaces/tabs at line start
    std::string indent;
    for (size_t i = lineStart; i < content.length(); ++i) {
        if (content[i] == ' ' || content[i] == '\t') {
            indent += content[i];
        } else {
            break;
        }
    }
    
    // Default to 2 spaces if no indentation found
    return indent.empty() ? "  " : indent;
}

std::string FileSyntaxDetector::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
