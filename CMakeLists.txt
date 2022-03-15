cmake_minimum_required(VERSION 3.0)

project(hook
  DESCRIPTION "The Hook interpreter"
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_VERBOSE_MAKEFILE OFF)

if(NOT MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address -fprofile-arcs -ftest-coverage")
endif()

include_directories(/usr/local/include)
include_directories(include)
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
  src/hook_array.c
  src/hook_builtin.c
  src/hook_callable.c
  src/hook_chunk.c
  src/hook_utils.c
  src/hook_compiler.c
  src/hook_dump.c
  src/hook_error.c
  src/hook_hash.c
  src/hook_iterator.c
  src/main.c
  src/hook_memory.c
  src/hook_module.c
  src/hook_range.c
  src/hook_scanner.c
  src/hook_string_map.c
  src/hook_string.c
  src/hook_struct.c
  src/hook_userdata.c
  src/hook_value.c
  src/hook_vm.c)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
  target_link_libraries(${PROJECT_NAME} dl)
endif()

add_subdirectory(core)
add_subdirectory(modules)

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
install(DIRECTORY examples/ DESTINATION examples)
install(DIRECTORY lib/ DESTINATION lib)
install(FILES ${CPACK_RESOURCE_FILE_README} ${CPACK_RESOURCE_FILE_LICENSE}
  DESTINATION docs)

include(CPack)
