
[![CI](https://github.com/fabiosvm/hook-lang/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/fabiosvm/hook-lang/actions/workflows/ci.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f2f1345083c1455683dabcf48b0ea6dd)](https://www.codacy.com/gh/fabiosvm/hook-lang/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fabiosvm/hook-lang&amp;utm_campaign=Badge_Grade)
[![Coverage Status](https://coveralls.io/repos/github/fabiosvm/hook-lang/badge.svg?branch=main)](https://coveralls.io/github/fabiosvm/hook-lang?branch=main)

# Hook 

The Hook Programming Language

## Hello, world!

```
echo 'Hello, world!';
```

## Building

```
git clone https://github.com/fabiosvm/hook-lang.git
cd hook-lang
cmake -B build
cmake --build build
```

### Building for debugging

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Building a release

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running an example

```
bin/hook examples/hello.hook
```

### Displaying the bytecode

```
bin/hook --disasm examples/hello.hook
```
