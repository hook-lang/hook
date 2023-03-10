
[![Build and test](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml/badge.svg)](https://github.com/fabiosvm/hook-lang/actions/workflows/build.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f2f1345083c1455683dabcf48b0ea6dd)](https://www.codacy.com/gh/fabiosvm/hook-lang/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fabiosvm/hook-lang&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/fabiosvm/hook-lang/branch/main/graph/badge.svg?token=mkmMpfS1yu)](https://codecov.io/gh/fabiosvm/hook-lang)

# The Hook Programming Language

Hook is a simple, cross-platform, dynamically typed scripting language that utilizes a mutable value semantics approach. It was developed from scratch using C and employs clear and simple techniques such as recursive descent parsing, stack-based virtual machine, and reference counting.

> **Note**: Hook is currently in the early stages of development and should not be used in production environments at this time.

## What does it look like

Hook features a modern syntax similar to C. For examples:

```rust
fn factorial(n) {
  if (n == 0)
    return 1;
  return n * factorial(n - 1);
}
```

## Features

Hook offers the following key features:

* Imperative and functional programming paradigms
* Cross-platform compatibility
* Dynamic typing
* Mutable value semantics approach
* Familiar C-like syntax
* Support for value-captured closures

## Installing

The following command can be used to install Hook on Linux (x64) and macOS (x64, arm64) systems:

```
curl -sSL https://github.com/fabiosvm/hook-lang/releases/download/0.1.0/install.sh | sh
```

For Windows (x64) users, open a terminal and use the following commands:

```
cd %tmp%
curl -sSLO https://github.com/fabiosvm/hook-lang/releases/download/0.1.0/install.bat
install
```

## Building from the source code

If you prefer, you can also build Hook locally by following the instructions provided in [BUILDING.md](BUILDING.md).

## Hello, world!

You can run the classic `Hello, World!` program directly from the terminal by using the following command:

```
hook -e 'println("Hello, world!");'
```

## Documentation

Unfortunately, as the language is undergoing constant updates and changes to its lexemes and syntax, documentation is not currently available. The `examples` folder may provide some guidance in the meantime.

## Play with Hook online

Get hands-on with Hook by visiting the [Hook Playground](https://hook-lang.github.io/hook-playground). You can easily write and run Hook code directly from your browser, whether it be from the provided examples or the code you've been itching to try.

## Editor Support

Hook offers an extension for the popular code editor, Visual Studio Code. This extension provides features such as syntax highlighting, code completion, and code snippets, making your development experience more efficient. The extension can be easily installed from the  [Visual Studio Marketplace](https://marketplace.visualstudio.com/items?itemName=fabiosvm.hook).

## Contributing

We welcome contributions to our project! Please refer to the guidelines in our [CONTRIBUTING](CONTRIBUTING.md) file to learn how you can help us improve our project. Thank you for your interest in contributing!

## License

Hook is licensed under the [MIT](https://choosealicense.com/licenses/mit) license. For more information, please refer to the [LICENSE](LICENSE) file.
