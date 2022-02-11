cmake_minimum_required(VERSION 3.0)

project(hook VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address -fprofile-arcs -ftest-coverage")
endif()

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/../bin)
set(LIBRARY_DIR ${CMAKE_BINARY_DIR}/../lib)
set(PACKAGE_DIR ${CMAKE_BINARY_DIR}/../package)

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR})
endif()

add_executable(${PROJECT_NAME}
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/compiler.c
  src/dump.c
  src/error.c
  src/fs.c
  src/hash.c
  src/main.c
  src/memory.c
  src/module.c
  src/scanner.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
  target_link_libraries(${PROJECT_NAME} dl)
endif()

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

add_library(math SHARED
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/compiler.c
  src/error.c
  src/hash.c
  src/math.c
  src/memory.c
  src/module.c
  src/scanner.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(os SHARED
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/compiler.c
  src/error.c
  src/hash.c
  src/memory.c
  src/module.c
  src/os.c
  src/scanner.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(io SHARED
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/compiler.c
  src/error.c
  src/hash.c
  src/io.c
  src/memory.c
  src/module.c
  src/scanner.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(strings SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/error.c
  src/hash.c
  src/memory.c
  src/module.c
  src/string.c
  src/strings.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(arrays SHARED
  src/array.c
  src/arrays.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/error.c
  src/hash.c
  src/memory.c
  src/module.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

if(MSVC)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
  find_package(hiredis CONFIG REQUIRED)
endif()

add_library(sqlite SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/error.c
  src/hash.c
  src/memory.c
  src/module.c
  src/sqlite.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(url SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/error.c
  src/hash.c
  src/memory.c
  src/module.c
  src/string.c
  src/struct.c
  src/url.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(redis SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/error.c
  src/hash.c
  src/memory.c
  src/module.c
  src/redis.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

  add_library(hashing SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/common.c
  src/error.c
  src/hash.c
  src/hashing.c
  src/memory.c
  src/module.c
  src/ripemd160.c
  src/sha2.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

if(MSVC)
  target_link_libraries(sqlite PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(url PRIVATE CURL::libcurl)
  target_link_libraries(redis PRIVATE hiredis::hiredis)
else()
  target_link_libraries(sqlite sqlite3)
  target_link_libraries(url curl)
  target_link_libraries(redis hiredis)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(fmt TGZ)
set(CPACK_GENERATOR ${fmt})
set(CPACK_SOURCE_GENERATOR ${fmt})

set(CPACK_OUTPUT_FILE_PREFIX ${PACKAGE_DIR})
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

string(TOLOWER ${PROJECT_NAME} project)
string(TOLOWER ${CMAKE_SYSTEM_NAME} os)
string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} arch)

if(${os} STREQUAL "darwin")
  set(os "macos")
endif()

if(${arch} MATCHES "^(x86_64|amd64)$")
  set(arch "x64")
elseif(${arch} MATCHES "^(i386|i586|i686|i786|i886)$")
  set(arch "x86")
elseif(${arch} MATCHES "^(aarch64|armv8)$")
  set(arch "arm64")
endif()

set(CPACK_PACKAGE_FILE_NAME "${project}-${PROJECT_VERSION}-${os}-${arch}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${project}-${PROJECT_VERSION}")

file(READ ${CMAKE_CURRENT_LIST_DIR}/.cpack_ignore cpack_ignore)
string(REGEX REPLACE "\n" ";" cpack_ignore ${cpack_ignore})
set(CPACK_SOURCE_IGNORE_FILES "${cpack_ignore}")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

install(DIRECTORY bin/ DESTINATION bin)
install(DIRECTORY example/ DESTINATION example)
install(DIRECTORY lib/ DESTINATION lib)
install(FILES ${CPACK_RESOURCE_FILE_README} ${CPACK_RESOURCE_FILE_LICENSE}
  DESTINATION docs)

include(CPack)
