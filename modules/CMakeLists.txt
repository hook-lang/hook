
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

add_library(math SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_compiler.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  h_math.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ../src/h_scanner.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(os SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_compiler.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  h_os.c
  ../src/h_range.c
  ../src/h_scanner.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(io SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_compiler.c
  ../src/h_error.c
  ../src/h_hash.c
  h_io.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ../src/h_scanner.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(strings SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ../src/h_string_map.c
  ../src/h_string.c
  h_strings.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(arrays SHARED
  ../src/h_array.c
  h_arrays.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

if(MSVC)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
  find_package(hiredis CONFIG REQUIRED)
endif()

add_library(sqlite SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  sqlite.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(url SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  url.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(redis SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  redis.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(hashing SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ../src/h_error.c
  ../src/h_hash.c
  hashing.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ripemd160.c
  sha2.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(encoding SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  base32.c
  base58.c
  base64.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  encoding.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

add_library(secp256r1 SHARED
  ../src/h_array.c
  ../src/h_builtin.c
  ../src/h_callable.c
  ../src/h_chunk.c
  ../src/h_common.c
  ecc.c
  ../src/h_error.c
  ../src/h_hash.c
  ../src/h_iterator.c
  ../src/h_memory.c
  ../src/h_module.c
  ../src/h_range.c
  secp256r1.c
  ../src/h_string_map.c
  ../src/h_string.c
  ../src/h_struct.c
  ../src/h_userdata.c
  ../src/h_value.c
  ../src/h_vm.c)

if(MSVC)
  target_link_libraries(sqlite PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(url PRIVATE CURL::libcurl)
  target_link_libraries(redis PRIVATE hiredis::hiredis)
else()
  target_link_libraries(sqlite sqlite3)
  target_link_libraries(url curl)
  target_link_libraries(redis hiredis)
endif()

