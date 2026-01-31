/**
 * @file main.cpp
 * @brief IPFS Ring DHT Simulator - Main Entry Point
 * @author Data Structures Project
 * @date 2024
 * 
 * @details This program simulates an IPFS-style Ring-based Distributed Hash Table (DHT).
 *          It supports:
 *          - Configurable identifier space (1-31 bits)
 *          - Dynamic machine join/leave operations
 *          - File storage using B-trees
 *          - O(log N) routing using finger tables
 *          - File operations from ANY machine
 * 
 * Compile: g++ -std=c++17 -Wall -Wextra -O2 -o ipfs_dht src/main.cpp
 */

#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <vector>
#include <algorithm>

#include "IPFS.h"
#include "SHA1.h"
#include "Menu.h"

using namespace std;

// Global variables
IPFS* ipfs = nullptr;
int btreeOrder = 5;

/**
 * @brief Clean up resources
 */
void cleanup() {
    if (ipfs != nullptr) {
        delete ipfs;
        ipfs = nullptr;
    }
}

/**
 * @brief Initialize the DHT system
 */
bool initializeSystem() {
    printSetupHeader();
    
    // Get identifier space bits
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |  STEP 1: Configure Identifier Space                                    |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    cout << "  The identifier space determines the range of possible IDs (0 to 2^bits - 1).\n";
    cout << "  Examples: 4 bits = 0-15, 8 bits = 0-255, 16 bits = 0-65535\n\n";
    
    int bits = getIntInput("Enter number of bits (1-31): ", 1, 31);
    
    int identifierSpace = 1 << bits;
    cout << "\n  Identifier Space: " << bits << " bits\n";
    cout << "  ID Range: 0 to " << (identifierSpace - 1) << "\n";
    cout << "  Total possible IDs: " << identifierSpace << "\n";
    
    // Get B-tree order
    cout << "\n  +------------------------------------------------------------------------+\n";
    cout << "  |  STEP 2: Configure B-Tree Order                                        |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    cout << "  The B-tree order determines how many keys each node can hold.\n";
    cout << "  Recommended: 3-7 for small systems, higher for large systems.\n\n";
    
    btreeOrder = getIntInput("Enter B-tree order (3-100): ", 3, 100);
    
    // Create IPFS instance
    cleanup();
    ipfs = new IPFS(bits, btreeOrder);
    
    // Get number of machines
    cout << "\n  +------------------------------------------------------------------------+\n";
    cout << "  |  STEP 3: Add Initial Machines                                          |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    int numMachines = getIntInput("Enter number of machines to add (1-" + to_string(min(identifierSpace, 1000)) + "): ", 
                                   1, min(identifierSpace, 1000));
    
    // Choose manual or automatic ID assignment
    cout << "\n  How would you like to assign machine IDs?\n";
    cout << "  1. Manual - Enter each ID yourself\n";
    cout << "  2. Automatic - Use hash function on machine names\n";
    cout << "  3. Random - Generate random unique IDs\n\n";
    
    int assignChoice = getIntInput("Enter choice (1-3): ", 1, 3);
    
    vector<int> machineIds;
    
    if (assignChoice == 1) {
        // Manual assignment
        cout << "\n  Enter machine IDs (must be in range 0-" << (identifierSpace - 1) << "):\n\n";
        
        for (int i = 0; i < numMachines; i++) {
            int id;
            bool valid = false;
            
            while (!valid) {
                id = getIntInput("  Machine " + to_string(i + 1) + " ID: ", 0, identifierSpace - 1);
                
                // Check for duplicates
                if (find(machineIds.begin(), machineIds.end(), id) != machineIds.end()) {
                    cout << "  Duplicate ID! Please enter a unique ID.\n";
                } else {
                    valid = true;
                }
            }
            machineIds.push_back(id);
        }
    } 
    else if (assignChoice == 2) {
        // Hash-based assignment
        cout << "\n  Enter machine names (IDs will be generated using hash function):\n\n";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        for (int i = 0; i < numMachines; i++) {
            string name;
            bool valid = false;
            
            while (!valid) {
                cout << "  Machine " << (i + 1) << " name: ";
                getline(cin, name);
                
                if (name.empty()) {
                    cout << "  Please enter a non-empty name.\n";
                    continue;
                }
                
                int id = generateHashInSpace(name, identifierSpace);
                
                // Handle collision
                int originalId = id;
                while (find(machineIds.begin(), machineIds.end(), id) != machineIds.end()) {
                    id = (id + 1) % identifierSpace;
                    if (id == originalId) {
                        cout << "  No more unique IDs available!\n";
                        break;
                    }
                }
                
                cout << "  -> Generated ID: " << id << " (from hash of \"" << name << "\")\n";
                machineIds.push_back(id);
                valid = true;
            }
        }
    } 
    else {
        // Random assignment
        cout << "\n  Generating random unique IDs...\n\n";
        srand(static_cast<unsigned>(time(nullptr)));
        
        for (int i = 0; i < numMachines; i++) {
            int id;
            int attempts = 0;
            const int maxAttempts = identifierSpace * 2;
            
            do {
                id = rand() % identifierSpace;
                attempts++;
            } while (find(machineIds.begin(), machineIds.end(), id) != machineIds.end() 
                     && attempts < maxAttempts);
            
            if (attempts >= maxAttempts) {
                cout << "  Could not generate unique ID for machine " << (i + 1) << "\n";
                continue;
            }
            
            cout << "  Machine " << (i + 1) << " -> ID: " << id << "\n";
            machineIds.push_back(id);
        }
    }
    
    // Insert machines
    if (!machineIds.empty()) {
        int* arr = new int[machineIds.size()];
        for (size_t i = 0; i < machineIds.size(); i++) {
            arr[i] = machineIds[i];
        }
        
        ipfs->InsertMachines(arr, static_cast<int>(machineIds.size()));
        delete[] arr;
    }
    
    cout << "\n  System initialized successfully!\n";
    printSeparator();
    ipfs->printDetailedStatus();
    
    waitForEnter();
    return true;
}

