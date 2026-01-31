/**
 * @file DoublyLL.h
 * @brief Doubly Linked List for Routing Table (Finger Table) implementation
 * @details Each machine has a routing table stored as a doubly linked list
 *          with log2(identifierSpace) entries
 * 
 * Compile: g++ -std=c++17 -Wall -o ipfs_dht src/*.cpp
 */

#pragma once
#include <iostream>
#include <cmath>
using namespace std;

// Forward declaration
class CircularNode;

/**
 * @brief Node in the routing table (finger table entry)
 * @tparam Node Type of machine node (CircularNode)
 */
template<typename Node>
class DoublyNode {
public:
    int filekey;      // Target ID: (machine_id + 2^i) mod identifier_space
    int machinekey;   // Successor machine's ID
    Node* m;          // Pointer to successor machine

    DoublyNode* prev;
    DoublyNode* next;

    DoublyNode() {
        filekey = -1;
        machinekey = -1;
        m = nullptr;
        prev = nullptr;
        next = nullptr;
    }
    
    DoublyNode(int f, int mk) {
        filekey = f;
        machinekey = mk;
        m = nullptr;
        prev = nullptr;
        next = nullptr;
    }
};

/**
 * @brief Routing Table (Finger Table) implementation
 * @details Stores log2(identifier_space) entries for O(log N) routing
 * @tparam Node Type of machine node
 */
template<class Node>
class DoublyLinkedList {
public:
    int index;                      // Current entry index
    int m_val;                      // Machine ID this table belongs to
    DoublyNode<Node>* head;
    DoublyNode<Node>* tail;

    DoublyLinkedList() {
        index = 0;
        m_val = 0;
        head = nullptr;
        tail = nullptr;
    }
    
    DoublyLinkedList(int machine_key, int identifier_space) {
        m_val = machine_key;
        index = 0;
        head = nullptr;
        tail = nullptr;
    }

    ~DoublyLinkedList() {
        clear();
    }

    void clear() {
        DoublyNode<Node>* current = head;
        while (current != nullptr) {
            DoublyNode<Node>* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        index = 0;
    }

    /**
     * @brief Initialize routing table with finger table entries
     * @param machine_key ID of the machine
     * @param identifierSpace Total identifier space (2^bits)
     */
    void initialize(int machine_key, int identifierSpace) {
        m_val = machine_key;
        
        // Number of entries = log2(identifierSpace)
        int numEntries = static_cast<int>(log2(identifierSpace));
        if (numEntries < 1) numEntries = 1;
        
        for (int i = 0; i < numEntries; i++) {
            // FT[i] = (machine_key + 2^i) mod identifierSpace
            long long temp = machine_key + (1LL << i);
            temp = temp % identifierSpace;
            
            DoublyNode<Node>* newNode = new DoublyNode<Node>(static_cast<int>(temp), -1);
            
            if (head == nullptr) {
                head = tail = newNode;
            } else {
                tail->next = newNode;
                newNode->prev = tail;
                tail = newNode;
            }
        }
    }

    DoublyNode<Node>* getHead() {
        return head;
    }

    bool isEmpty() {
        return head == nullptr;
    }

    bool search(int n) {
        DoublyNode<Node>* current = head;
        while (current != nullptr) {
            if (current->filekey == n) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    void update(int old_file, int new_file) {
        DoublyNode<Node>* current = head;
        while (current != nullptr) {
            if (current->filekey == old_file) {
                current->filekey = new_file;
                return;
            }
            current = current->next;
        }
    }

    void setHead(DoublyNode<Node>* h) {
        head = h;
    }
    
    void setTail(DoublyNode<Node>* t) {
        tail = t;
    }

    void print() {
        DoublyNode<Node>* current = head;
        while (current != nullptr) {
            cout << current->filekey << endl;
            current = current->next;
        }
    }
};
