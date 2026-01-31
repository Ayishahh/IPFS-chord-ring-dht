/**
 * @file CircularLL.h
 * @brief Circular Linked List implementation for the Ring DHT
 * @details Implements the ring of machines with routing tables and file storage
 * 
 * Compile: g++ -std=c++17 -Wall -o ipfs_dht src/*.cpp
 */

#pragma once
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <cmath>
#include "Queue.h"
#include "DoublyLL.h"
#include "BTree.h"

using namespace std;

/**
 * @brief Machine node in the Ring DHT
 */
class CircularNode {
public:
    int key;                              // Machine ID
    CircularNode* next;                   // Next machine in ring
    DoublyLinkedList<CircularNode> RT;    // Routing Table (Finger Table)
    BTree BTreeroot;                      // B-Tree for file storage

    CircularNode() : key(-1), next(nullptr) {
        RT.setHead(nullptr);
        RT.setTail(nullptr);
    }
    
    CircularNode(int v) : key(v), next(nullptr) {
        RT.setHead(nullptr);
        RT.setTail(nullptr);
    }
    
    CircularNode(int v, int btreeOrder) : key(v), next(nullptr), BTreeroot(btreeOrder) {
        RT.setHead(nullptr);
        RT.setTail(nullptr);
    }
};

// Forward declarations
void Traverse_delete(CircularNode* source, CircularNode* destination, int order);
void Traverse_insert(CircularNode* previous, int order, int identifierSpace);

/**
 * @brief Transfer all files from source to destination machine
 */
void Traverse_delete(CircularNode* source, CircularNode* destination, int order) {
    if (source->BTreeroot.root == nullptr) return;
    
    vector<FileNode> files = source->BTreeroot.getAllFiles(source->BTreeroot.root);
    
    cout << "\n  Transferring " << files.size() << " file(s) from Machine " 
         << source->key << " to Machine " << destination->key << ":\n";
    
    for (const FileNode& file : files) {
        destination->BTreeroot.insertHelper(file, order);
        cout << "    - File " << file.key << " (" << file.path << ") transferred\n";
    }
}

/**
 * @brief Redistribute files to newly inserted machine
 */
void Traverse_insert(CircularNode* previous, int order, int /*identifierSpace*/) {
    CircularNode* newMachine = previous->next;
    CircularNode* successor = newMachine->next;
    
    if (successor->BTreeroot.root == nullptr) return;
    
    vector<FileNode> files = successor->BTreeroot.getAllFiles(successor->BTreeroot.root);
    vector<FileNode> toMove;
    
    // Find files that should move to new machine
    for (const FileNode& file : files) {
        // File belongs to new machine if: previous->key < file.key <= newMachine->key
        // Handle wrap-around
        bool shouldMove = false;
        if (previous->key < newMachine->key) {
            shouldMove = (file.key > previous->key && file.key <= newMachine->key);
        } else {
            // Wrap around case
            shouldMove = (file.key > previous->key || file.key <= newMachine->key);
        }
        
        if (shouldMove) {
            toMove.push_back(file);
        }
    }
    
    if (!toMove.empty()) {
        cout << "\n  Redistributing " << toMove.size() << " file(s) to new Machine " 
             << newMachine->key << ":\n";
        
        for (const FileNode& file : toMove) {
            newMachine->BTreeroot.insertHelper(file, order);
            successor->BTreeroot.deleteHelper(file.key);
            cout << "    - File " << file.key << " (" << file.path << ") moved from Machine " 
                 << successor->key << "\n";
        }
    }
}

/**
 * @brief Circular Linked List (Ring) for DHT
 */
class CircularLinkedList {
public:
    CircularNode* head;
    int identifierSpace;  // 2^bits (total number of possible IDs)
    int bits;             // Number of bits in identifier space
    int btreeOrder;       // B-tree order for file storage

    CircularLinkedList() : head(nullptr), identifierSpace(16), bits(4), btreeOrder(5) {}
    
    CircularLinkedList(int IS, int order = 5) : head(nullptr), btreeOrder(order) {
        identifierSpace = IS;
        bits = static_cast<int>(log2(IS));
    }

    ~CircularLinkedList() {
        if (head == nullptr) return;
        
        CircularNode* current = head->next;
        while (current != head) {
            CircularNode* next = current->next;
            delete current;
            current = next;
        }
        delete head;
        head = nullptr;
    }

    CircularNode* getHead() { return head; }
    
    int getIdentifierSpace() { return identifierSpace; }
    
    int getBits() { return bits; }

    bool isEmpty() { return head == nullptr; }

