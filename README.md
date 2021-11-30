
[![Build and test](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml/badge.svg)](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f2f1345083c1455683dabcf48b0ea6dd)](https://www.codacy.com/gh/fabiosvm/hook-lang/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fabiosvm/hook-lang&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/fabiosvm/hook-lang/branch/main/graph/badge.svg?token=mkmMpfS1yu)](https://codecov.io/gh/fabiosvm/hook-lang)

# The Hook Programming Language

Hook is a simple, small, cross-plataform scripting language with an easy-to-understand implementation. It has been written entirely by hand in C99 using the most simple techniques like recursive descending parsing, stack-based virtual machine, reference counting, etc.

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

* Imperative programming
* C-style
* Dynamic-type
* No accidental side effects
* Single-pass compiler
* Automatic memory management

Warning: Hook is still in its early stages of development and is obviously not production ready. 

## Installing

To install Hook on Linux, Windows, and macOS, download a prebuilt version from the [GitHub release page](https://github.com/fabiosvm/hook-lang/releases).

```
cd ~
wget https://github.com/fabiosvm/hook-lang/releases/download/0.1.0-pre/hook-0.1.0-linux-x64.tar.gz
tar -xvf hook-0.1.0-linux-x64.tar.gz
mv hook-0.1.0-linux-x64 hook
```

### Setting environment variable 

The interpreter needs the environment variable `HOOK_HOME` in order to import libraries. Furthermore,
you might want to run scripts from anywhere in the terminal. 

```
echo "export HOOK_HOME=$HOME/hook" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc
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

## Running an example

In the `example` folder you will find some code samples.

```
hook example/hello.hook
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
