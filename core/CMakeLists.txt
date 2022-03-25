
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

add_library(math SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_compiler.c
  ../src/hook_error.c
  ../src/hook_hash.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  hook_math.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  ../src/hook_scanner.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(os SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_compiler.c
  ../src/hook_error.c
  ../src/hook_hash.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  hook_os.c
  ../src/hook_range.c
  ../src/hook_scanner.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(io SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_compiler.c
  ../src/hook_error.c
  ../src/hook_hash.c
  hook_io.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  ../src/hook_scanner.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(strings SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_error.c
  ../src/hook_hash.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  hook_strings.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(arrays SHARED
  ../src/hook_array.c
  hook_arrays.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_error.c
  ../src/hook_hash.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)
