# Formal Languages
## Content
- eNFA -> NFA -> DFA
## Prerequisites

- `cmake`
- `graphviz`
- `clang` or `gcc` C++20

## Build

```shell script
$ git submodule update --init
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j
```

## Run

### Main

```shell
$ ./eNFAtoDFA
```

### Tests

```shell
$ ./runUnitTests
```