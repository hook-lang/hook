
[![CI](https://github.com/fabiosvm/hook-lang/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/fabiosvm/hook-lang/actions/workflows/ci.yml)

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
cd bin
hook < ../examples/hello.hook
```

### Displaying the bytecode

```
hook --disasm < ../examples/hello.hook
```
