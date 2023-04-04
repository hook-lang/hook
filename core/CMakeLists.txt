
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

add_library(math_mod SHARED
  math.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/compiler.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/scanner.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(os_mod SHARED
  os.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/compiler.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/scanner.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(io_mod SHARED
  io.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/compiler.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/scanner.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(numbers_mod SHARED
  numbers.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(strings_mod SHARED
  strings.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(arrays_mod SHARED
  arrays.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(utf8_mod SHARED
  utf8.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/state.c
  ../src/range.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(regex_mod SHARED
  regex.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(hashing_mod SHARED
  hashing.c
  deps/crc32.c
  deps/crc64.c
  deps/md5.c
  deps/ripemd160.c
  deps/sha1.c
  deps/sha2.c
  deps/sha3.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(encoding_mod SHARED
  encoding.c
  deps/ascii85.c
  deps/base32.c
  deps/base58.c
  deps/base64.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(socket_mod SHARED
  socket.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(json_mod SHARED
  json.c
  deps/cJSON.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)

add_library(lists_mod SHARED
  lists.c
  ../src/array.c
  ../src/builtin.c
  ../src/callable.c
  ../src/check.c
  ../src/chunk.c
  ../src/utils.c
  ../src/error.c
  ../src/iterable.c
  ../src/iterator.c
  ../src/memory.c
  ../src/module.c
  ../src/range.c
  ../src/state.c
  ../src/string_map.c
  ../src/string.c
  ../src/struct.c
  ../src/userdata.c
  ../src/value.c)