/**
 * @brief Add a new machine
 */
void addMachine() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                         ADD NEW MACHINE                                |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    cout << "\n  How would you like to assign the ID?\n";
    cout << "  1. Manual - Enter ID yourself\n";
    cout << "  2. Automatic - Use hash of machine name\n\n";
    
    int choice = getIntInput("Enter choice (1-2): ", 1, 2);
    
    int machineId;
    
    if (choice == 1) {
        machineId = getIntInput("Enter machine ID (0-" + to_string(ipfs->getMaxId()) + "): ", 
                                 0, ipfs->getMaxId());
    } else {
        string name;
        cout << "  Enter machine name: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, name);
        
        machineId = generateHashInSpace(name, ipfs->getIdentifierSpace());
        cout << "  Generated ID: " << machineId << " (from hash of \"" << name << "\")\n";
    }
    
    ipfs->InsertMachine(machineId, btreeOrder);
    
    cout << "\n  Updated Ring:\n";
    ipfs->printRing();
    
    if (getConfirmation("View routing table of new machine?")) {
        ipfs->PrintRT(machineId);
    }
    
    waitForEnter();
}

/**
 * @brief Remove a machine
 */
void removeMachine() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                         REMOVE MACHINE                                 |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    if (ipfs->getMachineCount() == 0) {
        printError("Ring is empty! Add machines first.");
        waitForEnter();
        return;
    }
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    int machineId = getIntInput("\n  Enter machine ID to remove: ", 0, ipfs->getMaxId());
    
    if (!ipfs->machineExists(machineId)) {
        printError("Machine " + to_string(machineId) + " does not exist!");
        waitForEnter();
        return;
    }
    
    if (getConfirmation("Are you sure you want to remove Machine " + to_string(machineId) + "?")) {
        ipfs->DeleteMachine(machineId, btreeOrder);
        
        cout << "\n  Updated Ring:\n";
        ipfs->printRing();
    }
    
    waitForEnter();
}

/**
 * @brief Insert a file
 */
void insertFile() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                           INSERT FILE                                  |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    if (ipfs->getMachineCount() == 0) {
        printError("Ring is empty! Add machines first.");
        waitForEnter();
        return;
    }
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    int startMachine = getIntInput("\n  Enter starting machine ID: ", 0, ipfs->getMaxId());
    
    if (!ipfs->machineExists(startMachine)) {
        printError("Machine " + to_string(startMachine) + " does not exist!");
        waitForEnter();
        return;
    }
    
    int numFiles = getIntInput("  Enter number of files to insert (1-100): ", 1, 100);
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    for (int i = 0; i < numFiles; i++) {
        cout << "\n  File " << (i + 1) << " of " << numFiles << ":\n";
        
        string filePath;
        cout << "  Enter file path: ";
        getline(cin, filePath);
        
        if (filePath.empty()) {
            cout << "  Skipping empty path.\n";
            continue;
        }
        
        int fileKey = generateHashInSpace(filePath, ipfs->getIdentifierSpace());
        cout << "  File hash key: " << fileKey << "\n";
        
        ipfs->insertFile(startMachine, fileKey, filePath, btreeOrder);
    }
    
    waitForEnter();
}

/**
 * @brief Search for a file
 */
void searchFile() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                           SEARCH FILE                                  |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    if (ipfs->getMachineCount() == 0) {
        printError("Ring is empty! Add machines first.");
        waitForEnter();
        return;
    }
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    int startMachine = getIntInput("\n  Enter starting machine ID: ", 0, ipfs->getMaxId());
    
    if (!ipfs->machineExists(startMachine)) {
        printError("Machine " + to_string(startMachine) + " does not exist!");
        waitForEnter();
        return;
    }
    
    string filePath;
    cout << "  Enter file path to search: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, filePath);
    
    int fileKey = generateHashInSpace(filePath, ipfs->getIdentifierSpace());
    cout << "  File hash key: " << fileKey << "\n";
    
    ipfs->SearchFile(startMachine, fileKey);
    
    waitForEnter();
}

