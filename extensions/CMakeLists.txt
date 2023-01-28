
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

if(MSVC)
  find_package(CURL CONFIG REQUIRED)
  find_package(libmysql REQUIRED)
endif()

if (MSVC)
  include_directories("C:/Program Files (x86)/hiredis/include")
  link_directories("C:/Program Files (x86)/hiredis/lib")
  include_directories("C:/vcpkg/packages/fastcgi_x86-windows/include/fastcgi")
  link_directories("C:/vcpkg/packages/fastcgi_x86-windows/lib")
endif()

if(APPLE)
  include_directories(/usr/local/opt/mysql-client/include)
  link_directories(/usr/local/opt/mysql-client/lib)
  include_directories(/opt/homebrew/opt/mysql-client/include)
  link_directories(/opt/homebrew/opt/mysql-client/lib)
  include_directories(/opt/homebrew/Cellar/fcgi/2.4.2/include)
  link_directories(/opt/homebrew/Cellar/fcgi/2.4.2/lib)
  include_directories(/opt/homebrew/Cellar/gmp/6.2.1_1/include)
  link_directories(/opt/homebrew/Cellar/gmp/6.2.1_1/lib)
endif()

add_library(sqlite_mod SHARED
  sqlite.c
  deps/sqlite3.c
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

add_library(curl_mod SHARED
  curl.c
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

add_library(redis_mod SHARED
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

add_library(secp256r1_mod SHARED
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

add_library(fastcgi_mod SHARED
  fastcgi.c
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

add_library(mysql_mod SHARED
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

add_library(bigint_mod SHARED
  bigint.c
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
  target_link_libraries(curl_mod PRIVATE CURL::libcurl)
  target_link_libraries(mysql_mod PRIVATE ${MYSQL_LIBRARIES})
else()
  target_link_libraries(curl_mod curl)
  target_link_libraries(mysql_mod mysqlclient)
endif()

target_link_libraries(redis_mod hiredis)
target_link_libraries(fastcgi_mod fcgi)
target_link_libraries(bigint_mod gmp)
