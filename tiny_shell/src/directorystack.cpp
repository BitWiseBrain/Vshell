#include "directorystack.h"

DirectoryStack::~DirectoryStack() {
    while (top_ != nullptr) {
        pop();
    }
}

void DirectoryStack::push(const std::string& directory) {
    Node* node = new Node(directory);
    node->next = top_;
    top_ = node;
}

const std::string* DirectoryStack::top() const {
    return top_ == nullptr ? nullptr : &top_->directory;
}

void DirectoryStack::pop() {
    if (top_ == nullptr) {
        return;
    }
    Node* removed = top_;
    top_ = top_->next;
    delete removed;
}
