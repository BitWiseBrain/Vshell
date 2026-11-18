#include "shell.h"

#include "parser.h"

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
class TerminalMode {
public:
    bool enable() {
        if (tcgetattr(STDIN_FILENO, &original_) == -1) {
            perror("tcgetattr");
            return false;
        }
        termios raw = original_;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
            perror("tcsetattr");
            return false;
        }
        active_ = true;
        return true;
    }

    ~TerminalMode() {
        if (active_ && tcsetattr(STDIN_FILENO, TCSANOW, &original_) == -1) {
            perror("tcsetattr");
        }
    }

private:
    termios original_{};
    bool active_ = false;
};

bool readCharacter(char& character) {
    while (true) {
        const ssize_t count = read(STDIN_FILENO, &character, 1);
        if (count == 1) {
            return true;
        }
        if (count == 0) {
            return false;
        }
        if (errno != EINTR) {
            perror("read");
            return false;
        }
    }
}

void waitFor(pid_t process) {
    int status = 0;
    while (waitpid(process, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return;
        }
    }
}

std::string joinArguments(const std::vector<std::string>& arguments,
                          std::size_t first) {
    std::string result;
    for (std::size_t index = first; index < arguments.size(); ++index) {
        if (!result.empty()) {
            result += ' ';
        }
        result += arguments[index];
    }
    return result;
}
}

