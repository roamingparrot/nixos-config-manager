#include "core/configEditor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <regex>

ConfigEditor::ConfigEditor() {}

bool ConfigEditor::removePackage(const PackageEntry& entry) {
    std::vector<PackageEntry> packages = {entry};
    return removePackages(packages);
}

bool ConfigEditor::removePackages(const std::vector<PackageEntry>& packages) {
    std::cerr << "\n=== ConfigEditor Debug ===" << std::endl;
    
    // Group packages by file path
    std::map<std::string, std::vector<std::string>> packagesByFile;
    for (const PackageEntry& package : packages) {
        if (package.markedForDeletion) {
            packagesByFile[package.filePath].push_back(package.name);
            std::cerr << "Marked for deletion: " << package.name 
                      << " in file: " << package.filePath << std::endl;
        }
    }
    
    if (packagesByFile.empty()) {
        std::cerr << "No packages marked for deletion" << std::endl;
        return true;
    }
    
    // Process each file
    bool success = true;
    for (const auto& pair : packagesByFile) {
        const std::string& filePath = pair.first;
        const std::vector<std::string>& packagesToRemove = pair.second;
        
        std::cerr << "\nProcessing file: " << filePath << std::endl;
        
        try {
            // Read file content
            std::string content = readFile(filePath);
            std::vector<std::string> lines;
            std::stringstream ss(content);
            std::string line;
            
            while (std::getline(ss, line, '\n')) {
                lines.push_back(line);
            }
            
            std::cerr << "File has " << lines.size() << " lines" << std::endl;
            
            // Remove package lines by matching package names
            std::vector<std::string> newLines;
            int removedCount = 0;
            
            for (size_t i = 0; i < lines.size(); ++i) {
                const std::string& currentLine = lines[i];
                bool shouldRemove = false;
                std::string matchedPkg;
                
                // Check if this line contains a package to remove
                for (const std::string& pkgName : packagesToRemove) {
                    // Trim whitespace
                    std::string trimmedLine = currentLine;
                    size_t first = trimmedLine.find_first_not_of(" \t");
                    if (first != std::string::npos) {
                        trimmedLine = trimmedLine.substr(first);
                    }
                    size_t last = trimmedLine.find_last_not_of(" \t");
                    if (last != std::string::npos && last < trimmedLine.length() - 1) {
                        trimmedLine = trimmedLine.substr(0, last + 1);
                    }
                    
                    // Check multiple patterns
                    std::vector<std::string> patterns = {
                        pkgName,                    // exact match (indented or not)
                        pkgName + ")",              // end of pkgs.packageName)
                        "\"" + pkgName + "\"",      // quoted package name
                        "'" + pkgName + "'",        // single-quoted
                    };
                    
                    for (const auto& pattern : patterns) {
                        if (trimmedLine == pattern || 
                            trimmedLine.find(pattern) != std::string::npos) {
                            shouldRemove = true;
                            matchedPkg = pkgName;
                            break;
                        }
                    }
                    
                    if (shouldRemove) break;
                }
                
                if (shouldRemove) {
                    std::cerr << "  REMOVING line " << (i + 1) << ": " << currentLine << std::endl;
                    removedCount++;
                } else {
                    newLines.push_back(currentLine);
                }
            }
            
            std::cerr << "Removed " << removedCount << " lines for " 
                      << packagesToRemove.size() << " packages" << std::endl;
            
            // Reconstruct content
            std::stringstream newContent;
            for (size_t i = 0; i < newLines.size(); ++i) {
                newContent << newLines[i];
                if (i < newLines.size() - 1) {
                    newContent << "\n";
                }
            }
            
            // Write back to file
            std::cerr << "Writing changes to: " << filePath << std::endl;
            writeFile(filePath, newContent.str());
            std::cerr << "Successfully wrote: " << filePath << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error editing " << filePath << ": " << e.what() << std::endl;
            success = false;
        }
    }
    
    std::cerr << "=== ConfigEditor Complete ===" << std::endl << std::endl;
    return success;
}

std::string ConfigEditor::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ConfigEditor::writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not write to file: " + filePath);
    }
    
    file << content;
}
