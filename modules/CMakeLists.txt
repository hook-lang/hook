
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

if(MSVC)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
  find_package(libmysql REQUIRED)
endif()

if (MSVC)
  include_directories("C:/Program Files (x86)/hiredis/include")
  link_directories("C:/Program Files (x86)/hiredis/lib")
endif()

if(APPLE)
  include_directories(/usr/local/opt/mysql-client/include)
  link_directories(/usr/local/opt/mysql-client/lib)
  #include_directories(/opt/homebrew/opt/mysql-client/include)
  #link_directories(/opt/homebrew/opt/mysql-client/lib)
endif()

add_library(sqlite SHARED
  sqlite.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

add_library(url SHARED
  url.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

add_library(redis SHARED
  redis.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

add_library(hashing SHARED
  hashing.c
  deps/ripemd160.c
  deps/sha2.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

add_library(encoding SHARED
  encoding.c
  deps/base32.c
  deps/base58.c
  deps/base64.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

add_library(secp256r1 SHARED
  secp256r1.c
  deps/ecc.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

add_library(mysql SHARED
  mysql.c
  ../src/hk_array.c
  ../src/hk_builtin.c
  ../src/hk_callable.c
  ../src/hk_chunk.c
  ../src/hk_utils.c
  ../src/hk_error.c
  ../src/hk_iterable.c
  ../src/hk_iterator.c
  ../src/hk_memory.c
  ../src/hk_module.c
  ../src/hk_range.c
  ../src/hk_string_map.c
  ../src/hk_string.c
  ../src/hk_struct.c
  ../src/hk_userdata.c
  ../src/hk_value.c
  ../src/hk_vm.c)

if(MSVC)
  target_link_libraries(sqlite PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(url PRIVATE CURL::libcurl)
  target_link_libraries(mysql PRIVATE ${MYSQL_LIBRARIES})
else()
  target_link_libraries(sqlite sqlite3)
  target_link_libraries(url curl)
  target_link_libraries(mysql mysqlclient)
endif()

target_link_libraries(redis hiredis)
