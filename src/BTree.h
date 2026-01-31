/**
 * @file BTree.h
 * @brief B-Tree implementation for file storage on each machine
 * @details Each machine stores its files in a B-Tree indexed by file hash
 * 
 * Compile: g++ -std=c++17 -Wall -o ipfs_dht src/*.cpp
 */

#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "Queue.h"
using namespace std;

/**
 * @brief File node storing key (hash) and path
 */
class FileNode {
public:
    int key;
    string path;
    
    FileNode() : key(-1), path("") {}
    FileNode(int k, const string& p) : key(k), path(p) {}
};

/**
 * @brief B-Tree node
 */
class BTreeNode {
public:
    int MAX;                    // Maximum keys per node (order - 1)
    int MIN;                    // Minimum keys per node
    int count;                  // Current number of keys
    FileNode* value;            // Array of file nodes (1-indexed)
    BTreeNode** child;          // Array of child pointers

    BTreeNode(int order) {
        MAX = order - 1;
        if (order % 2 == 0)
            MIN = order / 2;
        else
            MIN = (order / 2) + 1;
        MIN--;
        if (MIN < 1) MIN = 1;
        
        count = 0;
        value = new FileNode[MAX + 2];
        child = new BTreeNode*[MAX + 3];
        
        for (int i = 0; i < MAX + 2; ++i) {
            value[i].key = -1;
            value[i].path = "";
        }
        for (int i = 0; i < MAX + 3; ++i) {
            child[i] = nullptr;
        }
    }
    
    BTreeNode(int max, int min) {
        MAX = max;
        MIN = min;
        count = 0;
        value = new FileNode[MAX + 2];
        child = new BTreeNode*[MAX + 3];

        for (int i = 0; i < MAX + 2; ++i) {
            value[i].key = -1;
            value[i].path = "";
        }
        for (int i = 0; i < MAX + 3; ++i) {
            child[i] = nullptr;
        }
    }
    
    ~BTreeNode() {
        delete[] value;
        delete[] child;
    }
};

/**
 * @brief B-Tree for file storage
 */
class BTree {
public:
    BTreeNode* root;
    int order;

    BTree() : root(nullptr), order(5) {}
    
    BTree(int o) : root(nullptr), order(o) {}

    ~BTree() {
        destroyTree(root);
    }

    void destroyTree(BTreeNode* node) {
        if (node == nullptr) return;
        for (int i = 0; i <= node->count; i++) {
            if (node->child[i] != nullptr) {
                destroyTree(node->child[i]);
            }
        }
        delete node;
    }

    /**
     * @brief Display B-Tree in breadth-first order with proper formatting
     */
    void displayBFT(BTreeNode* node) {
        if (node == nullptr) {
            cout << "  (empty tree)" << endl;
            return;
        }

        Queue<BTreeNode*> q;
        Queue<int> levels;
        q.enqueue(node);
        levels.enqueue(0);
        
        int currentLevel = -1;
        int fileCount = 0;
        
        cout << "\n  Tree Structure (BFS order):\n";
        cout << "  ---------------------------\n";
        
        while (!q.is_empty()) {
            BTreeNode* current = q.peek();
            int level = levels.peek();
            q.dequeue();
            levels.dequeue();

            if (level != currentLevel) {
                if (currentLevel >= 0) cout << " ]\n";
                currentLevel = level;
                cout << "  Level " << level << ": [ ";
            } else {
                cout << " | ";
            }

            // Display keys in current node
            cout << "[";
            for (int i = 1; i <= current->count; i++) {
                if (i > 1) cout << ", ";
                cout << current->value[i].key;
                fileCount++;
            }
            cout << "]";

            // Enqueue children
            for (int i = 0; i <= current->count; i++) {
                if (current->child[i] != nullptr) {
                    q.enqueue(current->child[i]);
                    levels.enqueue(level + 1);
                }
            }
        }
        cout << " ]\n\n";
        cout << "  Total files: " << fileCount << endl;
    }