bool Shell::readLine(std::string& line) {
    line.clear();
    if (!isatty(STDIN_FILENO)) {
        return static_cast<bool>(std::getline(std::cin, line));
    }
    std::cout << prompt_ << "> " << std::flush;

    TerminalMode terminal;
    if (!terminal.enable()) {
        return false;
    }
    history_.startNavigation();
    std::string draft;
    bool navigating = false;
    char character = '\0';
    while (readCharacter(character)) {
        if (character == '\n' || character == '\r') {
            std::cout << '\n';
            return true;
        }
        if (character == 4 && line.empty()) {
            std::cout << '\n';
            return false;
        }
        if (character == 127 || character == '\b') {
            if (!line.empty()) {
                line.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (character == '\x1b') {
            char bracket = '\0';
            char direction = '\0';
            if (!readCharacter(bracket) || !readCharacter(direction) || bracket != '[') {
                continue;
            }
            if (direction == 'A') {
                if (!navigating) {
                    draft = line;
                    navigating = true;
                }
                if (const std::string* older = history_.older()) {
                    line = *older;
                }
            } else if (direction == 'B' && navigating) {
                if (const std::string* newer = history_.newer()) {
                    line = *newer;
                } else {
                    line = draft;
                    navigating = false;
                }
            }
            std::cout << '\r' << prompt_ << "> \033[K" << line << std::flush;
            continue;
        }
        if (std::isprint(static_cast<unsigned char>(character))) {
            line += character;
            std::cout << character << std::flush;
        }
    }
    std::cout << '\n';
    return false;
}

bool Shell::isBuiltin(const std::string& name) const {
    return name == "cd" || name == "exit" || name == "prompt" ||
           name == "history" || name == "alias" || name == "unalias" ||
           name == "pushd" || name == "popd";
}

bool Shell::expandAlias(std::vector<std::string>& arguments) {
    std::set<std::string> seen;
    for (int expansion = 0; expansion < 32; ++expansion) {
        const std::string* value = aliases_.search(arguments.front());
        if (value == nullptr) {
            return true;
        }
        if (!seen.insert(arguments.front()).second) {
            std::cerr << "alias: recursive alias expansion\n";
            return false;
        }
        Pipeline replacement;
        std::string error;
        if (!Parser::parse(*value, replacement, error) ||
            replacement.size() != 1 || replacement.front().empty()) {
            std::cerr << "alias: invalid alias definition: " << error << '\n';
            return false;
        }
        Command expanded = replacement.front();
        expanded.insert(expanded.end(), arguments.begin() + 1, arguments.end());
        arguments.swap(expanded);
    }
    if (aliases_.search(arguments.front()) == nullptr) {
        return true;
    }
    std::cerr << "alias: expansion limit exceeded\n";
    return false;
}

bool Shell::executeBuiltin(const std::vector<std::string>& arguments) {
    const std::string& name = arguments.front();
    if (name == "exit") {
        return false;
    }
    if (name == "cd") {
        const char* destination = nullptr;
        if (arguments.size() == 1) {
            destination = std::getenv("HOME");
            if (destination == nullptr) {
                std::cerr << "cd: HOME is not set\n";
                return true;
            }
        } else if (arguments.size() == 2) {
            destination = arguments[1].c_str();
        } else {
            std::cerr << "cd: expected at most one directory\n";
            return true;
        }
        if (chdir(destination) == -1) {
            perror("cd");
        }
        return true;
    }
    if (name == "prompt") {
        if (arguments.size() == 1) {
            std::cout << prompt_ << '\n';
        } else {
            prompt_ = joinArguments(arguments, 1);
        }
        return true;
    }
    if (name == "history") {
        history_.display();
        return true;
    }
    if (name == "alias") {
        if (arguments.size() == 1) {
            for (const auto& entry : aliases_.entries()) {
                std::cout << "alias " << entry.first << "='" << entry.second << "'\n";
            }
        } else if (arguments.size() == 2) {
            const std::string* value = aliases_.search(arguments[1]);
            if (value == nullptr) {
                std::cerr << "alias: " << arguments[1] << ": not found\n";
            } else {
                std::cout << "alias " << arguments[1] << "='" << *value << "'\n";
            }
        } else {
            const std::string value = joinArguments(arguments, 2);
            if (arguments[1].empty() || value.empty()) {
                std::cerr << "alias: expected a name and a command\n";
            } else {
                aliases_.insert(arguments[1], value);
            }
        }
        return true;
    }
    if (name == "unalias") {
        if (arguments.size() != 2) {
            std::cerr << "unalias: expected one alias name\n";
        } else if (!aliases_.erase(arguments[1])) {
            std::cerr << "unalias: " << arguments[1] << ": not found\n";
        }
        return true;
    }
    if (name == "pushd") {
        if (arguments.size() != 2) {
            std::cerr << "pushd: expected one directory\n";
            return true;
        }
        char* current = getcwd(nullptr, 0);
        if (current == nullptr) {
            perror("pushd");
            return true;
        }
        if (chdir(arguments[1].c_str()) == -1) {
            perror("pushd");
            free(current);
            return true;
        }
        directories_.push(current);
        free(current);
        return true;
    }
    if (name == "popd") {
        const std::string* directory = directories_.top();
        if (directory == nullptr) {
            std::cerr << "popd: directory stack is empty\n";
        } else if (chdir(directory->c_str()) == -1) {
            perror("popd");
        } else {
            directories_.pop();
        }
        return true;
    }
    return true;
}

bool Shell::runExternal(const std::vector<std::vector<std::string>>& commands) {
    int pipeDescriptors[2] = {-1, -1};
    if (commands.size() == 2 && pipe(pipeDescriptors) == -1) {
        perror("pipe");
        return true;
    }

    pid_t first = fork();
    if (first == -1) {
        perror("fork");
        if (pipeDescriptors[0] != -1) {
            close(pipeDescriptors[0]);
            close(pipeDescriptors[1]);
        }
        return true;
    }
    if (first == 0) {
        if (commands.size() == 2) {
            if (dup2(pipeDescriptors[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                _exit(126);
            }
            close(pipeDescriptors[0]);
            close(pipeDescriptors[1]);
        }
        std::vector<char*> arguments;
        for (const std::string& argument : commands[0]) {
            arguments.push_back(const_cast<char*>(argument.c_str()));
        }
        arguments.push_back(nullptr);
        execvp(arguments[0], arguments.data());
        perror(arguments[0]);
        _exit(127);
    }

    pid_t second = -1;
    if (commands.size() == 2) {
        second = fork();
        if (second == -1) {
            perror("fork");
            close(pipeDescriptors[0]);
            close(pipeDescriptors[1]);
            waitFor(first);
            return true;
        }
        if (second == 0) {
            if (dup2(pipeDescriptors[0], STDIN_FILENO) == -1) {
                perror("dup2");
                _exit(126);
            }
            close(pipeDescriptors[0]);
            close(pipeDescriptors[1]);
            std::vector<char*> arguments;
            for (const std::string& argument : commands[1]) {
                arguments.push_back(const_cast<char*>(argument.c_str()));
            }
            arguments.push_back(nullptr);
            execvp(arguments[0], arguments.data());
            perror(arguments[0]);
            _exit(127);
        }
        close(pipeDescriptors[0]);
        close(pipeDescriptors[1]);
    }

    waitFor(first);
    if (second != -1) {
        waitFor(second);
    }
    return true;
}

bool Shell::execute(const std::string& line) {
    Pipeline pipeline;
    std::string error;
    if (!Parser::parse(line, pipeline, error)) {
        std::cerr << "syntax error: " << error << '\n';
        return true;
    }
    if (pipeline.empty()) {
        return true;
    }
    history_.append(line);

    for (Command& command : pipeline) {
        if (!expandAlias(command)) {
            return true;
        }
    }

    if (pipeline.size() == 1 && isBuiltin(pipeline[0][0])) {
        return executeBuiltin(pipeline[0]);
    }
    if (pipeline.size() == 2 &&
        (isBuiltin(pipeline[0][0]) || isBuiltin(pipeline[1][0]))) {
        std::cerr << "built-in commands cannot be used in a pipeline\n";
        return true;
    }
    return runExternal(pipeline);
}

int Shell::run() {
    std::string line;
    while (readLine(line)) {
        if (!execute(line)) {
            return 0;
        }
    }
    return 0;
}
