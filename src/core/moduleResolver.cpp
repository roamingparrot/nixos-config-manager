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
    
    // Regex to match imports = [ ... ] (non-greedy to handle multiline)
    std::regex importsRegex(R"(imports\s*=\s*\[(.*?)\])", 
                           std::regex_constants::ECMAScript);
    std::smatch match;
    
    if (std::regex_search(content, match, importsRegex)) {
        std::string importsBlock = match[1].str();
        
        // Regex to match paths in quotes or plain paths
        std::regex pathRegex(R"((?:\"|\s|^)(\.?\.?/[^\";\s]+|[^\"/\s][^\";\s]*/[^\";\s]+))");
        std::sregex_iterator iter(importsBlock.begin(), importsBlock.end(), pathRegex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::string path = iter->str(1);
            
            // Clean up the path
            size_t start = path.find_first_not_of(" \t\n\r\"");
            size_t end_pos = path.find_last_not_of(" \t\n\r\"");
            if (start != std::string::npos && end_pos != std::string::npos) {
                path = path.substr(start, end_pos - start + 1);
            }
            
            // Only add non-empty paths that don't start with <
            if (!path.empty() && path[0] != '<' && path.find('/') != std::string::npos) {
                imports.push_back(path);
            }
        }
    }
    
    return imports;
}

std::string ModuleResolver::resolvePath(const std::string& base, const std::string& relative) {
    // If relative path starts with /, it's already absolute
    if (relative.empty() || relative[0] == '/') {
        return relative;
    }
    
    // Handle relative paths
    std::filesystem::path basePath(base);
    std::filesystem::path relativePath(relative);
    std::filesystem::path resolved = basePath.parent_path() / relativePath;
    
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