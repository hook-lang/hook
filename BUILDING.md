
# Build Hook from source code

This project is written in C and uses the CMake build tool. Therefore, you may need to install 
this tool on your system.

Note that the following instructions are for the Ubuntu operating system, but the project is cross-platform, 
so you can compile it for Linux, macOS, and Windows.

## Installing development tools

Enter the following commands in the terminal to install CMake:

```
sudo apt-get update -y
sudo apt-get install -y cmake
```

## Building and compiling

Use the following commands to build the binaries:

```
cd ~
git clone https://github.com/hook-lang/hook.git
cd hook
scripts/build.sh
```

### Building the extensions

Hook's project brings some extensions (non-core built-in modules) that are not compiled by default.

To build the extensions, you need to install some dependencies:

```
sudo apt-get install -y libsqlite3-dev libcurl4-openssl-dev libmysqlclient-dev libgmp3-dev libzmq3-dev libleveldb-dev libpcre3-dev
```

So you can build the extensions with the following command:

```
scripts/build.sh Debug with-extensions
```

### Building for Release

To build the project for release, you can use the following command:

```
scripts/build.sh Release
```

## Setting environment variable 

The interpreter needs the environment variable `HOOK_HOME` in order to import modules.
Furthermore, you might want to run scripts from anywhere in the terminal.

```
echo "export HOOK_HOME=$HOME/hook" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc
```

## Running tests

To run the tests, enter the following:

```
scripts/test.sh
```
