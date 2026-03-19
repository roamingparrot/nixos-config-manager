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
    // Build command
    std::string cmd = "nix search nixpkgs " + query + " --json 2>&1";
    
    // Execute command
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute nix search");
    }
    
    // Read output
    char buffer[1024];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        throw std::runtime_error("nix search command failed");
    }
    
    return result;
}

std::vector<SearchResult> PackageSearcher::parseJsonOutput(const std::string& json) {
    std::vector<SearchResult> results;
    
    // Find all "nixpkgs#<attr>" patterns
    size_t pos = 0;
    while ((pos = json.find("\"nixpkgs#", pos)) != std::string::npos) {
        // Extract attribute path
        size_t attrStart = pos + 1;  // Skip opening quote
        size_t attrEnd = json.find("\"", attrStart);
        if (attrEnd == std::string::npos) break;
        
        std::string attrPath = json.substr(attrStart, attrEnd - attrStart);
        
        // Extract package name from attribute path
        std::string pkgName = attrPath.substr(8);  // Remove "nixpkgs#"
        
        // Find the value block for this attribute
        size_t blockStart = json.find("{", attrEnd);
        if (blockStart == std::string::npos) break;
        
        // Extract version
        std::string version = extractJsonValue(json, "version", blockStart);
        
        // Extract description
        std::string description = extractJsonValue(json, "description", blockStart);
        
        // Create search result
        results.emplace_back(attrPath, pkgName, version, description);
        
        pos = attrEnd + 1;
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
