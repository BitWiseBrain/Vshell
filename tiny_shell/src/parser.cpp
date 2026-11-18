#include "parser.h"

namespace {
enum class Quote { None, Single, Double };
}

bool Parser::parse(const std::string& input, Pipeline& pipeline, std::string& error) {
    pipeline.clear();
    error.clear();

    Command command;
    std::string token;
    Quote quote = Quote::None;
    bool tokenStarted = false;
    bool escaped = false;

    auto finishToken = [&]() {
        if (tokenStarted) {
            command.push_back(token);
            token.clear();
            tokenStarted = false;
        }
    };
    auto finishCommand = [&]() {
        finishToken();
        if (command.empty()) {
            return false;
        }
        pipeline.push_back(command);
        command.clear();
        return true;
    };

    for (char character : input) {
        if (escaped) {
            token += character;
            tokenStarted = true;
            escaped = false;
            continue;
        }
        if (quote != Quote::Single && character == '\\') {
            escaped = true;
            tokenStarted = true;
            continue;
        }
        if (quote == Quote::None && character == '\'') {
            quote = Quote::Single;
            tokenStarted = true;
        } else if (quote == Quote::Single && character == '\'') {
            quote = Quote::None;
        } else if (quote == Quote::None && character == '"') {
            quote = Quote::Double;
            tokenStarted = true;
        } else if (quote == Quote::Double && character == '"') {
            quote = Quote::None;
        } else if (quote == Quote::None && character == '|') {
            if (!finishCommand()) {
                error = "pipe requires a command on both sides";
                return false;
            }
        } else if (quote == Quote::None && (character == ' ' || character == '\t')) {
            finishToken();
        } else {
            token += character;
            tokenStarted = true;
        }
    }

    if (escaped) {
        error = "trailing escape character";
        return false;
    }
    if (quote != Quote::None) {
        error = "unterminated quote";
        return false;
    }
    if (!command.empty() || tokenStarted) {
        if (!finishCommand()) {
            error = "pipe requires a command on both sides";
            return false;
        }
    } else if (!pipeline.empty()) {
        error = "pipe requires a command on both sides";
        return false;
    }

    if (pipeline.size() > 2) {
        error = "only one pipe (two commands) is supported";
        return false;
    }
    return true;
}
