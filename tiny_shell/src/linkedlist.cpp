#include "linkedlist.h"

#include <iostream>

LinkedList::~LinkedList() {
    clear();
}

void LinkedList::clear() {
    while (head_ != nullptr) {
        Node* oldHead = head_;
        head_ = head_->next;
        delete oldHead;
    }
    tail_ = nullptr;
    cursor_ = nullptr;
    count_ = 0;
}

void LinkedList::append(const std::string& command) {
    if (command.empty()) {
        return;
    }

    cursor_ = nullptr;
    Node* node = new Node(command);
    node->prev = tail_;
    if (tail_ != nullptr) {
        tail_->next = node;
    } else {
        head_ = node;
    }
    tail_ = node;
    ++count_;

    if (count_ > LIMIT) {
        Node* oldHead = head_;
        head_ = head_->next;
        head_->prev = nullptr;
        delete oldHead;
        --count_;
    }
}

void LinkedList::display() const {
    std::size_t index = 1;
    for (Node* node = head_; node != nullptr; node = node->next) {
        std::cout << index++ << "  " << node->command << '\n';
    }
}

void LinkedList::startNavigation() {
    cursor_ = nullptr;
}

const std::string* LinkedList::older() {
    if (tail_ == nullptr) {
        return nullptr;
    }
    if (cursor_ == nullptr) {
        cursor_ = tail_;
    } else if (cursor_->prev != nullptr) {
        cursor_ = cursor_->prev;
    }
    return &cursor_->command;
}

const std::string* LinkedList::newer() {
    if (cursor_ == nullptr) {
        return nullptr;
    }
    if (cursor_->next == nullptr) {
        cursor_ = nullptr;
        return nullptr;
    }
    cursor_ = cursor_->next;
    return &cursor_->command;
}
