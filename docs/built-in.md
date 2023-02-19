
# Hook's built-in functions

Hook provides a range of convenient built-in functions that serve various purposes such as printing to the terminal, type checking, and more. These functions can be accessed globally, and this document provides a comprehensive list of all of them.

## List of functions

Here is a quick overview of all the functions. Click on a function name to jump to its documentation.

<table>
  <tbody>
    <tr>
      <td><a href="#print">print</a></td>
      <td><a href="#println">println</a></td>
      <td><a href="#type">type</a></td>
      <td><a href="#is_nil">is_nil</a></td>
      <td><a href="#is_bool">is_bool</a></td>
      <td><a href="#is_number">is_number</a></td>
    </tr>
    <tr>
      <td><a href="#is_int">is_int</a></td>
      <td><a href="#is_string">is_string</a></td>
      <td><a href="#is_range">is_range</a></td>
      <td><a href="#is_array">is_array</a></td>
      <td><a href="#is_struct">is_struct</a></td>
      <td><a href="#is_instance">is_instance</a></td>
    </tr>
    <tr>
      <td><a href="#is_iterator">is_iterator</a></td>
      <td><a href="#is_callable">is_callable</a></td>
      <td><a href="#is_userdata">is_userdata</a></td>
      <td><a href="#is_object">is_object</a></td>
      <td><a href="#is_comparable">is_comparable</a></td>
      <td><a href="#is_iterable">is_iterable</a></td>
    </tr>
    <tr>
      <td><a href="#to_bool">to_bool</a></td>
      <td><a href="#to_int">to_int</a></td>
      <td><a href="#to_number">to_number</a></td>
      <td><a href="#to_string">to_string</a></td>
      <td><a href="#ord">ord</a></td>
      <td><a href="#chr">chr</a></td>
    </tr>
    <tr>
      <td><a href="#hex">hex</a></td>
      <td><a href="#bin">bin</a></td>
      <td><a href="#adress">adress</a></td>
      <td><a href="#refcount">refcount</a></td>
      <td><a href="#cap">cap</a></td>
      <td><a href="#len">len</a></td>
    </tr>
    <tr>
      <td><a href="#is_empty">is_empty</a></td>
      <td><a href="#compare">compare</a></td>
      <td><a href="#split">split</a></td>
      <td><a href="#join">join</a></td>
      <td><a href="#iter">iter</a></td>
      <td><a href="#valid">valid</a></td>
    </tr>
    <tr>
      <td><a href="#current">current</a></td>
      <td><a href="#next">next</a></td>
      <td><a href="#sleep">sleep</a></td>
      <td><a href="#assert">assert</a></td>
      <td><a href="#panic">panic</a></td>
      <td></td>
    </tr>
  </tbody>
</table>

Below you will find all functions presented with their signatures, a brief description of their functionality, and examples of how to use them.

### print

Prints the given value without a newline.

```rust
fn print(value);
```

Example:

```rust
print("Hello, "); // Hello, world!
print("World!");  //
```

### println

Prints the given value with a newline.

```rust
fn println(value);
```

Example:

```rust
println("Hello, "); // Hello,
println("World!");  // World!
```

### type

Returns the type of the given value as a string.

```rust
fn type(value) -> string;
```

Example:

```rust
println(type(1));     // number
println(type(3.14));  // number
println(type("foo")); // string
println(type(true));  // bool
println(type(nil));   // nil
```

### is_nil

Returns `true` if the given value is nil.

```rust
fn is_nil(value) -> bool;
```

Example:

```rust
println(is_nil(nil)); // true
println(is_nil(1));   // false
```

### is_bool

Returns `true` if the given value is a boolean.

```rust
fn is_bool(value) -> bool;
```

Example:

```rust
println(is_bool(true));  // true
println(is_bool(false)); // true
println(is_bool(1));     // false
```

### is_number

Returns `true` if the given value is a number.

```rust
fn is_number(value) -> bool;
```

Example:

```rust
println(is_number(1));     // true
println(is_number(3.14));  // true
println(is_number("foo")); // false
```

### is_int

Returns `true` if the given value is an integer number.

```rust
fn is_int(value) -> bool;
```

Example:

```rust
println(is_int(1));     // true
println(is_int(3.14));  // false
println(is_int("foo")); // false
```

### is_string

Returns `true` if the given value is a string.

```rust
fn is_string(value) -> bool;
```

Example:

```rust
println(is_string("foo")); // true
println(is_string(1));     // false
```

### is_range

