#ifndef TINY_SHELL_LINKEDLIST_H
#define TINY_SHELL_LINKEDLIST_H

#include <cstddef>
#include <string>

class LinkedList {
public:
    static constexpr std::size_t LIMIT = 50;

    LinkedList() = default;
    ~LinkedList();

    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    void append(const std::string& command);
    void display() const;
    std::size_t size() const { return count_; }
    bool empty() const { return head_ == nullptr; }

    void startNavigation();
    const std::string* older();
    const std::string* newer();

private:
    struct Node {
        explicit Node(const std::string& value) : command(value) {}
        std::string command;
        Node* next = nullptr;
        Node* prev = nullptr;
    };

    void clear();
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    Node* cursor_ = nullptr;
    std::size_t count_ = 0;
};

#endif
