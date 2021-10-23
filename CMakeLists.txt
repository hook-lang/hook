cmake_minimum_required(VERSION 3.0)

project(hook VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address -fprofile-arcs -ftest-coverage")
endif(NOT MSVC)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/../bin)
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
else(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
endif(MSVC)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(${PROJECT_NAME}
  src/array.c
  src/builtin.c
  src/callable.c
  src/chunk.c
  src/compiler.c
  src/disasm.c
  src/error.c
  src/hash.c
  src/main.c
  src/memory.c
  src/scanner.c
  src/string.c
  src/struct.c
  src/value.c
  src/vm.c)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
endif(NOT MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