Returns `true` if the given value is a range.

```rust
fn is_range(value) -> bool;
```

Example:

```rust
println(is_range(1..10)); // true
println(is_range(1));     // false
```

### is_array

Returns `true` if the given value is an array.

```rust
fn is_array(value) -> bool;
```

Example:

```rust
println(is_array([1, 2, 3])); // true
println(is_array(1));         // false
```

### is_struct

Returns `true` if the given value is a structure.

```rust
fn is_struct(value) -> bool;
```

Example:

```rust
struct Point { x, y }
println(is_struct(Point)); // true
println(is_struct(1));     // false
```

### is_instance

Returns `true` if the given value is an instance of a structure.

```rust
fn is_instance(value) -> bool;
```

Example:

```rust
println(is_instance(Point { 1, 2 })); // true
println(is_instance(1));              // false
```

### is_iterator

Returns `true` if the given value is an iterator.

```rust
fn is_iterator(value) -> bool;
```

Example:

```rust
println(iter(1..10));     // true
println(iter([1, 2, 3])); // true
println(1);               // false
```

### is_callable

Returns `true` if the given value is callable (i.e. a function).

```rust
fn is_callable(value) -> bool;
```

Example:

```rust
fn foo() {}
println(is_callable(foo));   // true
println(is_callable(|| {})); // true
println(is_callable(print)); // true
println(is_callable(1));     // false
```

### is_userdata

Returns `true` if the given value is a userdata.

```rust
fn is_userdata(value) -> bool;
```

Example:

```rust
import { stdout } from io;
println(is_userdata(stdout)); // true
println(is_userdata(1));      // false
```

### is_object

Returns `true` if the given value is an object.

```rust
fn is_object(value) -> bool;
```

Example:

```rust
println(is_object(1));         // false
println(is_object(3.14));      // false
println(is_object(1..10));     // true
println(is_object("foo"));     // true
println(is_object([1, 2, 3])); // true
println(is_object(print));     // true
```

### is_comparable

Returns `true` if the given value is comparable.

```rust
fn is_comparable(value) -> bool;
```

Example:

```rust
println(is_comparable(1));         // true
println(is_comparable(3.14));      // true
println(is_comparable(1..10));     // true
println(is_comparable("foo"));     // true
println(is_comparable([1, 2, 3])); // true
println(is_comparable(print));     // false
```

### is_iterable

Returns `true` if the given value is iterable.

```rust
fn is_iterable(value) -> bool;
```

Example:

```rust
println(is_iterable(1));         // false
println(is_iterable(3.14));      // false
println(is_iterable("foo"));     // false
println(is_iterable(1..10));     // true
println(is_iterable([1, 2, 3])); // true
println(is_iterable(print));     // false
```

### to_bool

Converts a value to a boolean. In Hook, only `nil` and `false` are considered false. All other values are considered `true`.

```rust
fn to_bool(value) -> bool;
```

Example:

```rust
println(to_bool(false)); // false
println(to_bool(nil));   // false
println(to_bool(true));  // true
println(to_bool(0));     // true
println(to_bool(1));     // true
println(to_bool(3.14));  // true
println(to_bool(""));    // true
```

### to_int

Converts a floating point number or a string to an integer number. This function rises an error if the value cannot be converted.
In fact, all numbers in Hook are floating point numbers. This function converts the number to an integer by truncating the fractional part.

```rust
fn to_int(value: number|string) -> number;
```

Example:

```rust
println(to_int(1));      // 1
println(to_int(3.14));   // 3
println(to_int("3.14")); // 3
println(to_int("foo"));  // Raises an error.
```

### to_number

Converts a string to a floating point number. This function rises an error if the value cannot be converted.

```rust
fn to_number(value: number|string) -> number;
```

Example:

```rust
println(to_number(1));      // 1
println(to_number(3.14));   // 3.14
println(to_number("3.14")); // 3.14
println(to_number("foo"));  // Raises an error.
```

### to_string

Converts a value to a string. Some types of values aren't convertible to a string.

```rust
fn to_string(value: nil|bool|number|string) -> string;
```

Example:

```rust
println(to_string(1));         // "1"
println(to_string(3.14));      // "3.14"
println(to_string(true));      // "true"
println(to_string(nil));       // "nil"
println(to_string([1, 2, 3])); // Raises an error.
println(to_string(print));     // <callable print>
```

### ord

Returns the ASCII code of the first character of the given string.

```rust
fn ord(value: string) -> number;
```

Example:

```rust
println(ord("a")); // 97
```