/**
 * @brief Delete a file
 */
void deleteFile() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                           DELETE FILE                                  |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    if (ipfs->getMachineCount() == 0) {
        printError("Ring is empty! Add machines first.");
        waitForEnter();
        return;
    }
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    int startMachine = getIntInput("\n  Enter starting machine ID: ", 0, ipfs->getMaxId());
    
    if (!ipfs->machineExists(startMachine)) {
        printError("Machine " + to_string(startMachine) + " does not exist!");
        waitForEnter();
        return;
    }
    
    string filePath;
    cout << "  Enter file path to delete: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, filePath);
    
    int fileKey = generateHashInSpace(filePath, ipfs->getIdentifierSpace());
    cout << "  File hash key: " << fileKey << "\n";
    
    ipfs->DeleteFile(startMachine, fileKey);
    
    waitForEnter();
}

/**
 * @brief Print routing table
 */
void printRoutingTable() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                        PRINT ROUTING TABLE                             |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    if (ipfs->getMachineCount() == 0) {
        printError("Ring is empty! Add machines first.");
        waitForEnter();
        return;
    }
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    int machineId = getIntInput("\n  Enter machine ID: ", 0, ipfs->getMaxId());
    
    if (!ipfs->machineExists(machineId)) {
        printError("Machine " + to_string(machineId) + " does not exist!");
        waitForEnter();
        return;
    }
    
    ipfs->PrintRT(machineId);
    
    waitForEnter();
}

/**
 * @brief Print B-tree
 */
void printBTree() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                           PRINT B-TREE                                 |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    if (ipfs->getMachineCount() == 0) {
        printError("Ring is empty! Add machines first.");
        waitForEnter();
        return;
    }
    
    cout << "  Current Ring:\n";
    ipfs->printRing();
    
    int machineId = getIntInput("\n  Enter machine ID: ", 0, ipfs->getMaxId());
    
    if (!ipfs->machineExists(machineId)) {
        printError("Machine " + to_string(machineId) + " does not exist!");
        waitForEnter();
        return;
    }
    
    ipfs->printBTree(machineId);
    
    waitForEnter();
}

/**
 * @brief Print all routing tables
 */
void printAllRoutingTables() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                      ALL ROUTING TABLES                                |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    ipfs->printAllRoutingTables();
    
    waitForEnter();
}

/**
 * @brief Print all B-trees
 */
void printAllBTrees() {
    clearScreen();
    printHeader();
    cout << "  +------------------------------------------------------------------------+\n";
    cout << "  |                         ALL B-TREES                                    |\n";
    cout << "  +------------------------------------------------------------------------+\n\n";
    
    ipfs->printAllBTrees();
    
    waitForEnter();
}

/**
 * @brief View system status
 */
void viewStatus() {
    clearScreen();
    printHeader();
    ipfs->printDetailedStatus();
    waitForEnter();
}

/**
 * @brief Main entry point
 */
int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    bool running = true;
    bool initialized = false;
    
    while (running) {
        if (!initialized) {
            initialized = initializeSystem();
            if (!initialized) {
                cout << "Failed to initialize system. Exiting.\n";
                break;
            }
        }
        
        clearScreen();
        printHeader();
        printMainMenu();
        
        int choice = getIntInput("  Enter your choice (0-11): ", 0, 11);
        
        switch (choice) {
            case 0:  // Exit
                if (getConfirmation("Are you sure you want to exit?")) {
                    running = false;
                    cout << "\n  Thank you for using IPFS Ring DHT Simulator!\n";
                    cout << "  Goodbye!\n\n";
                }
                break;
                
            case 1:  // Add machine
                addMachine();
                break;
                
            case 2:  // Remove machine
                removeMachine();
                break;
                
            case 3:  // Insert file
                insertFile();
                break;
                
            case 4:  // Search file
                searchFile();
                break;
                
            case 5:  // Delete file
                deleteFile();
                break;
                
            case 6:  // Print routing table
                printRoutingTable();
                break;
                
            case 7:  // Print B-tree
                printBTree();
                break;
                
            case 8:  // Print all routing tables
                printAllRoutingTables();
                break;
                
            case 9:  // Print all B-trees
                printAllBTrees();
                break;
                
            case 10:  // View status
                viewStatus();
                break;
                
            case 11:  // Restart
                if (getConfirmation("Restart system? All data will be lost.")) {
                    cleanup();
                    initialized = false;
                }
                break;
                
            default:
                printError("Invalid choice!");
                waitForEnter();
        }
    }
    
    cleanup();
    return 0;
}
