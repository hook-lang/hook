
[![Build and test](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml/badge.svg)](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f2f1345083c1455683dabcf48b0ea6dd)](https://www.codacy.com/gh/fabiosvm/hook-lang/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fabiosvm/hook-lang&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/fabiosvm/hook-lang/branch/main/graph/badge.svg?token=mkmMpfS1yu)](https://codecov.io/gh/fabiosvm/hook-lang)

# The Hook Programming Language

Hook is an imperative, cross-platform, dynamically typed scripting language based on the mutable value semantics approach. It was written entirely by hand in C using easy-to-understand techniques like recursive descent parsing, stack-based virtual machine, and reference counting.

## What does it look like

Hook has a modern C-like syntax. See:

```rust
fn factorial(n) {
  if (n == 0)
    return 1;
  return n * factorial(n - 1);
}
```

## Features

The main features are:

* Imperative
* Cross-platform
* Dynamically typed
* Mutable value semantics approach
* C-like syntax
* Value-captured closure

**Warning**: Hook is still in its early stages of development and is not production-ready.

## Installing

For Linux (x64, arm64), and macOS (x64), use the following command to install Hook:

```
curl -sSL https://github.com/fabiosvm/hook-lang/releases/download/0.1.0/install.sh | sh
```

For Windows (x64), open a cmd prompt and use: 

```
cd %tmp%
curl -sSLO https://github.com/fabiosvm/hook-lang/releases/download/0.1.0/install.bat
install
```

## Hello, world!

It's possible to run our `Hello, world!` directly from the command-line interface:

```
hook -e 'println("Hello, world!");'
```

## License

Hook is released under [MIT](https://choosealicense.com/licenses/mit/) license.
See [LICENSE](https://github.com/fabiosvm/hook/blob/main/LICENSE) for more details.