    /**
     * @brief Display all files stored in tree
     */
    void displayAllFiles(BTreeNode* node) {
        if (node == nullptr) return;

        Queue<BTreeNode*> q;
        q.enqueue(node);
        
        cout << "\n  Stored Files:\n";
        cout << "  " << string(60, '-') << "\n";
        cout << "  " << left << setw(10) << "Key" << " | " << "Path" << endl;
        cout << "  " << string(60, '-') << "\n";

        while (!q.is_empty()) {
            BTreeNode* current = q.peek();
            q.dequeue();

            for (int i = 1; i <= current->count; i++) {
                cout << "  " << left << setw(10) << current->value[i].key 
                     << " | " << current->value[i].path << endl;
            }

            for (int i = 0; i <= current->count; i++) {
                if (current->child[i] != nullptr) {
                    q.enqueue(current->child[i]);
                }
            }
        }
        cout << "  " << string(60, '-') << "\n";
    }

    /**
     * @brief Count total files in tree
     */
    int countFiles(BTreeNode* node) {
        if (node == nullptr) return 0;
        
        int count = node->count;
        for (int i = 0; i <= node->count; i++) {
            if (node->child[i] != nullptr) {
                count += countFiles(node->child[i]);
            }
        }
        return count;
    }

    /**
     * @brief Get all files as vector
     */
    vector<FileNode> getAllFiles(BTreeNode* node) {
        vector<FileNode> files;
        if (node == nullptr) return files;

        Queue<BTreeNode*> q;
        q.enqueue(node);

        while (!q.is_empty()) {
            BTreeNode* current = q.peek();
            q.dequeue();

            for (int i = 1; i <= current->count; i++) {
                files.push_back(current->value[i]);
            }

            for (int i = 0; i <= current->count; i++) {
                if (current->child[i] != nullptr) {
                    q.enqueue(current->child[i]);
                }
            }
        }
        return files;
    }

    BTreeNode* insert(FileNode file, BTreeNode* node, int ord) {
        FileNode promoted;
        BTreeNode* newChild;
        bool needsNewRoot = setval(file, node, &promoted, &newChild, ord);
        
        if (needsNewRoot) {
            BTreeNode* newRoot;
            if (node) {
                newRoot = new BTreeNode(node->MAX, node->MIN);
            } else {
                newRoot = new BTreeNode(ord);
            }
            newRoot->count = 1;
            newRoot->value[1] = promoted;
            newRoot->child[0] = node;
            newRoot->child[1] = newChild;
            return newRoot;
        }
        return node;
    }

    bool setval(FileNode file, BTreeNode* n, FileNode* p, BTreeNode** c, int ord) {
        int k;
        if (n == nullptr) {
            *p = file;
            *c = nullptr;
            return true;
        }
        
        if (searchNode(file.key, n, &k)) {
            cout << "  Key value " << file.key << " already exists.\n";
            return false;
        }
        
        if (setval(file, n->child[k], p, c, ord)) {
            if (n->count < n->MAX) {
                fillnode(*p, *c, n, k);
                return false;
            } else {
                split(*p, *c, n, k, p, c);
                return true;
            }
        }
        return false;
    }

    BTreeNode* search(int key, BTreeNode* node, int* pos) {
        if (node == nullptr) {
            return nullptr;
        }
        
        if (searchNode(key, node, pos)) {
            return node;
        }
        return search(key, node->child[*pos], pos);
    }

    bool searchNode(int key, BTreeNode* n, int* pos) {
        if (key < n->value[1].key) {
            *pos = 0;
            return false;
        }
        
        *pos = n->count;
        while (key < n->value[*pos].key && *pos > 1) {
            (*pos)--;
        }
        
        return (key == n->value[*pos].key);
    }

    void fillnode(FileNode file, BTreeNode* c, BTreeNode* n, int k) {
        for (int i = n->count; i > k; i--) {
            n->value[i + 1] = n->value[i];
            n->child[i + 1] = n->child[i];
        }
        n->value[k + 1] = file;
        n->child[k + 1] = c;
        n->count++;
    }

    void split(FileNode file, BTreeNode* c, BTreeNode* n, int k, FileNode* y, BTreeNode** newNode) {
        int mid;
        if (k <= n->MIN)
            mid = n->MIN;
        else
            mid = n->MIN + 1;

        *newNode = new BTreeNode(n->MAX, n->MIN);
        
        for (int i = mid + 1; i <= n->MAX; i++) {
            (*newNode)->value[i - mid] = n->value[i];
            (*newNode)->child[i - mid] = n->child[i];
        }
        (*newNode)->count = n->MAX - mid;
        n->count = mid;

        if (k <= n->MIN)
            fillnode(file, c, n, k);
        else
            fillnode(file, c, *newNode, k - mid);

        *y = n->value[n->count];
        (*newNode)->child[0] = n->child[n->count];
        n->count--;
    }

