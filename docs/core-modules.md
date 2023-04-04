
# Hook's core modules

Hook's language comes with a set of core modules that serve as the building blocks for scripts and other modules. These modules form the foundation of Hook's language and provide essential functionalities that enable you to write powerful and efficient code.

## Importing modules

Before jumping into the documentation, it's important to remember how to import modules in Hook.

To use a core module in your script, you must first import it. You can import a module by using the `import` keyword followed by the module name. For example, to import the `math` module, you would write:

```js
import math;
```

Once you have imported the module, you can use its functions, data types, and constants in your code by prefixing them with the module name, followed by a `.`. For example, to use the `abs` function, you would write:

```js
println(math.abs(-5)); // 5
```

Also, it is possible to import only specific elements from a module. For example, to import only the `abs` function, you would write:

```js
import { abs } from math;

println(abs(-5)); // 5
```

## List of modules

Below is a comprehensive list of all the modules available. Click on any module name to quickly access its corresponding documentation.

<table>
  <tbody>
    <tr>
      <td><a href="#math">math</a></td>
      <td><a href="#os">os</a></td>
      <td><a href="#io">io</a></td>
      <td><a href="#numbers">numbers</a></td>
      <td><a href="#strings">strings</a></td>
    </tr>
    <tr>
      <td><a href="#arrays">arrays</a></td>
      <td><a href="#utf8">utf8</a></td>
      <td><a href="#regex">regex</a></td>
      <td><a href="#hashing">hashing</a></td>
      <td><a href="#encoding">encoding</a></td>
    </tr>
    <tr>
      <td><a href="#socket">socket</a></td>
      <td><a href="#json">json</a></td>
      <td><a href="#lists">lists</a></td>
      <td></td>
      <td></td>
    </tr>
  </tbody>
</table>

Below you will find all modules with their public functions and constants.

### math

The `math` module provides a wide range of functions for mathematical calculations, including basic arithmetic operations and more advanced functions such as trigonometric and logarithmic operations.

<table>
  <tbody>
    <tr>
      <td><a href="#abs">abs</a></td>
      <td><a href="#sin">sin</a></td>
      <td><a href="#cos">cos</a></td>
      <td><a href="#tan">tan</a></td>
      <td><a href="#asin">asin</a></td>
      <td><a href="#acos">acos</a></td>
    </tr>
    <tr>
      <td><a href="#atan">atan</a></td>
      <td><a href="#floor">floor</a></td>
      <td><a href="#ceil">ceil</a></td>
      <td><a href="#round">round</a></td>
      <td><a href="#pow">pow</a></td>
      <td><a href="#sqrt">sqrt</a></td>
    </tr>
    <tr>
      <td><a href="#cbrt">cbrt</a></td>
      <td><a href="#log">log</a></td>
      <td><a href="#log2">log2</a></td>
      <td><a href="#log10">log10</a></td>
      <td><a href="#exp">exp</a></td>
      <td></td>
    </tr>
  </tbody>
</table>

#### abs

Returns the absolute value of a number.

```rust
fn abs(num: number) -> number;
```

Example:

```rust
println(math.abs(-10)); // 10
println(math.abs(10));  // 10
```

#### sin

Returns the sine of a number.

```rust
fn sin(num: number) -> number;
```

Example:

```rust
println(math.sin(0)); // 0
println(math.sin(1)); // 0.841471
```

#### cos

Returns the cosine of a number.

```rust
fn cos(num: number) -> number;
```

Example:

```rust
println(math.cos(0)); // 1
println(math.cos(1)); // 0.540302
```

#### tan

Returns the tangent of a number.

```rust
fn tan(num: number) -> number;
```

Example:

```rust
println(math.tan(0)); // 0
println(math.tan(1)); // 1.55741
```

#### asin

Returns the arc sine of a number.

```rust
fn asin(num: number) -> number;
```

Example:

```rust
println(math.asin(0)); // 0
println(math.asin(1)); // 1.5708
```

#### acos

Returns the arc cosine of a number.

```rust
fn acos(num: number) -> number;
```

Example:

```rust
println(math.acos(0)); // 1.5708
println(math.acos(1)); // 0
```

#### atan

Returns the arc tangent of a number.

```rust
fn atan(num: number) -> number;
```

Example:

```rust
println(math.atan(0)); // 0
println(math.atan(1)); // 0.785398
```

#### floor

Returns the largest integer less than or equal to a number.

```rust
fn floor(num: number) -> number;
```

Example:

```rust
println(math.floor(0.5)); // 0
println(math.floor(1.5)); // 1
```

#### ceil

Returns the smallest integer greater than or equal to a number.

```rust
fn ceil(num: number) -> number;
```

Example:

```rust
println(math.ceil(0.5)); // 1
println(math.ceil(1.5)); // 2
```

#### round

Returns the nearest integer to a number.

```rust
fn round(num: number) -> number;
```

Example:

```rust
println(math.round(0.4)); // 0
println(math.round(0.5)); // 1
println(math.round(0.6)); // 1
```

#### pow

Returns the value of a number raised to a power.

```rust
fn pow(num: number, power: number) -> number;
```

Example:

```rust
println(math.pow(2, 2)); // 4
println(math.pow(2, 3)); // 8
```

#### sqrt

Returns the square root of a number.

```rust
fn sqrt(num: number) -> number;
```

Example:

