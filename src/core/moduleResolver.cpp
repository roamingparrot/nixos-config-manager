#include "moduleResolver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <algorithm>

ModuleResolver::ModuleResolver() {
    visitedFiles.clear();
}

std::vector<std::string> ModuleResolver::resolveAllModules(const std::string& entryFile) {
    std::vector<std::string> allModules;
    visitedFiles.clear();
    
    // Start resolution from entry file
    std::vector<std::string> toProcess = {entryFile};
    
    while (!toProcess.empty()) {
        std::string currentFile = toProcess.back();
        toProcess.pop_back();
        
        // Skip if already visited
        if (visitedFiles.find(currentFile) != visitedFiles.end()) {
            continue;
        }
        
        // Mark as visited
        visitedFiles.insert(currentFile);
        allModules.push_back(currentFile);
        
        try {
            // Load module and parse imports
            ModuleInfo module = loadModule(currentFile);
            
            // Add imports to processing queue
            for (const std::string& importPath : module.imports) {
                if (visitedFiles.find(importPath) == visitedFiles.end()) {
                    toProcess.push_back(importPath);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not process " << currentFile << ": " << e.what() << std::endl;
        }
    }
    
    return allModules;
}

ModuleInfo ModuleResolver::loadModule(const std::string& filePath) {
    std::string content = readFile(filePath);
    ModuleInfo module(filePath, content);
    module.imports = parseImports(content);
    
    // Resolve import paths to absolute paths
    std::string baseDir = getDirectory(filePath);
    for (size_t i = 0; i < module.imports.size(); ++i) {
        module.imports[i] = resolvePath(baseDir, module.imports[i]);
    }
    
    return module;
}

std::vector<std::string> ModuleResolver::parseImports(const std::string& content) {
    std::vector<std::string> imports;
    
    // Find "imports = [" and then find the matching "]"
    size_t importsPos = content.find("imports");
    if (importsPos == std::string::npos) {
        return imports;
    }
    
    // Find the opening bracket
    size_t openBracket = content.find('[', importsPos);
    if (openBracket == std::string::npos) {
        return imports;
    }
    
    // Find the matching closing bracket
    int bracketCount = 1;
    size_t closeBracket = openBracket + 1;
    while (closeBracket < content.length() && bracketCount > 0) {
        if (content[closeBracket] == '[') {
            bracketCount++;
        } else if (content[closeBracket] == ']') {
            bracketCount--;
        }
        closeBracket++;
    }
    
    if (bracketCount != 0) {
        return imports; // Unmatched brackets
    }
    
    // Extract the content between brackets
    std::string importsBlock = content.substr(openBracket + 1, closeBracket - openBracket - 2);
    
    // Parse paths - look for lines starting with ./ or containing /
    std::stringstream ss(importsBlock);
    std::string line;
    while (std::getline(ss, line)) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) continue;
        
        size_t end = line.find_last_not_of(" \t\r");
        std::string path = line.substr(start, end - start + 1);
        
        // Remove trailing semicolon if present
        if (!path.empty() && path.back() == ';') {
            path.pop_back();
        }
        
        // Skip empty paths, comments, and nixpkgs paths (starting with <)
        if (path.empty() || path[0] == '#' || path[0] == '<') {
            continue;
        }
        
        // Accept paths that start with ./ or contain /
        if (path[0] == '.' || path.find('/') != std::string::npos) {
            imports.push_back(path);
        }
    }
    
    return imports;
}

std::string ModuleResolver::resolvePath(const std::string& base, const std::string& relative) {
    // If relative path starts with /, it's already absolute
    if (relative.empty() || relative[0] == '/') {
        return relative;
    }
    
    // Handle relative paths - base is already a directory
    std::filesystem::path basePath(base);
    std::filesystem::path relativePath(relative);
    std::filesystem::path resolved = basePath / relativePath;
    
    // Normalize path
    return std::filesystem::absolute(resolved).lexically_normal().string();
}

std::string ModuleResolver::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string ModuleResolver::getDirectory(const std::string& filePath) {
    std::filesystem::path path(filePath);
    return path.parent_path().string();
}