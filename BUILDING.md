
# Build Hook from source code

This project uses the CMake build tool. Use the following commands to build the interpreter.

```
cd ~
git clone https://github.com/fabiosvm/hook-lang.git
mv hook-lang hook && cd hook
cmake -B build
cmake --build build
```

## Setting environment variable 

The interpreter needs the environment variable `HOOK_HOME` in order to import libraries. Furthermore, you might want to run scripts from anywhere in the terminal. 

```
echo "export HOOK_HOME=$HOME/hook" >> ~/.bashrc
echo "export PATH=\$HOOK_HOME/bin:\$PATH" >> ~/.bashrc
```

## Running tests

To run the tests, enter the following:

```
./run-tests
```