```rust
println(math.sqrt(4));  // 2
println(math.sqrt(9));  // 3
println(math.sqrt(10)); // 3.16228
```

#### cbrt

Returns the cube root of a number.

```rust
fn cbrt(num: number) -> number;
```

Example:

```rust
println(math.cbrt(8));  // 2
println(math.cbrt(27)); // 3
println(math.cbrt(28)); // 3.03659
```

#### log

Returns the natural logarithm of a number.

```rust
fn log(num: number) -> number;
```

Example:

```rust
println(math.log(1));  // 0
println(math.log(2));  // 0.693147
println(math.log(10)); // 2.30259
```

#### log2

Returns the base 2 logarithm of a number.

```rust
fn log2(num: number) -> number;
```

Example:

```rust
println(math.log2(1));  // 0
println(math.log2(2));  // 1
println(math.log2(10)); // 3.32193
```

#### log10

Returns the base 10 logarithm of a number.

```rust
fn log10(num: number) -> number;
```

Example:

```rust
println(math.log10(1));  // 0
println(math.log10(2));  // 0.30103
println(math.log10(10)); // 1
```

#### exp

Returns the value of `e` constant raised to a power.

```rust
fn exp(num: number) -> number;
```

Example:

```rust
println(math.exp(0));  // 1
println(math.exp(1));  // 2.71828
println(math.exp(10)); // 22026.5
```

### os

The `os` module provides a variety of functions and constants that allow you to interact with the operating system. For example, you can use this module to access system information like environment variables.

<table>
  <tbody>
    <tr>
      <td><a href="#clocks_per_sec">CLOCKS_PER_SEC</a></td>
      <td><a href="#clock">clock</a></td>
      <td><a href="#time">time</a></td>
      <td><a href="#system">system</a></td>
    <tr>
      <td><a href="#getenv">getenv</a></td>
      <td><a href="#getcwd">getcwd</a></td>
      <td><a href="#name">name</a></td>
      <td></td>
    </tr>
  </tbody>
</table>

#### CLOCKS_PER_SEC

The number of clock ticks per second.

```rust
let CLOCKS_PER_SEC: number;
```

Example:

```rust
println(os.CLOCKS_PER_SEC); // 1e+06
```

#### clock

Returns the time in seconds since the start of the program.

```rust
fn clock() -> number;
```

Example:

```rust
println(os.clock()); // 0.007073
```

#### time

Returns the current time in seconds since the Unix epoch.

```rust
fn time() -> number;
```

Example:

```rust
println(os.time()); // 1.67683e+09
```

#### system

Executes a command in the operating system shell and returns the exit code.

```rust
fn system(command: string) -> number;
```

Example:

```rust
os.system("echo hello"); // In *nix systems, this will print "hello" to the terminal.
```

#### getenv

Returns the value of an environment variable.

```rust
fn getenv(str: string) -> string;
```

Example:

```rust
println(os.getenv("HOME")); // /home/username
```

#### getcwd

Returns the current working directory.

```rust
fn getcwd() -> string;
```

Example:

```rust
println(os.getcwd()); // /home/username
```

#### name

Returns the name of the operating system. Returns `linux`, `macos`, `windows`, or `unknown`.

```rust
fn name() -> string;
```

Example:

```rust
println(os.name()); // linux
```

### io

The `io` module provides various facilities for handling I/O operations. It enables you to read from and write to different streams, such as files, and provides tools for working with binary and text files.

<table>
  <tbody>
    <tr>
      <td><a href="#stdin-stdout-stderr">stdin</a></td>
      <td><a href="#stdin-stdout-stderr">stdout</a></td>
      <td><a href="#stdin-stdout-stderr">stderr</a></td>
      <td><a href="#seek_set-seek_cur-seek_end">SEEK_SET</a></td>
      <td><a href="#seek_set-seek_cur-seek_end">SEEK_CUR</a></td>
    </tr>
    <tr>
      <td><a href="#seek_set-seek_cur-seek_end">SEEK_END</a></td>
      <td><a href="#open">open</a></td>
      <td><a href="#close">close</a></td>
      <td><a href="#popen">popen</a></td>
      <td><a href="#pclose">pclose</a></td>
    </tr>
    <tr>
      <td><a href="#eof">eof</a></td>
      <td><a href="#flush">flush</a></td>
      <td><a href="#sync">sync</a></td>
      <td><a href="#tell">tell</a></td>
      <td><a href="#rewind">rewind</a></td>
    </tr>
    <tr>
      <td><a href="#seek">seek</a></td>
      <td><a href="#read">read</a></td>
      <td><a href="#write">write</a></td>
      <td><a href="#readln">readln</a></td>
      <td><a href="#writeln">writeln</a></td>
    </tr>
  </tbody>
</table>

#### stdin, stdout, stderr

The `stdin`, `stdout`, and `stderr` constants are used to access the standard input, output, and error streams.

```rust
let stdin: userdata;
let stdout: userdata;
let stderr: userdata;
```

Example:

```rust
println(io.readln(io.stdin));           // Read a line from stdin and print it.
io.writeln(io.stdout, "Hello, world!"); // Hello, world!
io.writeln(io.stderr, "Error!");        // Error!
```

#### SEEK_SET, SEEK_CUR, SEEK_END

