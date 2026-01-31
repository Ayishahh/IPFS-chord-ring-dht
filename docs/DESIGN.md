# IPFS Ring DHT Simulator - Design Document

## 1. Overview

This document describes the architecture and design decisions for the IPFS Ring DHT (Distributed Hash Table) Simulator. The system implements a Chord-like protocol for distributed file storage using a ring topology.

## 2. Architecture

### 2.1 System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         IPFS Class                                   │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                 CircularLinkedList                           │    │
│  │  ┌───────────────────────────────────────────────────────┐  │    │
│  │  │                     Ring DHT                           │  │    │
│  │  │                                                        │  │    │
│  │  │    ┌──────┐    ┌──────┐    ┌──────┐    ┌──────┐      │  │    │
│  │  │    │Node 1│───▶│Node 4│───▶│Node 9│───▶│Node 14│──┐   │  │    │
│  │  │    └──────┘    └──────┘    └──────┘    └──────┘  │   │  │    │
│  │  │        ▲                                          │   │  │    │
│  │  │        └──────────────────────────────────────────┘   │  │    │
│  │  │                                                        │  │    │
│  │  │  Each Node Contains:                                   │  │    │
│  │  │  ┌─────────────────┐  ┌─────────────────────────────┐ │  │    │
│  │  │  │  Routing Table  │  │        B-Tree               │ │  │    │
│  │  │  │  (Finger Table) │  │   (File Storage)            │ │  │    │
│  │  │  │                 │  │                             │ │  │    │
│  │  │  │  FT[1] → Node X │  │      [key1, key2]          │ │  │    │
│  │  │  │  FT[2] → Node Y │  │     /     |     \          │ │  │    │
│  │  │  │  FT[3] → Node Z │  │  [...]  [...]  [...]       │ │  │    │
│  │  │  │  ...            │  │                             │ │  │    │
│  │  │  └─────────────────┘  └─────────────────────────────┘ │  │    │
│  │  └───────────────────────────────────────────────────────┘  │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

### 2.2 Class Diagram

```
┌───────────────────────────────────────────────────────────────┐
│                           IPFS                                 │
├───────────────────────────────────────────────────────────────┤
│ - C: CircularLinkedList*                                       │
│ - identifierSpace: int                                         │
│ - bits: int                                                    │
│ - order: int                                                   │
├───────────────────────────────────────────────────────────────┤
│ + InsertMachines(arr, num): void                              │
│ + InsertMachine(key, order): void                             │
│ + DeleteMachine(key, order): void                             │
│ + insertFile(machineKey, fileKey, path, order): void          │
│ + DeleteFile(machineKey, fileKey): bool                       │
│ + SearchFile(machineKey, fileKey): CircularNode*              │
│ + printBTree(machineKey): void                                │
│ + PrintRT(machineKey): void                                   │
└───────────────────────────────────────────────────────────────┘
                              │
                              │ contains
                              ▼
┌───────────────────────────────────────────────────────────────┐
│                     CircularLinkedList                         │
├───────────────────────────────────────────────────────────────┤
│ - head: CircularNode*                                          │
│ - identifierSpace: int                                         │
│ - bits: int                                                    │
│ - btreeOrder: int                                              │
├───────────────────────────────────────────────────────────────┤
│ + insert(value): void                                          │
│ + deletekey(value, order): void                                │
│ + search(value): bool                                          │
│ + succ(value): CircularNode*                                   │
│ + updateRT(): void                                             │
│ + routeToKey(startMachineId, key): vector<int>                │
│ + InsertFileToTree(machineKey, fileKey, path, order): void    │
│ + DeleteFileFromTree(machineKey, fileKey): bool               │
│ + SearchFile_WithMachine(machineKey, fileKey): CircularNode*  │
│ + printRT(machineKey): void                                   │
│ + printBTree(machineKey): void                                │
└───────────────────────────────────────────────────────────────┘
                              │
                              │ contains many
                              ▼
┌───────────────────────────────────────────────────────────────┐
│                       CircularNode                             │
├───────────────────────────────────────────────────────────────┤
│ - key: int                                                     │
│ - next: CircularNode*                                          │
│ - RT: DoublyLinkedList<CircularNode>                          │
│ - BTreeroot: BTree                                            │
└───────────────────────────────────────────────────────────────┘
          │                                    │
          │ contains                           │ contains
          ▼                                    ▼
┌─────────────────────────┐      ┌─────────────────────────────┐
│   DoublyLinkedList      │      │          BTree               │
│   (Routing Table)       │      │    (File Storage)            │
├─────────────────────────┤      ├─────────────────────────────┤
│ - head: DoublyNode*     │      │ - root: BTreeNode*          │
│ - tail: DoublyNode*     │      │ - order: int                │
├─────────────────────────┤      ├─────────────────────────────┤
│ + initialize()          │      │ + insert(file, order): void │
│ + search(key): bool     │      │ + del(key): void            │
└─────────────────────────┘      │ + search(key): BTreeNode*   │
          │                       │ + displayBFT(): void        │
          │ contains many         └─────────────────────────────┘
          ▼                                    │
┌─────────────────────────┐                    │ contains many
│      DoublyNode         │                    ▼
├─────────────────────────┤      ┌─────────────────────────────┐
│ - filekey: int          │      │        BTreeNode             │
│ - machinekey: int       │      ├─────────────────────────────┤
│ - m: CircularNode*      │      │ - value: FileNode[]         │
│ - prev: DoublyNode*     │      │ - child: BTreeNode*[]       │
│ - next: DoublyNode*     │      │ - count: int                │
└─────────────────────────┘      │ - MAX, MIN: int             │
                                 └─────────────────────────────┘
```

