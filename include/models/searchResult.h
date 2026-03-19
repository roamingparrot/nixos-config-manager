#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include <string>

/**
 * @brief Represents a search result from nix search
 * 
 * This struct holds information about a package returned from
 * nix search nixpkgs <query> --json
 */
struct SearchResult {
    std::string attributePath;    // "nixpkgs#neovim"
    std::string packageName;      // "neovim"
    std::string version;          // "0.9.5"
    std::string description;      // "Hyperextensible Vim-based text editor"
    
    /**
     * @brief Constructor with default values
     */
    SearchResult() {}
    
    /**
     * @brief Constructor with all values
     */
    SearchResult(const std::string& attrPath, const std::string& name,
                const std::string& ver, const std::string& desc)
        : attributePath(attrPath), packageName(name), 
          version(ver), description(desc) {}
    
    /**
     * @brief Get the pkgs.* attribute name
     * @return Attribute name like "pkgs.neovim"
     */
    std::string getPkgsAttribute() const {
        // Convert "nixpkgs#neovim" to "pkgs.neovim"
        if (attributePath.find("nixpkgs#") == 0) {
            return "pkgs." + attributePath.substr(8);
        }
        return packageName;
    }
};

#endif // SEARCH_RESULT_H
