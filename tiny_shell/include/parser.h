#ifndef TINY_SHELL_PARSER_H
#define TINY_SHELL_PARSER_H

#include <string>
#include <vector>

using Command = std::vector<std::string>;
using Pipeline = std::vector<Command>;

class Parser {
public:
    static bool parse(const std::string& input, Pipeline& pipeline, std::string& error);
};

#endif
