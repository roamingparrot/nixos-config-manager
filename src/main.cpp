#include <iostream>
#include "core/moduleResolver.h"
#include "core/configParser.h"
#include "core/moduleSelector.h"
#include "ui/tui.h"

int main() {
    try {
        // Resolve all imported modules
        ModuleResolver resolver;
        std::vector<std::string> modules =
            resolver.resolveAllModules("/etc/nixos/configuration.nix");

        // Extract installed packages from all modules
        ConfigParser parser;
        std::vector<PackageEntry> allPackages;
        for (const std::string& path : modules) {
            try {
                ModuleInfo mod = resolver.loadModule(path);
                std::vector<PackageEntry> pkgs =
                    parser.extractPackages(mod.content, path);
                allPackages.insert(allPackages.end(), pkgs.begin(), pkgs.end());
            } catch (...) {
                // Skip unreadable modules
            }
        }

        // Discover which modules have environment.systemPackages (install targets)
        ModuleSelector selector;
        std::vector<InstallTarget> targets = selector.discoverTargets(modules);

        // Hand everything to the TUI and let it own the rest
        TUI tui;
        tui.initialize(allPackages, targets, "/etc/nixos/configuration.nix");
        tui.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
