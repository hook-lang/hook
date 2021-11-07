cmake_minimum_required(VERSION 3.0)

project(hook VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address -fprofile-arcs -ftest-coverage")
endif(NOT MSVC)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/../bin)
set(LIBRARY_DIR ${CMAKE_BINARY_DIR}/../lib)

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
else(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR})
endif(MSVC)

add_executable(${PROJECT_NAME}
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/compiler.c
  src/dump.c
  src/error.c
  src/hash.c
  src/main.c
  src/memory.c
  src/scanner.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
  target_link_libraries(${PROJECT_NAME} dl)
endif(NOT MSVC)

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif(MSVC)

add_library(math SHARED
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/compiler.c
  src/error.c
  src/hash.c
  src/math.c
  src/memory.c
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
  src/compiler.c
  src/error.c
  src/hash.c
  src/memory.c
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
  src/compiler.c
  src/error.c
  src/hash.c
  src/io.c
  src/memory.c
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
  src/error.c
  src/hash.c
  src/memory.c
  src/string.c
  src/strings.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

add_library(sqlite SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/error.c
  src/hash.c
  src/memory.c
  src/sqlite.c
  src/sqlite3.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
