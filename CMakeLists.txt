cmake_minimum_required(VERSION 2.8.12)
project(hook C)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR}) 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR}) 

add_executable(hook 
  src/chunk.c
  src/compiler.c
  src/disasm.c
  src/error.c
  src/main.c
  src/memory.c
  src/scanner.c
  src/stack.c
  src/string.c
  src/vm.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
