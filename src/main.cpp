#include <iostream>
#include "core/moduleResolver.h"
#include "core/configParser.h"
#include "core/configEditor.h"
#include "core/rebuildManager.h"
#include "ui/tui.h"

int main() {
    try {
        // Resolve all modules
        ModuleResolver resolver;
        std::vector<std::string> modules = resolver.resolveAllModules("/etc/nixos/configuration.nix");
        
        std::cout << "Found " << modules.size() << " modules:" << std::endl;
        for (const auto& mod : modules) {
            std::cout << "  - " << mod << std::endl;
        }
        std::cout << std::endl;
        
        // Parse packages from all modules
        ConfigParser parser;
        std::vector<PackageEntry> allPackages;
        
        for (const std::string& modulePath : modules) {
            try {
                ModuleInfo module = resolver.loadModule(modulePath);
                std::vector<PackageEntry> packages = parser.extractPackages(module.content, modulePath);
                std::cout << "Module " << modulePath << ": found " << packages.size() << " packages" << std::endl;
                allPackages.insert(allPackages.end(), packages.begin(), packages.end());
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not parse " << modulePath << ": " << e.what() << std::endl;
            }
        }
        std::cout << std::endl;
        
        if (allPackages.empty()) {
            std::cout << "No packages found in NixOS configuration." << std::endl;
            return 0;
        }
        
        // Run TUI
        TUI tui;
        tui.initialize(allPackages);
        bool shouldSave = tui.run();
        
        if (shouldSave) {
            // Get modified packages
            std::vector<PackageEntry> modifiedPackages = tui.getPackages();
            
            // Check if any packages are marked for deletion
            bool hasMarkedPackages = false;
            for (const auto& pkg : modifiedPackages) {
                if (pkg.markedForDeletion) {
                    hasMarkedPackages = true;
                    break;
                }
            }
            
            if (!hasMarkedPackages) {
                std::cout << "No packages marked for deletion." << std::endl;
                return 0;
            }
            
            // Remove marked packages
            std::cout << "Removing marked packages..." << std::endl;
            ConfigEditor editor;
            if (!editor.removePackages(modifiedPackages)) {
                std::cerr << "Error: Failed to remove some packages." << std::endl;
                return 1;
            }
            
            // Rebuild system
            std::cout << "Running nixos-rebuild switch..." << std::endl;
            RebuildManager rebuild;
            if (!rebuild.rebuild()) {
                std::cerr << "Error: Rebuild failed." << std::endl;
                std::cerr << rebuild.getOutput() << std::endl;
                return 1;
            }
            
            std::cout << "System rebuilt successfully!" << std::endl;
        } else {
            std::cout << "Exited without saving." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}