### chr

Returns a string that contains the character with the given ASCII code.

```rust
fn chr(value: number) -> string;
```

Example:

```rust
println(chr(97)); // "a"
```

### hex

Converts a string to a hexadecimal string.

```rust
fn hex(value: string) -> string;
```

Example:

```rust
println(hex("foo")); // "666f6f"
```

### bin

Converts a string to a binary string.

```rust
fn bin(value: string) -> string;
```

Example:

```rust
println(bin("666f6f")); // "foo"
```

### address

Returns a hexadecimal string that represents the memory address of the given value if it is an object.
Otherwise, it returns "(nil)".

```rust
fn address(value) -> string;
```

Example:

```rust
println(address(1));     // (nil)
println(address(3.14));  // (nil)
println(address("foo")); // 0x563ff4a0f730
println(address(1..10)); // 0x555b2c87dac0
```

### refcount

Returns the reference count of the given value if it is an object. Otherwise, it returns 0.

```rust
fn refcount(value) -> number;
```

Example:

```rust
println(refcount(1));     // 0
println(refcount(3.14));  // 0
println(refcount("foo")); // 1
println(refcount(1..10)); // 1
```

### cap

Returns the capacity of the given string or array.

```rust
fn cap(value: string|array) -> number;
```

Example:

```rust
println(cap("foo"));     // 8
println(cap([1, 2], 3)); // 8
```

### len

Returns the length of the given compond value.

```rust
fn len(value: string|range|array|struct|instance) -> number;
```

Example:

```rust
println(len("foo"));     // 3
println(len([1, 2, 3])); // 3
println(len(1..10));     // 10
```

### is_empty

Returns `true` if the given compound value is empty.

```rust
fn is_empty(value: string|range|array|struct|instance) -> bool;
```

Example:

```rust
println(is_empty(""));        // true
println(is_empty([]));        // true
println(is_empty(1..1));      // false
println(is_empty("foo"));     // false
println(is_empty([1, 2, 3])); // false
```

### compare

Compares two values and returns a number that represents the result of the comparison.

```rust
fn compare(val1, val2) -> number;
```

Example:

```rust
println(compare(1, 2)); // -1
println(compare(2, 1)); // 1
println(compare(1, 1)); // 0
```

### split

Splits the given string into an array of strings using the given separator.

```rust
fn split(str: string, separator: string) -> array;
```

Example:

```rust
println(split("foo,bar,baz", ",")); // ["foo", "bar", "baz"]
```

### join

Joins the given array of strings into a single string containing the given separator between each element.

```rust
fn join(arr: array, separator: string) -> string;
```

Example:

```rust
println(join(["foo", "bar", "baz"], ",")); // "foo,bar,baz"
```

### iter

Creates an iterator from a range or an array. This function raises an error if the given value is not iterable.

```rust
fn iter(value: iterator|range|array) -> iterator;
```

Example:

```rust
for (x in iter(1..3)) { // 1
  println(x);           // 2
}                       // 3
```

### valid

Returns `true` if the given iterator has a valid current value.

```rust
fn valid(it: iterator) -> bool;
```

Example:

```rust
mut it = iter(1..3);
println(valid(it)); // true
it = next(it);
println(valid(it)); // true
it = next(it);
println(valid(it)); // true
it = next(it);
println(valid(it)); // false
```

### current

Returns the current value of the given iterator.

```rust
fn current(it: iterator) -> any;
```

Example:

```rust
mut it = iter(1..3);
println(current(it)); // 1
it = next(it);
println(current(it)); // 2
it = next(it);
println(current(it)); // 3
it = next(it);
println(current(it)); // nil
```

### next

Advances the given iterator to the next value and returns an updated iterator.

```rust
fn next(it: iterator) -> iterator;
```

Example:

```rust
mut it = iter(1..3);
println(current(it)); // 1
it = next(it);
println(current(it)); // 2
it = next(it);
println(current(it)); // 3
```

### sleep

Sleeps for the given number of milliseconds.

```rust
fn sleep(ms: number);
```

Example:

```rust
sleep(1000); // Sleeps for 1 second.
```

### assert

Raises an error if the given assertion is `false`.

```rust
fn assert(assertion: bool, msg: string);
```

Example:

```rust
assert(1 == 2, "1 is not equal to 2"); // Raises an error.
```

### panic

Raises an error with the given message.

```rust
fn panic(msg: string);
```

Example:

```rust
panic("something went wrong!"); // panic: something went wrong!
```