    int getMachineCount() {
        if (head == nullptr) return 0;
        int count = 1;
        CircularNode* current = head->next;
        while (current != head) {
            count++;
            current = current->next;
        }
        return count;
    }

    /**
     * @brief Insert machine in sorted order
     */
    void insert(int value) {
        CircularNode* newNode = new CircularNode(value, btreeOrder);
        newNode->RT.initialize(value, identifierSpace);

        if (head == nullptr) {
            head = newNode;
            newNode->next = newNode;
            return;
        }

        // Insert in sorted order
        if (value < head->key) {
            // Insert before head
            CircularNode* tail = head;
            while (tail->next != head) {
                tail = tail->next;
            }
            newNode->next = head;
            tail->next = newNode;
            head = newNode;
        } else {
            CircularNode* current = head;
            while (current->next != head && current->next->key < value) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
    }

    /**
     * @brief Insert machine with validation and file redistribution
     */
    void insertAfter(int value, int order) {
        if (search(value)) {
            cout << "\n  ERROR: Machine " << value << " already exists!\n";
            return;
        }
        
        if (value < 0 || value >= identifierSpace) {
            cout << "\n  ERROR: Machine ID must be in range [0, " << (identifierSpace - 1) << "]\n";
            return;
        }
        
        insert(value);
        updateRT();
        
        CircularNode* prev = SearchNewMachine(value);
        if (prev != nullptr && getMachineCount() > 1) {
            Traverse_insert(prev, order, identifierSpace);
        }
        
        cout << "\n  Machine " << value << " added successfully!\n";
    }

    /**
     * @brief Search for machine by ID
     */
    bool search(int value) {
        if (head == nullptr) return false;
        
        CircularNode* current = head;
        do {
            if (current->key == value) return true;
            current = current->next;
        } while (current != head);
        
        return false;
    }

    /**
     * @brief Delete machine and redistribute files
     */
    void deletekey(int value, int order) {
        if (head == nullptr) {
            cout << "\n  ERROR: Ring is empty!\n";
            return;
        }

        CircularNode* current = head;
        CircularNode* previous = nullptr;
        
        // Find the last node to maintain circularity
        CircularNode* tail = head;
        while (tail->next != head) {
            tail = tail->next;
        }
        
        // Find the node to delete
        do {
            if (current->key == value) break;
            previous = current;
            current = current->next;
        } while (current != head);
        
        if (current->key != value) {
            cout << "\n  ERROR: Machine " << value << " not found!\n";
            return;
        }
        
        // Transfer files to successor
        CircularNode* successor = current->next;
        if (current->next != current) {  // More than one machine
            Traverse_delete(current, successor, order);
        }
        
        // Remove from ring
        if (current == head) {
            if (current->next == current) {
                // Only one node
                head = nullptr;
            } else {
                head = head->next;
                tail->next = head;
            }
        } else {
            previous->next = current->next;
        }
        
        delete current;
        
        if (head != nullptr) {
            updateRT();
        }
        
        cout << "\n  Machine " << value << " removed successfully!\n";
    }

    /**
     * @brief Print all machines in ring
     */
    void print() {
        if (head == nullptr) {
            cout << "  (Ring is empty)\n";
            return;
        }
        
        cout << "  Ring: ";
        CircularNode* current = head;
        do {
            cout << current->key;
            current = current->next;
            if (current != head) cout << " -> ";
        } while (current != head);
        cout << " -> (head)\n";
    }

    /**
     * @brief Print detailed ring information
     */
    void printDetailed() {
        cout << "\n  +============================================================+\n";
        cout << "  |                    RING DHT STATUS                         |\n";
        cout << "  +============================================================+\n";
        cout << "  |  Identifier Space: " << bits << " bits (0-" << setw(5) << left << (identifierSpace - 1) 
             << ")                    |\n";
        cout << "  |  Number of Machines: " << setw(5) << left << getMachineCount() 
             << "                               |\n";
        cout << "  +------------------------------------------------------------+\n";
        
        if (head == nullptr) {
            cout << "  |  (Ring is empty)                                           |\n";
        } else {
            CircularNode* current = head;
            do {
                int fileCount = current->BTreeroot.countFiles(current->BTreeroot.root);
                cout << "  |  Machine " << setw(5) << left << current->key 
                     << " | Files: " << setw(5) << left << fileCount << "                         |\n";
                current = current->next;
            } while (current != head);
        }
        cout << "  +============================================================+\n";
    }

    /**
     * @brief Find successor of a key
     */
    CircularNode* succ(int value) {
        if (head == nullptr) return nullptr;
        
        CircularNode* current = head;
        do {
            if (current->key >= value) {
                return current;
            }
            current = current->next;
        } while (current != head);
        
        // Wrap around to head
        return head;
    }

    /**
     * @brief Update all routing tables
     */
    void updateRT() {
        if (head == nullptr) return;
        
        CircularNode* current = head;
        do {
            // Clear and reinitialize routing table
            current->RT.clear();
            current->RT.initialize(current->key, identifierSpace);
            
            // Update finger table entries
            DoublyNode<CircularNode>* finger = current->RT.head;
            int i = 0;
            while (finger != nullptr) {
                // FT[i] = succ(n + 2^i) where n is current machine's key
                int target = (current->key + (1 << i)) % identifierSpace;
                finger->filekey = target;
                CircularNode* succNode = succ(target);
                finger->m = succNode;
                finger->machinekey = succNode ? succNode->key : -1;
                finger = finger->next;
                i++;
            }
            current = current->next;
        } while (current != head);
    }

    /**
     * @brief Find machine responsible for a key using routing
     * @return Vector of machine IDs representing the routing path
     */
    vector<int> routeToKey(int startMachineId, int key) {
        vector<int> path;
        
        if (head == nullptr) return path;
        
        // Find starting machine
        CircularNode* current = head;
        do {
            if (current->key == startMachineId) break;
            current = current->next;
        } while (current != head);
        
        if (current->key != startMachineId) {
            cout << "  ERROR: Starting machine " << startMachineId << " not found!\n";
            return path;
        }
        
        path.push_back(current->key);
        set<int> visited;
        visited.insert(current->key);
        
        // Route using finger table
        while (true) {
            // Check if current machine is responsible
            CircularNode* pred = findPredecessor(current->key);
            int predKey = pred ? pred->key : -1;
            
            bool responsible = false;
            if (getMachineCount() == 1) {
                responsible = true;
            } else if (predKey < current->key) {
                responsible = (key > predKey && key <= current->key);
            } else {
                // Wrap around case
                responsible = (key > predKey || key <= current->key);
            }
            
            if (responsible) {
                break;
            }
            
            // Find best finger table entry
            CircularNode* nextHop = nullptr;
            DoublyNode<CircularNode>* finger = current->RT.head;
            
            // Use finger table for O(log N) routing
            while (finger != nullptr) {
                if (finger->m != nullptr) {
                    // Check if this finger gets us closer
                    if (isBetween(finger->machinekey, current->key, key)) {
                        nextHop = finger->m;
                    }
                }
                finger = finger->next;
            }
            
            // If no better hop found, go to immediate successor
            if (nextHop == nullptr) {
                nextHop = current->next;
            }
            
            // Prevent infinite loops
            if (visited.count(nextHop->key) > 0) {
                break;
            }
            
            current = nextHop;
            path.push_back(current->key);
            visited.insert(current->key);
        }
        
        return path;
    }

    /**
     * @brief Check if target is between start and end (circular)
     */
    bool isBetween(int target, int start, int end) {
        if (start < end) {
            return (target > start && target <= end);
        } else {
            return (target > start || target <= end);
        }
    }

    /**
     * @brief Find predecessor of a machine
     */
    CircularNode* findPredecessor(int machineKey) {
        if (head == nullptr) return nullptr;
        
        CircularNode* current = head;
        do {
            if (current->next->key == machineKey) {
                return current;
            }
            current = current->next;
        } while (current != head);
        
        return nullptr;
    }

    /**
     * @brief Print routing path
     */
    void printRoutingPath(const vector<int>& path, int fileKey, bool isStore = false) {
        cout << "\n  Routing Path for key " << fileKey << ":\n  ";
        for (size_t i = 0; i < path.size(); i++) {
            cout << path[i];
            if (i < path.size() - 1) {
                cout << " -> ";
            }
        }
        if (isStore && !path.empty()) {
            cout << " (STORED)";
        }
        cout << "\n";
    }

    /**
     * @brief Find machine that should store a key
     */
    CircularNode* findResponsibleMachine(int key) {
        if (head == nullptr) return nullptr;
        
        CircularNode* current = head;
        do {
            CircularNode* pred = findPredecessor(current->key);
            int predKey = pred ? pred->key : (identifierSpace - 1);
            
            bool responsible = false;
            if (getMachineCount() == 1) {
                responsible = true;
            } else if (predKey < current->key) {
                responsible = (key > predKey && key <= current->key);
            } else {
                responsible = (key > predKey || key <= current->key);
            }
            
            if (responsible) {
                return current;
            }
            current = current->next;
        } while (current != head);
        
        return head;  // Default to head
    }

    /**
     * @brief Insert file from a specific machine (shows routing path)
     */
    void InsertFileToTree(int machineKey, int fileKey, const string& path, int order) {
        if (head == nullptr) {
            cout << "\n  ERROR: Ring is empty! Add machines first.\n";
            return;
        }
        
        // Find starting machine
        if (!search(machineKey)) {
            cout << "\n  ERROR: Machine " << machineKey << " not found!\n";
            return;
        }
        
        FileNode file(fileKey, path);
        
        // Route to responsible machine
        cout << "\n  ============================================================\n";
        cout << "  INSERTING FILE\n";
        cout << "  ============================================================\n";
        cout << "  File Path: " << path << "\n";
        cout << "  File Hash Key: " << fileKey << "\n";
        cout << "  Starting from Machine: " << machineKey << "\n";
        
        vector<int> routingPath = routeToKey(machineKey, fileKey);
        
        if (routingPath.empty()) {
            cout << "  ERROR: Could not route to responsible machine!\n";
            return;
        }
        
        printRoutingPath(routingPath, fileKey, true);
        
        // Get responsible machine (last in path)
        int responsibleId = routingPath.back();
        CircularNode* responsible = findMachineById(responsibleId);
        
        if (responsible == nullptr) {
            cout << "  ERROR: Responsible machine not found!\n";
            return;
        }
        
        // Check if file already exists
        if (responsible->BTreeroot.searchFile(fileKey)) {
            cout << "\n  WARNING: File with key " << fileKey << " already exists on Machine " 
                 << responsible->key << "!\n";
            return;
        }
        
        // Insert into B-tree
        responsible->BTreeroot.insertHelper(file, order);
        
        cout << "\n  SUCCESS: File stored on Machine " << responsible->key << "\n";
        cout << "\n  B-Tree of Machine " << responsible->key << " after insertion:\n";
        responsible->BTreeroot.displayBFT(responsible->BTreeroot.root);
        cout << "  ============================================================\n";
    }

    /**
     * @brief Search for file from a specific machine (shows routing path)
     */
    CircularNode* SearchFile_WithMachine(int machineKey, int fileKey) {
        if (head == nullptr) {
            cout << "\n  ERROR: Ring is empty!\n";
            return nullptr;
        }
        
        cout << "\n  ============================================================\n";
        cout << "  SEARCHING FOR FILE\n";
        cout << "  ============================================================\n";
        cout << "  File Hash Key: " << fileKey << "\n";
        cout << "  Starting from Machine: " << machineKey << "\n";
        
        vector<int> routingPath = routeToKey(machineKey, fileKey);
        printRoutingPath(routingPath, fileKey);
        
        if (routingPath.empty()) {
            cout << "  ERROR: Could not route!\n";
            return nullptr;
        }
        
        int responsibleId = routingPath.back();
        CircularNode* responsible = findMachineById(responsibleId);
        
        if (responsible != nullptr && responsible->BTreeroot.searchFile(fileKey)) {
            cout << "\n  FOUND: File with key " << fileKey << " exists on Machine " 
                 << responsible->key << "\n";
            
            FileNode* file = responsible->BTreeroot.findFile(fileKey);
            if (file) {
                cout << "  File Path: " << file->path << "\n";
            }
            cout << "  ============================================================\n";
            return responsible;
        }
        
        cout << "\n  NOT FOUND: File with key " << fileKey << " does not exist.\n";
        cout << "  ============================================================\n";
        return nullptr;
    }

    /**
     * @brief Delete file from a specific machine (shows routing path)
     */
    bool DeleteFileFromTree(int machineKey, int fileKey) {
        if (head == nullptr) {
            cout << "\n  ERROR: Ring is empty!\n";
            return false;
        }
        
        cout << "\n  ============================================================\n";
        cout << "  DELETING FILE\n";
        cout << "  ============================================================\n";
        cout << "  File Hash Key: " << fileKey << "\n";
        cout << "  Starting from Machine: " << machineKey << "\n";
        
        vector<int> routingPath = routeToKey(machineKey, fileKey);
        printRoutingPath(routingPath, fileKey);
        
        if (routingPath.empty()) {
            cout << "  ERROR: Could not route!\n";
            return false;
        }
        
        int responsibleId = routingPath.back();
        CircularNode* responsible = findMachineById(responsibleId);
        
        if (responsible == nullptr) {
            cout << "  ERROR: Responsible machine not found!\n";
            return false;
        }
        
        // Get file info before deletion
        FileNode* file = responsible->BTreeroot.findFile(fileKey);
        if (file == nullptr) {
            cout << "\n  NOT FOUND: File with key " << fileKey << " does not exist.\n";
            cout << "  ============================================================\n";
            return false;
        }
        
        string filePath = file->path;
        
        // Delete from B-tree
        responsible->BTreeroot.deleteHelper(fileKey);
        
        cout << "\n  DELETED: File with key " << fileKey << "\n";
        cout << "  Removed from Machine: " << responsible->key << "\n";
        cout << "  File Path was: " << filePath << "\n";
        cout << "\n  B-Tree of Machine " << responsible->key << " after deletion:\n";
        responsible->BTreeroot.displayBFT(responsible->BTreeroot.root);
        cout << "  ============================================================\n";
        
        return true;
    }

    /**
     * @brief Find machine by ID
     */
    CircularNode* findMachineById(int id) {
        if (head == nullptr) return nullptr;
        
        CircularNode* current = head;
        do {
            if (current->key == id) return current;
            current = current->next;
        } while (current != head);
        
        return nullptr;
    }

    CircularNode* SearchNewMachine(int machineKey) {
        if (head == nullptr) return nullptr;
        
        CircularNode* current = head;
        do {
            if (current->next->key == machineKey) {
                return current;
            }
            current = current->next;
        } while (current != head);
        
        return nullptr;
    }

    /**
     * @brief Print routing table for a machine with proper formatting
     */
    void printRT(int machineKey) {
        CircularNode* machine = findMachineById(machineKey);
        
        if (machine == nullptr) {
            cout << "\n  ERROR: Machine " << machineKey << " not found!\n";
            return;
        }
        
        cout << "\n  +========================================================================+\n";
        cout << "  |              ROUTING TABLE (FINGER TABLE) - Machine " << setw(5) << left << machineKey << "           |\n";
        cout << "  +------------------------------------------------------------------------+\n";
        cout << "  |  Identifier Space: " << bits << " bits (0 to " << setw(5) << left << (identifierSpace - 1) << ")                         |\n";
        cout << "  +------------------------------------------------------------------------+\n";
        cout << "  |  Entry |    Formula          | Target ID | Successor Machine          |\n";
        cout << "  +------------------------------------------------------------------------+\n";
        
        DoublyNode<CircularNode>* finger = machine->RT.head;
        int i = 0;
        while (finger != nullptr && i < bits) {
            cout << "  |  FT[" << setw(2) << left << (i + 1) << "] | succ(" << setw(3) << machineKey 
                 << " + 2^" << setw(2) << left << i << ") | succ(" << setw(3) << left << finger->filekey 
                 << ") | Machine " << setw(5) << left << finger->machinekey << "              |\n";
            finger = finger->next;
            i++;
        }
        
        cout << "  +========================================================================+\n";
    }

    /**
     * @brief Print B-Tree for a machine with responsible range
     */
    void printBTree(int machineKey) {
        CircularNode* machine = findMachineById(machineKey);
        
        if (machine == nullptr) {
            cout << "\n  ERROR: Machine " << machineKey << " not found!\n";
            return;
        }
        
        // Find responsible range
        CircularNode* pred = findPredecessor(machineKey);
        int rangeStart = pred ? (pred->key + 1) % identifierSpace : 0;
        int rangeEnd = machineKey;
        
        cout << "\n  +========================================================================+\n";
        cout << "  |                    B-TREE - Machine " << setw(5) << left << machineKey << "                           |\n";
        cout << "  +------------------------------------------------------------------------+\n";
        
        if (getMachineCount() == 1) {
            cout << "  |  Responsible for: ALL IDs (0 to " << setw(5) << left << (identifierSpace - 1) << ")                          |\n";
        } else if (rangeStart <= rangeEnd) {
            cout << "  |  Responsible for IDs: [" << setw(3) << rangeStart << ", " << setw(3) << rangeEnd << "]                                   |\n";
        } else {
            cout << "  |  Responsible for IDs: [" << setw(3) << rangeStart << ", " << setw(3) << (identifierSpace - 1) 
                 << "] and [0, " << setw(3) << rangeEnd << "]                    |\n";
        }
        
        int fileCount = machine->BTreeroot.countFiles(machine->BTreeroot.root);
        cout << "  |  Total Files: " << setw(5) << left << fileCount << "                                                |\n";
        cout << "  +========================================================================+\n";
        
        machine->BTreeroot.displayBFT(machine->BTreeroot.root);
        
        if (fileCount > 0) {
            machine->BTreeroot.displayAllFiles(machine->BTreeroot.root);
        }
    }
};
