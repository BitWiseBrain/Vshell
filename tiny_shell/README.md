# VShell

## Run

Requirements: Linux, GNU Make, and a C++17 compiler.

```sh
make
./build/vshell
```

## Features

- Built-in `cd`, `exit`, `prompt`, `history`, `alias`, `unalias`, `pushd`, and `popd` commands.
- Run external commands using `fork`, `execvp`, and `waitpid`.
- Support one pipeline between external commands.
- Navigate command history with the Up/Down keys in a terminal.
- Implement history, aliases, and the directory stack using custom data structures.
