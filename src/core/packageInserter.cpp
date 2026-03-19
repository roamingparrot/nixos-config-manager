#include "packageInserter.h"
#include <fstream>
#include <sstream>
#include <iostream>

PackageInserter::PackageInserter() {}

bool PackageInserter::insertPackage(const InstallTarget& target, 
                                   const std::string& packageName) {
    try {
        // Read file content
        std::string content = readFile(target.filePath);
        
        // Find environment.systemPackages
        size_t sysPackagesPos = content.find("environment.systemPackages");
        if (sysPackagesPos == std::string::npos) {
            std::cerr << "No environment.systemPackages found in " << target.filePath << std::endl;
            return false;
        }
        
        // Find the opening bracket
        size_t bracketPos = content.find('[', sysPackagesPos);
        if (bracketPos == std::string::npos) {
            std::cerr << "No opening bracket found in " << target.filePath << std::endl;
            return false;
        }
        
        // Find the position right after the opening bracket
        size_t insertPos = bracketPos + 1;
        
        // Skip whitespace to find where packages start
        while (insertPos < content.length() && 
               (content[insertPos] == ' ' || content[insertPos] == '\t' || 
                content[insertPos] == '\n' || content[insertPos] == '\r')) {
            insertPos++;
        }
        
        // Check if list is empty (next char is ])
        bool isEmpty = (content[insertPos] == ']');
        
        // Build the package string to insert
        std::string toInsert;
        if (isEmpty) {
            // Empty list: insert "  packageName\n  "
            toInsert = " " + packageName + " ";
        } else {
            // Non-empty list: insert "\n  packageName" before first package
            // Go back to after the bracket
            insertPos = bracketPos + 1;
            toInsert = "\n" + target.indentation + packageName;
        }
        
        // Insert the package
        content.insert(insertPos, toInsert);
        
        // Write back to file
        writeFile(target.filePath, content);
        
        std::cout << "  Inserted " << packageName << " into " << target.fileName << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error inserting package: " << e.what() << std::endl;
        return false;
    }
}

int PackageInserter::findInsertPosition(const std::string& content, 
                                       const InstallTarget& target) {
    (void)target;  // Unused for now
    
    // Find environment.systemPackages
    size_t sysPackagesPos = content.find("environment.systemPackages");
    if (sysPackagesPos == std::string::npos) {
        return -1;
    }
    
    // Find the opening bracket
    size_t bracketPos = content.find('[', sysPackagesPos);
    if (bracketPos == std::string::npos) {
        return -1;
    }
    
    return static_cast<int>(bracketPos + 1);
}

std::string PackageInserter::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void PackageInserter::writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not write to file: " + filePath);
    }
    
    file << content;
}
