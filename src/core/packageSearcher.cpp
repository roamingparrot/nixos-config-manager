#include "core/packageSearcher.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <memory>

PackageSearcher::PackageSearcher() {}

std::vector<SearchResult> PackageSearcher::search(const std::string& query) {
    lastQuery = query;
    lastResults.clear();
    
    if (query.empty()) {
        return lastResults;
    }
    
    try {
        std::string json = executeSearch(query);
        lastResults = parseJsonOutput(json);
    } catch (const std::exception& e) {
        std::cerr << "Search error: " << e.what() << std::endl;
    }
    
    return lastResults;
}

std::string PackageSearcher::executeSearch(const std::string& query) {
    // Use nix-env -qaP which works on all NixOS systems
    std::string cmd = "nix-env -qaP " + query + " 2>/dev/null";
    
    // Execute command
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute nix-env");
    }
    
    // Read output
    char buffer[4096];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    
    return result;
}

std::vector<SearchResult> PackageSearcher::parseJsonOutput(const std::string& output) {
    std::vector<SearchResult> results;
    
    // nix-env -qaP output format:
    // nixos.package-name  package-name-version
    // Example: nixos.haskellPackages.youtube  youtube-0.2.1.1
    
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        // Split by whitespace
        size_t spacePos = line.find_first_of(" \t");
        if (spacePos == std::string::npos) continue;
        
        // Extract attribute path (e.g., "nixos.youtube-dl")
        std::string attrPath = line.substr(0, spacePos);
        
        // Extract package with version (e.g., "youtube-dl-2021.12.17")
        size_t valueStart = line.find_first_not_of(" \t", spacePos);
        if (valueStart == std::string::npos) continue;
        std::string pkgWithVersion = line.substr(valueStart);
        
        // Parse package name and version
        // Find last dash to separate version
        size_t lastDash = pkgWithVersion.find_last_of('-');
        std::string pkgName = pkgWithVersion;
        std::string version = "";
        
        if (lastDash != std::string::npos) {
            // Check if what follows is a version (starts with digit)
            if (lastDash + 1 < pkgWithVersion.length() && 
                isdigit(pkgWithVersion[lastDash + 1])) {
                pkgName = pkgWithVersion.substr(0, lastDash);
                version = pkgWithVersion.substr(lastDash + 1);
            }
        }
        
        // Extract just the package name from attribute path
        // nixos.youtube-dl -> youtube-dl
        size_t lastDot = attrPath.find_last_of('.');
        std::string shortName = (lastDot != std::string::npos) ? 
                                 attrPath.substr(lastDot + 1) : attrPath;
        
        // Create nixpkgs# format
        std::string nixpkgsPath = "nixpkgs#" + shortName;
        
        // Create search result
        results.emplace_back(nixpkgsPath, pkgName, version, "");
        
        // Limit results
        if (results.size() >= 50) {
            break;
        }
    }
    
    return results;
}

std::string PackageSearcher::extractJsonValue(const std::string& json, 
                                             const std::string& key, 
                                             size_t startPos) {
    // Find key
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey, startPos);
    if (keyPos == std::string::npos) {
        return "";
    }
    
    // Find colon after key
    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) {
        return "";
    }
    
    // Find opening quote
    size_t quoteStart = json.find("\"", colonPos);
    if (quoteStart == std::string::npos) {
        return "";
    }
    
    // Find closing quote
    size_t quoteEnd = quoteStart + 1;
    while (quoteEnd < json.length()) {
        if (json[quoteEnd] == '"' && json[quoteEnd - 1] != '\\') {
            break;
        }
        quoteEnd++;
    }
    
    if (quoteEnd >= json.length()) {
        return "";
    }
    
    return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
}

std::vector<SearchResult> PackageSearcher::getResults() const {
    return lastResults;
}

std::string PackageSearcher::getLastQuery() const {
    return lastQuery;
}