## 3. Data Structures

### 3.1 Circular Linked List (Ring)

**Purpose**: Represents the ring of machines in the DHT.

**Properties**:
- Nodes are sorted by machine ID
- Last node points back to first (circular)
- O(n) insertion/deletion, O(n) search

**Implementation Details**:
```cpp
class CircularLinkedList {
    CircularNode* head;
    int identifierSpace;  // 2^bits
    int bits;
};
```

### 3.2 Doubly Linked List (Routing Table / Finger Table)

**Purpose**: Each machine has a finger table for O(log N) routing.

**Properties**:
- Contains log2(identifierSpace) entries
- Entry i points to successor of (n + 2^i) mod 2^m

**Finger Table Formula**:
```
FT[i] = succ(n + 2^(i-1)) mod 2^m
where n = current machine's ID
      m = number of bits
      i = finger table index (1 to m)
```

### 3.3 B-Tree (File Storage)

**Purpose**: Each machine stores its files in a B-tree.

**Properties**:
- Order configurable (default 5)
- Keys are file hashes
- Values are file paths
- Maintains sorted order for efficient search

## 4. Algorithms

### 4.1 Hash Function

Uses SHA-1 truncated to identifier space:
```cpp
int generateHashInSpace(const string& input, int identifierSpace) {
    string hash = SHA1::hash(input);
    unsigned long long value = hexToInt(hash.substr(0, 8));
    return value % identifierSpace;
}
```

### 4.2 Successor Finding

```cpp
CircularNode* succ(int key) {
    // Find first machine with ID >= key
    CircularNode* current = head;
    do {
        if (current->key >= key) return current;
        current = current->next;
    } while (current != head);
    return head;  // Wrap around
}
```

### 4.3 Routing Algorithm

```cpp
vector<int> routeToKey(int startMachine, int key) {
    vector<int> path;
    CircularNode* current = findMachine(startMachine);
    
    while (!isResponsible(current, key)) {
        path.push_back(current->key);
        
        // Find best finger
        CircularNode* next = findBestFinger(current, key);
        
        if (next == current) break;
        current = next;
    }
    
    path.push_back(current->key);
    return path;
}
```

**Time Complexity**: O(log N) where N is number of machines

### 4.4 File Responsibility

Machine n is responsible for key k if:
```
predecessor(n).key < k <= n.key
```

With wrap-around handling for circular ID space.

## 5. Operations

### 5.1 Add Machine

1. Insert machine into sorted position in ring
2. Initialize finger table
3. Update all machines' finger tables
4. Redistribute files from successor

**Time**: O(N) for ring traversal + O(N * log S) for finger table updates

### 5.2 Remove Machine

1. Transfer all files to successor
2. Remove machine from ring
3. Update all machines' finger tables

### 5.3 Insert File

1. Hash file path to get key
2. Route from source machine to responsible machine
3. Insert into responsible machine's B-tree
4. Display routing path

### 5.4 Search File

1. Hash file path to get key
2. Route from source machine
3. Check B-tree at each machine
4. Display routing path and result

### 5.5 Delete File

1. Route to responsible machine
2. Delete from B-tree
3. Display routing path

## 6. Time & Space Complexity

| Operation | Time Complexity | Space Complexity |
|-----------|-----------------|------------------|
| Add Machine | O(N) | O(log S) |
| Remove Machine | O(N) | O(1) |
| Route to Key | O(log N) | O(log N) |
| Insert File | O(log N + log F) | O(1) |
| Search File | O(log N + log F) | O(1) |
| Delete File | O(log N + log F) | O(1) |
| Update Routing Tables | O(N * log S) | O(1) |

Where:
- N = number of machines
- S = identifier space size (2^bits)
- F = number of files on a machine

## 7. Design Decisions

### 7.1 Why Circular Linked List?

- Natural representation of ring topology
- Easy to maintain sorted order
- Successor relationship built into structure

### 7.2 Why Doubly Linked List for Routing Table?

- Easy traversal in both directions
- Simple to update individual entries
- Memory efficient for small tables

### 7.3 Why B-Tree for Storage?

- Efficient for both read and write
- Maintains sorted order
- Good cache performance
- Handles variable number of files well

### 7.4 Why 31-bit Limit?

- C++ `int` is typically 32-bit
- Bit shifting `1 << 32` causes undefined behavior
- 31 bits provides 2 billion possible IDs (sufficient for simulation)

## 8. Future Improvements

1. **Persistent Storage**: Add file I/O for saving state
2. **Concurrent Operations**: Thread-safe data structures
3. **Network Simulation**: Add latency and failure simulation
4. **Replication**: Implement file replication for fault tolerance
5. **Load Balancing**: Virtual nodes for better distribution

## 9. References

- Stoica, I., et al. "Chord: A Scalable Peer-to-peer Lookup Service for Internet Applications"
- IPFS Documentation: https://docs.ipfs.io/
- Cormen, T.H., et al. "Introduction to Algorithms" (B-Tree chapter)
