
[![Build and test](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml/badge.svg)](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f2f1345083c1455683dabcf48b0ea6dd)](https://www.codacy.com/gh/fabiosvm/hook-lang/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fabiosvm/hook-lang&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/fabiosvm/hook-lang/branch/main/graph/badge.svg?token=mkmMpfS1yu)](https://codecov.io/gh/fabiosvm/hook-lang)

# The Hook Programming Language

Hook is a simple, small, cross-platform, dynamically typed scripting language that combines imperative and functional programming. It was written entirely by hand in C using easy-to-understand techniques like recursive descent parsing, stack-based virtual machine, reference counting, etc.

## What does it look like 

```rust
fn factorial(n) {
  if (n == 0)
    return 1;
  return n * factorial(n - 1);
}
```

## Features

The main features are:

* Cross-platform
* Dynamically typed
* Imperative and functional programming
* C-style syntax
* No side effects
* Value-captured closure
* Automatic memory management

Warning: Hook is still in its early stages of development and is obviously not production ready. 

## Installing

For Linux (x64, arm64), and macOS (x64), use the following command to install Hook: 

```
curl -sSL https://github.com/fabiosvm/hook-lang/releases/download/0.1.0/install.sh | bash
```

### Building from source code

This project uses the CMake build tool. Use the following commands to build the interpreter.

```
cd ~
git clone https://github.com/fabiosvm/hook-lang.git
mv hook-lang hook && cd hook
cmake -B build
cmake --build build
```

### Setting environment variable 

The interpreter needs the environment variable `HOOK_HOME` in order to import libraries. Furthermore, you might want to run scripts from anywhere in the terminal. 

```
echo "export HOOK_HOME=$HOME/hook" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc
```

## Running an example

In the `example` folder you will find some code samples.

```
hook /opt/hook/example/hello.hook
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
