#ifndef TINY_SHELL_DIRECTORYSTACK_H
#define TINY_SHELL_DIRECTORYSTACK_H

#include <string>
#include <utility>

class DirectoryStack {
public:
    DirectoryStack() = default;
    ~DirectoryStack();

    DirectoryStack(const DirectoryStack&) = delete;
    DirectoryStack& operator=(const DirectoryStack&) = delete;

    void push(const std::string& directory);
    const std::string* top() const;
    void pop();
    bool empty() const { return top_ == nullptr; }

private:
    struct Node {
        explicit Node(std::string value) : directory(std::move(value)) {}
        std::string directory;
        Node* next = nullptr;
    };
    Node* top_ = nullptr;
};

#endif
