
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

if(MSVC)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
  find_package(hiredis CONFIG REQUIRED)
endif()

add_library(sqlite SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_error.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  sqlite.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(url SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_error.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  url.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(redis SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_error.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  redis.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(hashing SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ../src/hook_error.c
  hashing.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  ripemd160.c
  sha2.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(encoding SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  base32.c
  base58.c
  base64.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  encoding.c
  ../src/hook_error.c
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

add_library(secp256r1 SHARED
  ../src/hook_array.c
  ../src/hook_builtin.c
  ../src/hook_callable.c
  ../src/hook_chunk.c
  ../src/hook_utils.c
  ecc.c
  ../src/hook_error.c
  ../src/hook_iterable.c
  ../src/hook_iterator.c
  ../src/hook_memory.c
  ../src/hook_module.c
  ../src/hook_range.c
  secp256r1.c
  ../src/hook_string_map.c
  ../src/hook_string.c
  ../src/hook_struct.c
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

if(MSVC)
  target_link_libraries(sqlite PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(url PRIVATE CURL::libcurl)
  target_link_libraries(redis PRIVATE hiredis::hiredis)
else()
  target_link_libraries(sqlite sqlite3)
  target_link_libraries(url curl)
  target_link_libraries(redis hiredis)
endif()
