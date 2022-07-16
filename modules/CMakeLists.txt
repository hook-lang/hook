
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

if(MSVC)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
  find_package(hiredis CONFIG REQUIRED)
  find_package(libmysql REQUIRED)
endif()

if(APPLE)
  include_directories(/usr/local/opt/mysql-client/include)
  link_directories(/usr/local/opt/mysql-client/lib)
endif()

add_library(sqlite SHARED
  sqlite.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(url SHARED
  url.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(redis SHARED
  redis.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(hashing SHARED
  hashing.c
  ripemd160.c
  sha2.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(encoding SHARED
  base32.c
  base58.c
  base64.c
  encoding.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(secp256r1 SHARED
  ecc.c
  secp256r1.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

add_library(mysql SHARED
  mysql.c
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
  ../src/hook_userdata.c
  ../src/hook_value.c
  ../src/hook_vm.c)

if(MSVC)
  target_link_libraries(sqlite PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(url PRIVATE CURL::libcurl)
  target_link_libraries(redis PRIVATE hiredis::hiredis)
  target_link_libraries(mysql PRIVATE ${MYSQL_LIBRARIES})
else()
  target_link_libraries(sqlite sqlite3)
  target_link_libraries(url curl)
  target_link_libraries(redis hiredis)
  target_link_libraries(mysql mysqlclient)
endif()
