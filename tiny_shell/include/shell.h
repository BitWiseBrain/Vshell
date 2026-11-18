#ifndef TINY_SHELL_SHELL_H
#define TINY_SHELL_SHELL_H

#include "directorystack.h"
#include "hashtable.h"
#include "linkedlist.h"

#include <string>
#include <vector>

class Shell {
public:
    int run();

private:
    bool readLine(std::string& line);
    bool execute(const std::string& line);
    bool executeBuiltin(const std::vector<std::string>& arguments);
    bool expandAlias(std::vector<std::string>& arguments);
    bool runExternal(const std::vector<std::vector<std::string>>& commands);
    bool isBuiltin(const std::string& name) const;
    std::string prompt_ = "vshell";
    LinkedList history_;
    HashTable aliases_;
    DirectoryStack directories_;
};

#endif
