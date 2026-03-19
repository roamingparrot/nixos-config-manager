#include <iostream>
#include "core/moduleResolver.h"
#include "core/configParser.h"
#include "ui/tui.h"

int main() {
    std::cout << "NixOS Configuration Manager" << std::endl;
    
    // For testing, create some sample packages
    std::vector<PackageEntry> testPackages;
    testPackages.emplace_back("git", "/etc/nixos/packages.nix", 10, 10);
    testPackages.emplace_back("firefox", "/etc/nixos/configuration.nix", 15, 15);
    testPackages.emplace_back("neovim", "/etc/nixos/dev/tools.nix", 5, 5);
    
    // Test TUI
    TUI tui;
    tui.initialize(testPackages);
    tui.run();
    
    return 0;
}