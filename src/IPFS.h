/**
 * @file IPFS.h
 * @brief IPFS Ring DHT Simulator main class
 * @details Provides high-level interface for DHT operations
 * 
 * Compile: g++ -std=c++17 -Wall -o ipfs_dht src/*.cpp
 */

#pragma once
#include "CircularLL.h"
#include <vector>
#include <string>

/**
 * @brief IPFS Ring DHT Simulator
 */
class IPFS {
public:
    CircularLinkedList* C;      // Circular linked list of machines
    int identifierSpace;        // 2^bits
    int bits;                   // Number of bits
    int order;                  // B-tree order

    IPFS() : C(nullptr), identifierSpace(16), bits(4), order(5) {}

    /**
     * @brief Constructor with configurable identifier space
     * @param numBits Number of bits (1-31)
     * @param btreeOrder Order of B-tree for file storage
     */
    IPFS(int numBits, int btreeOrder) {
        // Validate and set bits
        if (numBits < 1) numBits = 1;
        if (numBits > 31) numBits = 31;  // Limit to 31 bits to avoid int overflow
        
        bits = numBits;
        identifierSpace = 1 << bits;  // 2^bits
        order = btreeOrder;
        
        C = new CircularLinkedList(identifierSpace, btreeOrder);
    }

    ~IPFS() {
        delete C;
    }

    // Getters
    int getBits() const { return bits; }
    int getIdentifierSpace() const { return identifierSpace; }
    int getMaxId() const { return identifierSpace - 1; }
    int getMachineCount() const { return C ? C->getMachineCount() : 0; }

    /**
     * @brief Validate machine ID
     */
    bool isValidMachineId(int id) const {
        return id >= 0 && id < identifierSpace;
    }

    /**
     * @brief Insert multiple machines (initial setup)
     */
    void InsertMachines(int* arr, int num_mac) {
        for (int i = 0; i < num_mac; i++) {
            if (isValidMachineId(arr[i])) {
                C->insert(arr[i]);
            } else {
                cout << "  WARNING: Machine ID " << arr[i] << " is out of range and was skipped.\n";
            }
        }
        C->updateRT();
        cout << "\n  " << C->getMachineCount() << " machine(s) added successfully!\n";
    }

    /**
     * @brief Add a single machine dynamically
     */
    void InsertMachine(int machineKey, int btreeOrder) {
        if (!isValidMachineId(machineKey)) {
            cout << "\n  ERROR: Machine ID must be in range [0, " << getMaxId() << "]\n";
            return;
        }
        C->insertAfter(machineKey, btreeOrder);
    }

    /**
     * @brief Remove a machine dynamically
     */
    void DeleteMachine(int machineKey, int btreeOrder) {
        C->deletekey(machineKey, btreeOrder);
    }

    /**
     * @brief Insert file from specified machine
     */
    void insertFile(int machineKey, int fileKey, const string& path, int btreeOrder) {
        C->InsertFileToTree(machineKey, fileKey, path, btreeOrder);
    }

    /**
     * @brief Delete file starting from specified machine
     */
    bool DeleteFile(int machineKey, int fileKey) {
        return C->DeleteFileFromTree(machineKey, fileKey);
    }

    /**
     * @brief Search for file starting from specified machine
     */
    CircularNode* SearchFile(int machineKey, int fileKey) {
        return C->SearchFile_WithMachine(machineKey, fileKey);
    }

    /**
     * @brief Print B-tree for a machine
     */
    void printBTree(int machineKey) {
        C->printBTree(machineKey);
    }

    /**
     * @brief Print routing table for a machine
     */
    void PrintRT(int machineKey) {
        C->printRT(machineKey);
    }

    /**
     * @brief Print all machines
     */
    void printRing() {
        C->print();
    }

    /**
     * @brief Print detailed ring status
     */
    void printDetailedStatus() {
        C->printDetailed();
    }

    /**
     * @brief Check if machine exists
     */
    bool machineExists(int machineKey) {
        return C->search(machineKey);
    }

    /**
     * @brief Print all routing tables
     */
    void printAllRoutingTables() {
        if (C->isEmpty()) {
            cout << "\n  Ring is empty!\n";
            return;
        }
        
        CircularNode* current = C->head;
        do {
            PrintRT(current->key);
            current = current->next;
        } while (current != C->head);
    }

    /**
     * @brief Print all B-trees
     */
    void printAllBTrees() {
        if (C->isEmpty()) {
            cout << "\n  Ring is empty!\n";
            return;
        }
        
        CircularNode* current = C->head;
        do {
            printBTree(current->key);
            current = current->next;
        } while (current != C->head);
    }
};
