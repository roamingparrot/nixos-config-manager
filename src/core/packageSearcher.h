#ifndef PACKAGE_SEARCHER_H
#define PACKAGE_SEARCHER_H

#include <string>
#include <vector>
#include "../models/searchResult.h"

/**
 * @brief Searches for packages using nix search
 * 
 * This class executes 'nix search nixpkgs <query> --json'
 * and parses the results.
 */
class PackageSearcher {
private:
    std::vector<SearchResult> lastResults;
    std::string lastQuery;
    
    /**
     * @brief Parse JSON output from nix search
     * @param json JSON string from nix search
     * @return Vector of search results
     */
    std::vector<SearchResult> parseJsonOutput(const std::string& json);
    
    /**
     * @brief Execute nix search command
     * @param query Search term
     * @return JSON output as string
     */
    std::string executeSearch(const std::string& query);
    
    /**
     * @brief Extract value from JSON between quotes
     * @param json JSON string
     * @param key Key to find
     * @param startPos Position to start searching from
     * @return Value string (empty if not found)
     */
    std::string extractJsonValue(const std::string& json, 
                                 const std::string& key, 
                                 size_t startPos);

public:
    /**
     * @brief Constructor
     */
    PackageSearcher();
    
    /**
     * @brief Search for packages
     * @param query Search term
     * @return Vector of matching packages
     */
    std::vector<SearchResult> search(const std::string& query);
    
    /**
     * @brief Get last search results
     * @return Vector of search results
     */
    std::vector<SearchResult> getResults() const;
    
    /**
     * @brief Get last query
     * @return Last search query
     */
    std::string getLastQuery() const;
};

#endif // PACKAGE_SEARCHER_H