The `SEEK_SET`, `SEEK_CUR`, and `SEEK_END` constants are used to specify the origin when seeking in a file. See the [seek](#seek) function for more information.

```rust
let SEEK_SET: number;
let SEEK_CUR: number;
let SEEK_END: number;
```

Example:

```rust
let stream = io.open("file.txt", "r");
io.seek(stream, 0, io.SEEK_SET); // Seek to the beginning of the file.
io.seek(stream, 0, io.SEEK_CUR); // Seek to the current position.
io.seek(stream, 0, io.SEEK_END); // Seek to the end of the file.
```

#### open

Opens a file and returns a file handle.

```rust
fn open(filename: string, mode: string) -> userdata;
```

The `filename` argument is the name of the file to open, and the `mode` argument is a string that specifies the mode in which the file is opened.

The following table lists the possible values of the `mode` argument:

- `"r"`: Open file for reading. The file must exist.
- `"w"`: Truncate file to zero length or create file for writing.
- `"a"`: Append; open or create file for writing at end-of-file.
- `"r+"`: Open file for update (reading and writing).
- `"w+"`: Truncate file to zero length or create file for update.
- `"a+"`: Append; open or create file for update, writing at end-of-file.

Example:

```rust
let stream = io.open("file.txt", "r"); // Open file.txt for reading.
```

#### close

Closes a file.

```rust
fn close(stream: userdata);
```

Example:

```rust
let stream = io.open("file.txt", "r");
io.close(stream);
```

#### popen

Opens a process by creating a pipe, forking, and invoking the shell.

```rust
fn popen(command: string, mode: string) -> userdata;
```

The `command` argument is the command to execute, and the `mode` argument is the same as the `mode` argument of the [open](#open) function.

Example:

```rust
let stream = io.popen("ls", "r"); // Execute the "ls" command for reading.
```

#### pclose

Closes a process.

```rust
let stream = io.popen("ls", "r");
io.pclose(stream);
```

#### eof

Checks whether the end-of-file indicator is set for the given stream.

```rust
fn eof(stream: userdata) -> bool;
```

Example:

```rust
let stream = io.open("file.txt", "r");
while (!io.eof(stream)) {
  println(io.readln(stream));
}
```

#### flush

Flushes the output buffer of the given stream.

```rust
fn flush(stream: userdata);
```

Example:

```rust
io.writeln(io.stdout, "Hello, world!");
io.flush(io.stdout);
```

#### sync

Flushes and synchronizes the output buffer of the given stream.

```rust
fn sync(stream: userdata);
```

Example:

```rust
io.writeln(io.stdout, "Hello, world!");
io.sync(io.stdout);
```

#### tell

Returns the current position of the file pointer for the given stream.

```rust
fn tell(stream: userdata) -> number;
```

Example:

```rust
let stream = io.open("file.txt", "r");
println(io.tell(stream));              // 0
io.readln(stream);
println(io.tell(stream));              // 6
```

#### rewind

Rewinds the file pointer to the beginning of the given stream.

```rust
fn rewind(stream: userdata);
```

Example:

```rust
let stream = io.open("file.txt", "r");
io.readln(stream);
io.rewind(stream);
println(io.tell(stream));              // 0
```

#### seek

Sets the position of the file pointer for the given stream.

```rust
fn seek(stream: userdata, offset: number, whence: number);
```

The `offset` argument is the number of bytes to offset from the position specified by `whence`. The `whence` argument is an integer that determines the reference position from which to calculate the new position. It can be one of the following constants:

- `io.SEEK_SET`: The beginning of the file.
- `io.SEEK_CUR`: The current position of the file pointer.
- `io.SEEK_END`: The end of the file.

Example:

```rust
let stream = io.open("file.txt", "w");
io.seek(stream, 0, io.SEEK_END);       // Seek to the end of the file.
io.writeln(stream, "Hello, world!");
```

#### read

Reads data from the given stream and returns it as a string.

```rust
fn read(stream: userdata, size: number) -> string;
```

The `size` argument is the number of bytes to read.

Example:

```rust
let stream = io.open("file.txt", "r");
println(io.read(stream, 13));          // Hello, world!
```

#### write

Writes the given string to the given stream.

```rust
fn write(stream: userdata, data: string);
```

Example:

```rust
let stream = io.open("file.txt", "w");
io.write(stream, "Hello, world!");
```

#### readln

Reads data from the given stream until a newline character is encountered and returns it as a string.

```rust
fn readln(stream: userdata) -> string;
```

Example:

```rust
let stream = io.open("file.txt", "r");
println(io.readln(stream));            // Hello, world!
```

#### writeln

Writes the given string to the given stream, followed by a newline character.

```rust
fn writeln(stream: userdata, data: string);
```

Example:

```rust
let stream = io.open("file.txt", "w");
io.writeln(stream, "Hello, world!");   // Appends a newline character at the end.
```

### numbers

The `numbers` module provides mathematical constants and limits, such as the maximum and minimum representable integers.

<table>
  <tbody>
    <tr>
      <td><a href="#pi-tau">PI</a></td>
      <td><a href="#pi-tau">TAU</a></td>
      <td><a href="#largest-smallest">LARGEST</a></td>
      <td><a href="#largest-smallest">SMALLEST</a></td>
    </tr>
    <tr>
      <td><a href="#max_integer-min_integer">MAX_INTEGER</a></td>
      <td><a href="#max_integer-min_integer">MIN_INTEGER</a></td>
      <td><a href="#srand">srand</a></td>
      <td><a href="#rand">rand</a></td>
    </tr>
  </tbody>
</table>

#### PI, TAU

The mathematical constant `π` and `τ` (2π).

```rust
let PI: number;
let TAU: number;
```

Example:

```rust
println(numbers.PI);  // 3.14159
println(numbers.TAU); // 6.28319
```

#### LARGEST, SMALLEST

The largest and smallest representable numbers.

```rust

let LARGEST: number;
let SMALLEST: number;
```

Example:

```rust
println(numbers.LARGEST);  // 1.79769e+308
println(numbers.SMALLEST); // 2.22507e-308
```

#### MAX_INTEGER, MIN_INTEGER

The largest and smallest safely representable integers.

```rust
let MAX_INTEGER: number;
let MIN_INTEGER: number;
```

Example:

```rust
println(numbers.MAX_INTEGER); // 9.0072e+15
println(numbers.MIN_INTEGER); // -9.0072e+15
```

#### srand

Sets the seed for the random number generator.

```rust
fn srand(seed: number);
```

Example:

```rust
numbers.srand(123);
println(numbers.rand());
```

#### rand

Returns a random number between 0 and 1.

```rust
fn rand() -> number;
```

Example:

```rust
println(numbers.rand());
println(numbers.rand());
```

### strings

The `strings` module provides functions for working with strings.

<table>
  <tbody>
    <tr>
      <td><a href="#new_string">new_string</a></td>
      <td><a href="#repeat">repeat</a></td>
      <td><a href="#hash">hash</a></td>
      <td><a href="#lower">lower</a></td>
      <td><a href="#upper">upper</a></td>
    </tr>
    <tr>
      <td><a href="#trim">trim</a></td>
      <td><a href="#starts_with">starts_with</a></td>
      <td><a href="#ends_with">ends_with</a></td>
      <td><a href="#reverse">reverse</a></td>
      <td></td>
    </tr>
  </tbody>
</table>

#### new_string

Creates a new string with the given minimum capacity.

> **Note:** When setting the initial capacity of a string, it will be adjusted to the next power of two. As a result, the actual capacity of the string may be larger than the specified value.

```rust
fn new_string(min_capacity: number) -> string;
```

Example:

```rust
let str = strings.new_string(10);
str += "Hello, world!";
println(str);                     // Hello, world!
```

#### repeat

Returns a new string that is a copy of the given string repeated the given number of times.

```rust
fn repeat(str: string, count: number) -> string;
```

Example:

```rust
let str = strings.repeat("foo", 3);
println(str);                       // foofoofoo
```

#### hash

Returns the hash of the given string.

```rust
fn hash(str: string) -> number;
```

Example:

```rust
println(strings.hash("Hello, world!")); // 3.9857e+09
```

#### lower

Returns a copy of the given string with all uppercase characters converted to lowercase.

```rust
fn lower(str: string) -> string;
```

Example:

```rust
println(strings.lower("Hello, world!")); // hello, world!
```

#### upper

Returns a copy of the given string with all lowercase characters converted to uppercase.

```rust
fn upper(str: string) -> string;
```

Example:

```rust
println(strings.upper("Hello, world!")); // HELLO, WORLD!
```

#### trim

Returns a copy of the given string with all leading and trailing whitespace removed.

```rust
fn trim(str: string) -> string;
```

Example:

```rust
println(strings.trim(" Hello, world! ")); // Hello, world!
```

#### starts_with

Returns `true` if the given string starts with the given prefix.

```rust
fn starts_with(str1: string, str2: string) -> bool;
```

Example:

```rust
println(strings.starts_with("Hello, world!", "Hello")); // true
println(strings.starts_with("Hello, world!", "world")); // false
```

#### ends_with

Returns `true` if the given string ends with the given suffix.

```rust
fn ends_with(str1: string, str2: string) -> bool;
```

Example:

```rust
println(strings.ends_with("Hello, world!", "world!")); // true
println(strings.ends_with("Hello, world!", "Hello"));  // false
```

#### reverse

Returns a copy of the given string with the characters in reverse order.

```rust
fn reverse(str: string) -> string;
```

Example:

```rust
println(strings.reverse("!dlrow ,olleH")); // Hello, world!
```

### arrays

The `arrays` module provides functions for working with arrays.

<table>
  <tbody>
    <tr>
      <td><a href="#new_array">new_array</a></td>
      <td><a href="#fill">fill</a></td>
      <td><a href="#index_of">index_of</a></td>
      <td><a href="#min">min</a></td>
      <td><a href="#max">max</a></td>
    </tr>
    <tr>
      <td><a href="#sum">sum</a></td>
      <td><a href="#avg">avg</a></td>
      <td><a href="#reverse">reverse</a></td>
      <td><a href="#sort">sort</a></td>
      <td></td>
    </tr>
  </tbody>
</table>

#### new_array

Creates a new array with the given minimum capacity.

> **Note:** When setting the initial capacity of an array, it will be adjusted to the next power of two. As a result, the actual capacity of the array may be larger than the specified value.

```rust
fn new_array(min_capacity: number) -> array;
```

Example:

```rust
let arr = arrays.new_array(10);
arr[] = 1;
arr[] = 2;
arr[] = 3;
println(arr); // [1, 2, 3]
```

#### fill

Returns a new array filled with the given value repeated the given number of times.

```rust
fn fill(elem, count: number) -> array;
```

Example:

```rust
let arr = arrays.fill(1, 3);
println(arr); // [1, 1, 1]
```

#### index_of

Returns the index of the first occurrence of the given element in the given array, or `-1` if the element is not found.

```rust
fn index_of(arr: array, elem) -> number;
```

Example:

```rust
let arr = [1, 2, 3];
println(arrays.index_of(arr, 2)); // 1
println(arrays.index_of(arr, 4)); // -1
```

#### min

Returns the minimum value in the given array. All elements in the array must be of the same type and comparable; otherwise, a runtime error will be raised.

```rust
fn min(arr: array) -> any;
```

Example:

```rust
let arr = [1, 2, 3];
println(arrays.min(arr)); // 1
```

#### max

Returns the maximum value in the given array. All elements in the array must be of the same type and comparable; otherwise, a runtime error will be raised.

```rust
fn max(arr: array) -> any;
```

Example:

```rust
let arr = [1, 2, 3];
println(arrays.max(arr)); // 3
```

#### sum

Returns the sum of all numbers in the given array. If any element in the array is not a number, the result will be `0`.

```rust
fn sum(arr: array) -> number;
```

Example:

```rust
let arr = [1, 2, 3];
println(arrays.sum(arr)); // 6
```

#### avg

Returns the average of all numbers in the given array. If any element in the array is not a number, the result will be `0`.

```rust
fn avg(arr: array) -> number;
```

Example:

```rust
let arr = [1, 2, 3];
println(arrays.avg(arr)); // 2
```

#### reverse

Returns a copy of the given array with the elements in reverse order.

```rust
fn reverse(arr: array) -> array;
```

Example:

```rust
let arr = [1, 2, 3];
println(arrays.reverse(arr)); // [3, 2, 1]
```

#### sort

Returns a copy of the given array with the elements sorted in ascending order.

```rust
fn sort(arr: array) -> array;
```

Example:

```rust
let arr = [2, 3, 1];
println(arrays.sort(arr)); // [1, 2, 3]
```

### utf8

The `utf8` module provides functions for working with UTF-8 strings. In Hook, strings are represented as arrays of bytes, making the functions in this module useful for working with strings that contain non-ASCII characters.

<table>
  <tbody>
    <tr>
      <td><a href="#len">len</a></td>
    </tr>
    <tr>
      <td><a href="#sub">sub</a></td>
    </tr>
  </tbody>
</table>

#### len

Returns the number of unicode characters (code points) in the given string.

```rust
fn len(str: string) -> number;
```

Example:

```rust
println(utf8.len("Hello, world!")); // 13
println(utf8.len("こんにちは世界"));   // 7
```

#### sub

Returns a substring of the given string, starting at the given index and ending at the given index (exclusive).

```rust
fn sub(str: string, start: number, end: number) -> string;
```

Example:

```rust
println(utf8.sub("Hello, world!", 7, 12)); // world
```

### regex

The `regex` module provides functions for working with regular expressions.

<table>
  <tbody>
    <tr>
      <td><a href="#new">new</a></td>
    </tr>
    <tr>
      <td><a href="#find">find</a></td>
    </tr>
    <tr>
      <td><a href="#is_match">is_match</a></td>
    </tr>
  </tbody>
</table

#### new

Creates a new regular expression with the given pattern.

```rust
fn new(pattern: string) -> userdata;
```

Example:

```rust
let regex = regex.new("^[a-z]+$");
```

#### find

Returns the start and end indices of the first match of the given regular expression in the given string, or `nil` if no match is found.

```rust
fn find(regex: userdata, str: string) -> nil|array;
```

Example:

```rust
let r = regex.new(",");
println(regex.find(r, "Hello, world!")); // [5, 6]
```

#### is_match

Returns `true` if the given regular expression matches the given string, or `false` otherwise.

```rust
fn is_match(regex: userdata, str: string) -> bool;
```

Example:

```rust
let r = regex.new("^[a-z]+$");
println(regex.is_match(r, "Hello"));         // true
println(regex.is_match(r, "Hello, world!")); // false
```

### hashing

The `hashing` module provides functions for computing cryptographic hashes.

<table>
  <tboby>
    <tr>
      <td><a href="#crc32">crc32</a></td>
      <td><a href="#crc64">crc64</a></td>
      <td><a href="#sha224">sha224</a></td>
      <td><a href="#sha256">sha256</a></td>
      <td><a href="#sha384">sha384</a></td>
    </tr>
    <tr>
      <td><a href="#sha512">sha512</a></td>
      <td><a href="#sha1">sha1</a></td>
      <td><a href="#sha3">sha3</a></td>
      <td><a href="#md5">md5</a></td>
      <td><a href="#ripemd160">ripemd160</a></td>
    </tr>
  </tbody>
</table>

#### crc32

Computes the CRC-32 hash of the given string and returns the result as a integer number.

```rust
fn crc32(str: string) -> number;
```

Example:

```rust
println(hashing.crc32("Hello, world!")); // 2.35637e+09
```

#### crc64

Computes the CRC-64 hash of the given string and returns the result as a integer number.

```rust
fn crc64(str: string) -> number;
```

Example:

```rust
println(hashing.crc64("Hello, world!")); // 1.26263e+19
```

#### sha224

Computes the SHA-224 hash of the given string and returns the result as a binary string.

```rust
fn sha224(str: string) -> string;
```

Example:

```rust
println(hex(hashing.sha224("Hello, world!"))); // 8552d8b7a7dc5476cb9e25dee69a8091290764b7...
```

#### sha256

Computes the SHA-256 hash of the given string and returns the result as a binary string.

```rust
fn sha256(str: string) -> string;
```

Example:

```rust
println(hex(hashing.sha256("Hello, world!"))); // 315f5bdb76d078c43b8ac0064e4a0164612b1fce...
```

#### sha384

Computes the SHA-384 hash of the given string and returns the result as a binary string.

```rust
fn sha384(str: string) -> string;
```

Example:

```rust
println(hex(hashing.sha384("Hello, world!"))); // 55bc556b0d2fe0fce582ba5fe07baafff0356536...
```

#### sha512

Computes the SHA-512 hash of the given string and returns the result as a binary string.

```rust
fn sha512(str: string) -> string;
```

Example:

```rust
println(hex(hashing.sha512("Hello, world!"))); // c1527cd893c124773d811911970c8fe6e857d6df...
```

#### sha1

Computes the SHA-1 hash of the given string and returns the result as a binary string.

```rust
fn sha1(str: string) -> string;
```

Example:

```rust
println(hex(hashing.sha1("Hello, world!"))); // 943a702d06f34599aee1f8da8ef9f7296031d699
```

#### sha3

Computes the SHA-3 (256) hash of the given string and returns the result as a binary string.

```rust
fn sha3(str: string) -> string;
```

Example:

```rust
println(hex(hashing.sha3("Hello, world!"))); // f345a219da005ebe9c1a1eaad97bbf38a10c8473...
```

#### md5

Computes the MD5 hash of the given string and returns the result as a binary string.

```rust
fn md5(str: string) -> string;
```

Example:

```rust
println(hex(hashing.md5("Hello, world!"))); // 6cd3556deb0da54bca060b4c39479839
```

#### ripemd160

Computes the RIPEMD-160 hash of the given string and returns the result as a binary string.

```rust
fn ripemd160(str: string) -> string;
```

Example:

```rust
println(hex(hashing.ripemd160("Hello, world!"))); // 58262d1fbdbe4530d8865d3518c6d6e41002610f
```

### encoding

The `encoding` module provides functions for encoding and decoding data.

<table>
  <tboby>
    <tr>
      <td><a href="#base32_encode">base32_encode</a></td>
      <td><a href="#base32_decode">base32_decode</a></td>
      <td><a href="#base58_encode">base58_encode</a></td>
      <td><a href="#base58_decode">base58_decode</a></td>
    </tr>
    <tr>
      <td><a href="#base64_encode">base64_encode</a></td>
      <td><a href="#base64_decode">base64_decode</a></td>
      <td><a href="#ascii85_encode">ascii85_encode</a></td>
      <td><a href="#ascii85_decode">ascii85_decode</a></td>
    </tr>
  </tbody>
</table>

#### base32_encode

Encodes the given string using the Base32 encoding.

```rust
fn base32_encode(str: string) -> string;
```

Example:

```rust
println(encoding.base32_encode("Hello, world!")); // JBSWY3DPFQQHO33SNRSCC===
```

#### base32_decode

Decodes the given string using the Base32 encoding.

```rust
fn base32_decode(str: string) -> string;
```

Example:

```rust
println(encoding.base32_decode("JBSWY3DPFQQHO33SNRSCC===")); // Hello, world!
```

#### base58_encode

Encodes the given string using the Base58 encoding.

```rust
fn base58_encode(str: string) -> string;
```

Example:

```rust
println(encoding.base58_encode("Hello, world!")); // 72k1xXWG59wUsYv7h2
```

#### base58_decode

Decodes the given string using the Base58 encoding.

```rust
fn base58_decode(str: string) -> string;
```

Example:

```rust
println(encoding.base58_decode("72k1xXWG59wUsYv7h2")); // Hello, world!
```

#### base64_encode

Encodes the given string using the Base64 encoding.

```rust
fn base64_encode(str: string) -> string;
```

Example:

```rust
println(encoding.base64_encode("Hello, world!")); // SGVsbG8sIHdvcmxkIQ==
```

#### base64_decode

Decodes the given string using the Base64 encoding.

```rust
fn base64_decode(str: string) -> string;
```

Example:

```rust
println(encoding.base64_decode("SGVsbG8sIHdvcmxkIQ==")); // Hello, world!
```

#### ascii85_encode

Encodes the given string using the Ascii85 encoding.

```rust
fn ascii85_encode(str: string) -> string;
```

Example:

```rust
println(encoding.ascii85_encode("Hello, world!")); // 87cURD_*#TDfTZ)+T
```

#### ascii85_decode

Decodes the given string using the Ascii85 encoding.

```rust
fn ascii85_decode(str: string) -> string;
```

Example:

```rust
println(encoding.ascii85_decode("87cURD_*#TDfTZ)+T")); // Hello, world!
```

### socket

The `socket` module provides functions and constants for working with sockets.

<table>
  <tbody>
    <tr>
      <td><a href="#af_inet-af_inet6">AF_INET</a></td>
      <td><a href="#af_inet-af_inet6">AF_INET6</a></td>
      <td><a href="#sock_stream-sock_dgram">SOCK_STREAM</a></td>
      <td><a href="#sock_stream-sock_dgram">SOCK_DGRAM</a></td>
      <td><a href="#ipproto_tcp-ipproto_udp">IPPROTO_TCP</a></td>
    </tr>
    <tr>
      <td><a href="#ipproto_tcp-ipproto_udp">IPPROTO_UDP</a></td>
      <td><a href="#sol_socket-so_reuseaddr">SOL_SOCKET</a></td>
      <td><a href="#sol_socket-so_reuseaddr">SO_REUSEADDR</a></td>
      <td><a href="#new">new</a></td>
      <td><a href="#close">close</a></td>
    </tr>
    <tr>
      <td><a href="#connect">connect</a></td>
      <td><a href="#accept">accept</a></td>
      <td><a href="#bind">bind</a></td>
      <td><a href="#listen">listen</a></td>
      <td><a href="#send">send</a></td>
    </tr>
    <tr>
      <td><a href="#recv">recv</a></td>
      <td><a href="#set_option">set_option</a></td>
      <td><a href="#get_option">get_option</a></td>
      <td><a href="#set_block">set_block</a></td>
      <td><a href="#set_nonblock">set_nonblock</a></td>
    </tr>
  </tbody>
</table>

#### AF_INET, AF_INET6

The `AF_INET` and `AF_INET6` constants are used to specify the address family of a socket.

- `AF_INET` is used for IPv4 sockets.
- `AF_INET6` is used for IPv6 sockets.

```rust
let AF_INET: number;
let AF_INET6: number;
```

Example:

```rust
let sock_ipv4 = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
let sock_ipv6 = socket.new(socket.AF_INET6, socket.SOCK_STREAM, 0);
```

#### SOCK_STREAM, SOCK_DGRAM

The `SOCK_STREAM` and `SOCK_DGRAM` constants are used to specify the type of a socket.

- `SOCK_STREAM` is used for TCP sockets.
- `SOCK_DGRAM` is used for UDP sockets.

```rust
let SOCK_STREAM: number;
let SOCK_DGRAM: number;
```

Example:

```rust
let sock_tcp = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
let sock_udp = socket.new(socket.AF_INET, socket.SOCK_DGRAM, 0);
```

#### IPPROTO_TCP, IPPROTO_UDP

The `IPPROTO_TCP` and `IPPROTO_UDP` constants are used to specify the protocol of a socket.

- `IPPROTO_TCP` is used for TCP sockets.
- `IPPROTO_UDP` is used for UDP sockets.

> **Note:** it is possible to pass `0` as the protocol argument to `socket.new` and the protocol will be automatically chosen based on the type of the socket.

```rust
let IPPROTO_TCP: number;
let IPPROTO_UDP: number;
```

Example:

```rust
let sock_tcp = socket.new(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP);
let sock_udp = socket.new(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP);
```

#### SOL_SOCKET, SO_REUSEADDR

The `SOL_SOCKET` and `SO_REUSEADDR` constants are used to specify the level and option of a socket option. See [set_option](#set_option) for more information.

- `SOL_SOCKET` is used to specify the level of a socket option.
- `SO_REUSEADDR` is a socket option that allows a socket to be bound to an address that is already in use.

```rust
let SOL_SOCKET: number;
let SO_REUSEADDR: number;
```

Example:

```rust
socket.set_option(sock, socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);
```

#### new

Creates a new socket with the given domain, type, and protocol.

```rust
fn new(domain: number, type: number, protocol: number) -> userdata;
```

- `domain` is the address family of the socket. See [AF_INET, AF_INET6](#af_inet-af_inet6).
- `type` is the type of the socket. See [SOCK_STREAM, SOCK_DGRAM](#sock_stream-sock_dgram).
- `protocol` is the protocol of the socket. See [IPPROTO_TCP, IPPROTO_UDP](#ipproto_tcp-ipproto_udp).

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
```

#### close

Closes the given socket.

```rust
fn close(sock: userdata);
```

Example:

```rust
socket.close(sock);
```

#### connect

Connects the given socket to the given host and port.

```rust
fn connect(sock: userdata, host: string, port: number);
```

- `sock` is the socket to connect.
- `host` is a string representing the host to connect to.
- `port` is integer number representing the port to connect to.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.connect(sock, "localhost", 8080);
```

#### accept

Accepts a new connection on the given socket.

```rust
fn accept(sock: userdata) -> nil|userdata;
```

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.bind(sock, "localhost", 8080);
socket.listen(sock, 5);
let client = socket.accept(sock);
```

#### bind

Binds the given socket to the given host and port.

```rust
fn bind(sock: userdata, host: string, port: number);
```

- `sock` is the socket to bind.
- `host` is a string representing the host to bind to.
- `port` is integer number representing the port to bind to.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.bind(sock, "localhost", 8080);
```

#### listen

Starts listening on the given socket.

```rust
fn listen(sock: userdata, backlog: number);
```

- `sock` is the socket to listen on.
- `backlog` is the maximum number of pending connections.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.bind(sock, "localhost", 8080);
socket.listen(sock, 5);
```

#### send

Sends data on the given socket.

```rust
fn send(sock: userdata, data: string) -> number;
```

- `sock` is the socket to send data on.
- `data` is a string containing the data to send.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.connect(sock, "localhost", 8080);
socket.send(sock, "hello");
```

#### recv

Receives data on the given socket.

```rust
fn recv(sock: userdata, size: number) -> string;
```

- `sock` is the socket to receive data on.
- `size` is the maximum number of bytes to receive.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.connect(sock, "localhost", 8080);
let data = socket.recv(sock, 5);
```

#### set_option

Sets a socket option.

```rust
fn set_option(sock: userdata, level: number, option: number, value: number);
```

- `sock` is the socket to set the option on.
- `level` is a integer number representing the level of the option.
- `option` is a integer number representing the option.
- `value` is a integer number representing the value of the option.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.set_option(sock, socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);
```

#### get_option

Gets a socket option.

```rust
fn get_option(sock: userdata, level: number, option: number) -> number;
```

- `sock` is the socket to get the option from.
- `level` is a integer number representing the level of the option.
- `option` is a integer number representing the option.

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.set_option(sock, socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);
let value = socket.get_option(sock, socket.SOL_SOCKET, socket.SO_REUSEADDR);
println(value); // 1
```
  set_block(sock: userdata)
  set_nonblock(sock: userdata)

#### set_block

Sets the given socket to blocking mode.

```rust
fn set_block(sock: userdata);
```

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.set_block(sock);
```

#### set_nonblock

Sets the given socket to non-blocking mode.

```rust
fn set_nonblock(sock: userdata);
```

Example:

```rust
let sock = socket.new(socket.AF_INET, socket.SOCK_STREAM, 0);
socket.set_nonblock(sock);
```

### json

The `json` module provides functions for encoding and decoding JSON.

<table>
  <tbody>
    <tr>
      <td><a href="#encode">encode</a></td>
    </tr>
    <tr>
      <td><a href="#decode">decode</a></td>
    </tr>
  </tbody>
</table>

#### encode

Encodes the given value to JSON.

```rust
fn encode(value: any) -> string;
```

Example:

```rust
let json = json.encode({ hello: "world" });
println(json); // {"hello":"world"}
```

#### decode

Decodes the given JSON string to a value.

```rust
fn decode(json: string) -> any;
```

Example:

```rust
let value = json.decode('{"hello":"world"}');
println(value.hello); // world
```

### lists

The `lists` module provides functions for working with lists.

<table>
  <tbody>
    <tr>
      <td><a href="#new_linked_list">new_linked_list</a></td>
      <td><a href="#len">len</a></td>
      <td><a href="#is_empty">is_empty</a></td>
      <td><a href="#push_front">push_front</a></td>
      <td><a href="#push_back">push_back</a></td>
    </tr>
    <tr>
      <td><a href="#pop_front">pop_front</a></td>
      <td><a href="#pop_back">pop_back</a></td>
      <td><a href="#front">front</a></td>
      <td><a href="#back">back</a></td>
      <td></td>
    </tr>
  </tbody>
</table>

#### new_linked_list

Creates a new linked list.

```rust
fn new_linked_list() -> userdata;
```

Example:

```rust
let list = lists.new_linked_list();
```

#### len

Returns the length of the given list.

```rust
fn len(list: userdata) -> number;
```

Example:

```rust
mut list = lists.new_linked_list();
list = lists.push_back(list, 1);
list = lists.push_back(list, 2);
list = lists.push_back(list, 3);
println(lists.len(list)); // 3
```

#### is_empty

Returns `true` if the given list is empty.

```rust
fn is_empty(list: userdata) -> bool;
```

Example:

```rust
let list = lists.new_linked_list();
println(lists.is_empty(list)); // true
```

#### push_front

Pushes the given value to the front of the given list and returns the new list.

```rust
fn push_front(list: userdata, value: any) -> userdata;
```

Example:

```rust
mut list = lists.new_linked_list();
list = lists.push_front(list, 1);
list = lists.push_front(list, 2);
println(lists.front(list)); // 2
println(lists.back(list));  // 1
```
```

#### push_back

Pushes the given value to the back of the given list and returns the new list.

```rust
fn push_back(list: userdata, value: any) -> userdata;
```

Example:

```rust
mut list = lists.new_linked_list();
list = lists.push_back(list, 1);
list = lists.push_back(list, 2);
println(lists.front(list)); // 1
println(lists.back(list));  // 2
```

#### pop_front

Pops the first value from the given list and returns the new list.

```rust
fn pop_front(list: userdata) -> userdata;
```

Example:

```rust
mut list1 = lists.new_linked_list();
list1 = lists.push_back(list1, 1);
list1 = lists.push_back(list1, 2);
let list2 = lists.pop_front(list1);
println(lists.front(list2)); // 2
```

#### pop_back

Pops the last value from the given list and returns the new list.

```rust
fn pop_back(list: userdata) -> userdata;
```

Example:

```rust
mut list1 = lists.new_linked_list();
list1 = lists.push_back(list1, 1);
list1 = lists.push_back(list1, 2);
let list2 = lists.pop_back(list1);
println(lists.back(list2)); // 1
```

#### front

Returns the first value from the given list.

```rust
fn front(list: userdata) -> any;
```

Example:

```rust
mut list = lists.new_linked_list();
list = lists.push_back(list, 1);
list = lists.push_back(list, 2);
println(lists.front(list)); // 1
```

#### back

Returns the last value from the given list.

```rust
fn back(list: userdata) -> any;
```

Example:

```rust
mut list = lists.new_linked_list();
list = lists.push_back(list, 1);
list = lists.push_back(list, 2);
println(lists.back(list)); // 2
```
