#include "configEditor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

ConfigEditor::ConfigEditor() {}

bool ConfigEditor::removePackage(const PackageEntry& entry) {
    std::vector<PackageEntry> packages = {entry};
    return removePackages(packages);
}

bool ConfigEditor::removePackages(const std::vector<PackageEntry>& packages) {
    // Group packages by file path
    std::map<std::string, std::vector<std::string>> packagesByFile;
    for (const PackageEntry& package : packages) {
        if (package.markedForDeletion) {
            packagesByFile[package.filePath].push_back(package.name);
        }
    }
    
    // Process each file
    bool success = true;
    for (const auto& pair : packagesByFile) {
        const std::string& filePath = pair.first;
        const std::vector<std::string>& packagesToRemove = pair.second;
        
        try {
            // Read file content
            std::string content = readFile(filePath);
            std::vector<std::string> lines;
            std::stringstream ss(content);
            std::string line;
            
            while (std::getline(ss, line, '\n')) {
                lines.push_back(line);
            }
            
            // Remove package lines by matching package names
            std::vector<std::string> newLines;
            for (const std::string& currentLine : lines) {
                bool shouldRemove = false;
                
                // Check if this line contains a package to remove
                for (const std::string& pkgName : packagesToRemove) {
                    // Match lines that are just the package name (with optional whitespace)
                    std::string trimmedLine = currentLine;
                    trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));
                    trimmedLine.erase(trimmedLine.find_last_not_of(" \t") + 1);
                    
                    if (trimmedLine == pkgName) {
                        shouldRemove = true;
                        std::cout << "  Removing: " << pkgName << std::endl;
                        break;
                    }
                }
                
                if (!shouldRemove) {
                    newLines.push_back(currentLine);
                }
            }
            
            // Reconstruct content
            std::stringstream newContent;
            for (size_t i = 0; i < newLines.size(); ++i) {
                newContent << newLines[i];
                if (i < newLines.size() - 1) {
                    newContent << "\n";
                }
            }
            
            // Write back to file
            writeFile(filePath, newContent.str());
        } catch (const std::exception& e) {
            std::cerr << "Error editing " << filePath << ": " << e.what() << std::endl;
            success = false;
        }
    }
    
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