/**
 * @file Queue.h
 * @brief Generic Queue implementation using linked list
 * @details Used for BFS traversals in B-Tree operations
 * 
 * Compile: g++ -std=c++17 -Wall -o ipfs_dht src/*.cpp
 */

#pragma once
#include <iostream>
using namespace std;

template<typename T>
class QueueNode
{
public:
    T data;
    QueueNode* next;

    QueueNode(T d) : data(d), next(nullptr) {}
};

template <typename T>
class Queue
{
private:
    QueueNode<T>* front;
    QueueNode<T>* rear;
    int count;

public:
    Queue() : front(nullptr), rear(nullptr), count(0) {}

    Queue(const Queue& other) : front(nullptr), rear(nullptr), count(0) {
        // Deep copy
        QueueNode<T>* current = other.front;
        while (current != nullptr) {
            enqueue(current->data);
            current = current->next;
        }
    }

    ~Queue() {
        clear();
    }

    void enqueue(T item) {
        QueueNode<T>* newNode = new QueueNode<T>(item);
        if (is_empty()) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        count++;
    }

    T dequeue() {
        if (is_empty()) {
            cerr << "Queue is empty.\n";
            return T();
        }

        T data = front->data;
        QueueNode<T>* temp = front;
        front = front->next;
        delete temp;
        count--;

        if (front == nullptr) {
            rear = nullptr;
        }

        return data;
    }

    T peek() const {
        if (is_empty()) {
            cerr << "Queue is empty\n";
            return T();
        }
        return front->data;
    }

    bool is_empty() const {
        return (count == 0);
    }

    int size() const {
        return count;
    }

    void clear() {
        while (!is_empty()) {
            dequeue();
        }
    }
};