    BTreeNode* del(int file_key, BTreeNode* node) {
        if (node == nullptr) return nullptr;
        
        if (!delhelp(file_key, node)) {
            cout << "  Value not found: " << file_key << endl;
        } else if (node->count == 0) {
            BTreeNode* temp = node;
            node = node->child[0];
            temp->child[0] = nullptr;
            delete temp;
        }
        return node;
    }

    bool delhelp(int file_key, BTreeNode* node) {
        if (node == nullptr) return false;
        
        int i;
        bool flag = searchNode(file_key, node, &i);
        
        if (flag) {
            if (node->child[i - 1]) {
                copysucc(node, i);
                flag = delhelp(node->value[i].key, node->child[i]);
                if (!flag) {
                    cout << "  Value not found: " << file_key << endl;
                }
            } else {
                clear(node, i);
            }
        } else {
            flag = delhelp(file_key, node->child[i]);
        }
        
        if (node->child[i] != nullptr && node->child[i]->count < node->MIN) {
            restore(node, i);
        }
        
        return flag;
    }

    void clear(BTreeNode* node, int k) {
        for (int i = k + 1; i <= node->count; i++) {
            node->value[i - 1] = node->value[i];
            node->child[i - 1] = node->child[i];
        }
        node->count--;
    }

    void copysucc(BTreeNode* node, int i) {
        BTreeNode* temp = node->child[i];
        while (temp->child[0])
            temp = temp->child[0];
        node->value[i] = temp->value[1];
    }

    void restore(BTreeNode* node, int i) {
        if (i == 0) {
            if (node->child[1]->count > node->MIN)
                leftshift(node, 1);
            else
                merge(node, 1);
        } else if (i == node->count) {
            if (node->child[i - 1]->count > node->MIN)
                rightshift(node, i);
            else
                merge(node, i);
        } else {
            if (node->child[i - 1]->count > node->MIN)
                rightshift(node, i);
            else if (node->child[i + 1]->count > node->MIN)
                leftshift(node, i + 1);
            else
                merge(node, i);
        }
    }

    void rightshift(BTreeNode* node, int k) {
        BTreeNode* temp = node->child[k];
        
        for (int i = temp->count; i > 0; i--) {
            temp->value[i + 1] = temp->value[i];
            temp->child[i + 1] = temp->child[i];
        }
        
        temp->child[1] = temp->child[0];
        temp->count++;
        temp->value[1] = node->value[k];
        
        BTreeNode* left = node->child[k - 1];
        node->value[k] = left->value[left->count];
        node->child[k]->child[0] = left->child[left->count];
        left->count--;
    }

    void leftshift(BTreeNode* node, int k) {
        BTreeNode* left = node->child[k - 1];
        left->count++;
        left->value[left->count] = node->value[k];
        left->child[left->count] = node->child[k]->child[0];
        
        BTreeNode* right = node->child[k];
        node->value[k] = right->value[1];
        right->child[0] = right->child[1];
        right->count--;
        
        for (int i = 1; i <= right->count; i++) {
            right->value[i] = right->value[i + 1];
            right->child[i] = right->child[i + 1];
        }
    }

    void merge(BTreeNode* node, int k) {
        BTreeNode* right = node->child[k];
        BTreeNode* left = node->child[k - 1];
        
        left->count++;
        left->value[left->count] = node->value[k];
        
        if (right->child[0] != nullptr) {
            left->child[left->count] = right->child[0];
        }
        
        for (int i = 1; i <= right->count; i++) {
            left->count++;
            left->value[left->count] = right->value[i];
            left->child[left->count] = right->child[i];
        }
        
        for (int i = k; i < node->count; i++) {
            node->value[i] = node->value[i + 1];
            node->child[i] = node->child[i + 1];
        }
        
        node->count--;
        delete right;
    }

    // Helper functions
    void insertHelper(FileNode f, int ord) {
        root = insert(f, root, ord);
    }

    void deleteHelper(int file_key) {
        root = del(file_key, root);
    }

    bool searchFile(int file_key) {
        int pos;
        return search(file_key, root, &pos) != nullptr;
    }

    FileNode* findFile(int file_key) {
        int pos;
        BTreeNode* node = search(file_key, root, &pos);
        if (node != nullptr) {
            return &(node->value[pos]);
        }
        return nullptr;
    }
};
