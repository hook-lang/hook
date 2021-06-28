cmake_minimum_required(VERSION 3.0)

project(hook VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(BINARY_DIR ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR}) 

add_executable(${PROJECT_NAME} 
  src/array.c
  src/chunk.c
  src/compiler.c
  src/disasm.c
  src/error.c
  src/main.c
  src/memory.c
  src/scanner.c
  src/string.c
  src/value.c
  src/vm.c)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
endif(NOT MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
