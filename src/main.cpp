#include <iostream>
#include "core/moduleResolver.h"
#include "core/configParser.h"
#include "core/configEditor.h"
#include "core/rebuildManager.h"
#include "core/packageSearcher.h"
#include "core/moduleSelector.h"
#include "core/packageInserter.h"
#include "ui/tui.h"

void handlePackageRemoval(const std::vector<PackageEntry>& packages) {
    bool hasMarkedPackages = false;
    for (const auto& pkg : packages) {
        if (pkg.markedForDeletion) {
            hasMarkedPackages = true;
            break;
        }
    }
    
    if (!hasMarkedPackages) {
        std::cout << "No packages marked for deletion." << std::endl;
        return;
    }
    
    std::cout << "Removing marked packages..." << std::endl;
    ConfigEditor editor;
    if (!editor.removePackages(packages)) {
        std::cerr << "Error: Failed to remove some packages." << std::endl;
        return;
    }
    
    std::cout << "Running nixos-rebuild switch..." << std::endl;
    RebuildManager rebuild;
    if (!rebuild.rebuild()) {
        std::cerr << "Error: Rebuild failed." << std::endl;
        std::cerr << rebuild.getOutput() << std::endl;
        return;
    }
    
    std::cout << "System rebuilt successfully!" << std::endl;
}

void handlePackageInstallation(const SearchResult& package, 
                               const InstallTarget& target) {
    std::cout << "Installing " << package.packageName << " to " 
              << target.fileName << "..." << std::endl;
    
    std::string pkgToInsert;
    if (target.usesWithPkgs) {
        pkgToInsert = package.packageName;
    } else {
        pkgToInsert = package.getPkgsAttribute();
    }
    
    PackageInserter inserter;
    if (!inserter.insertPackage(target, pkgToInsert)) {
        std::cerr << "Error: Failed to insert package." << std::endl;
        return;
    }
    
    std::cout << "Running nixos-rebuild switch..." << std::endl;
    RebuildManager rebuild;
    if (!rebuild.rebuild()) {
        std::cerr << "Error: Rebuild failed." << std::endl;
        std::cerr << rebuild.getOutput() << std::endl;
        return;
    }
    
    std::cout << "Package installed successfully!" << std::endl;
}

int main() {
    try {
        ModuleResolver resolver;
        std::vector<std::string> modules = 
            resolver.resolveAllModules("/etc/nixos/configuration.nix");
        
        ConfigParser parser;
        std::vector<PackageEntry> allPackages;
        
        for (const std::string& modulePath : modules) {
            try {
                ModuleInfo module = resolver.loadModule(modulePath);
                std::vector<PackageEntry> packages = 
                    parser.extractPackages(module.content, modulePath);
                allPackages.insert(allPackages.end(), packages.begin(), packages.end());
            } catch (const std::exception& e) {
                // Skip problematic modules
            }
        }
        
        if (allPackages.empty()) {
            std::cout << "No packages found in NixOS configuration." << std::endl;
        }
        
        TUI tui;
        tui.initialize(allPackages);
        bool shouldSave = tui.run();
        
        // Check if user wants to perform an action
        std::string query = tui.getSearchQuery();
        if (!query.empty()) {
            // User initiated a search
            std::cout << "Searching for: " << query << std::endl;
            PackageSearcher searcher;
            std::vector<SearchResult> results = searcher.search(query);
            
            if (results.empty()) {
                std::cout << "No packages found." << std::endl;
                return 0;
            }
            
            tui.setSearchResults(results);
            tui.run();
            
            SearchResult selected = tui.getSelectedSearchResult();
            if (!selected.packageName.empty()) {
                // User selected a package, now choose module
                ModuleSelector selector;
                std::vector<InstallTarget> targets = selector.discoverTargets(modules);
                
                if (targets.empty()) {
                    std::cerr << "No installation targets found." << std::endl;
                    return 1;
                }
                
                tui.setInstallTargets(targets);
                tui.run();
                
                InstallTarget target = tui.getSelectedTarget();
                if (!target.filePath.empty()) {
                    handlePackageInstallation(selected, target);
                }
            }
        } else if (shouldSave) {
            // User wants to save deletions
            handlePackageRemoval(tui.getPackages());
        } else {
            std::cout << "Exited without changes." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
