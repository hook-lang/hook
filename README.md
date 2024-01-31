
[![Build and test](https://github.com/hook-lang/hook/actions/workflows/build.yml/badge.svg)](https://github.com/hook-lang/hook/actions/workflows/build.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b58fa787c8cc480091a8e976164ee203)](https://app.codacy.com/gh/hook-lang/hook/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![codecov](https://codecov.io/gh/hook-lang/hook/graph/badge.svg?token=oRSpRBTqp8)](https://codecov.io/gh/hook-lang/hook)

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
curl -sSL https://raw.githubusercontent.com/hook-lang/hook/main/scripts/install.sh | sh
```

For Windows (x64) users, open a terminal and use the following commands:

```
cd %tmp%
curl -sSLO https://raw.githubusercontent.com/hook-lang/hook/main/scripts/install.bat
install
```

### Installing with Homebrew

If you're a macOS user (works on Linux too), you can also install Hook using [Homebrew](https://brew.sh):

```
brew tap hook-lang/hook
brew install hook
```

> **Note**: Follow the instructions provided by Homebrew to add the `HOOK_HOME` environment variable to your system.

### Building from the source code

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
