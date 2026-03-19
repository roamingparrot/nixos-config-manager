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
    std::map<std::string, std::vector<const PackageEntry*>> packagesByFile;
    for (const PackageEntry& package : packages) {
        if (package.markedForDeletion) {
            packagesByFile[package.filePath].push_back(&package);
        }
    }
    
    // Process each file
    bool success = true;
    for (const auto& pair : packagesByFile) {
        const std::string& filePath = pair.first;
        const std::vector<const PackageEntry*>& filePackages = pair.second;
        
        try {
            // Read file content
            std::string content = readFile(filePath);
            
            // Sort packages by line number in descending order for safe removal
            std::vector<const PackageEntry*> sortedPackages = filePackages;
            std::sort(sortedPackages.begin(), sortedPackages.end(), 
                     [](const PackageEntry* a, const PackageEntry* b) {
                         return a->startLine > b->startLine;
                     });
            
            // Remove each package line by line
            std::vector<std::string> lines = {};
            std::stringstream ss(content);
            std::string line;
            int lineNumber = 1;
            
            while (std::getline(ss, line, '\n')) {
                lines.push_back(line);
                lineNumber++;
            }
            
            // Remove package lines (in reverse order to maintain line numbers)
            for (const PackageEntry* package : sortedPackages) {
                if (package->startLine > 0 && package->startLine <= (int)lines.size()) {
                    // Remove the line
                    lines.erase(lines.begin() + package->startLine - 1);
                }
            }
            
            // Reconstruct content
            std::stringstream newContent;
            for (size_t i = 0; i < lines.size(); ++i) {
                newContent << lines[i];
                if (i < lines.size() - 1) {
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