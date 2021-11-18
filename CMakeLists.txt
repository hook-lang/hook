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
set(PACKAGE_DIR ${CMAKE_BINARY_DIR}/../package)

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
  src/fs.c
  src/hash.c
  src/library.c
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
  src/library.c
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
  src/library.c
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
  src/library.c
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
  src/library.c
  src/memory.c
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
  src/error.c
  src/hash.c
  src/library.c
  src/memory.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

if(MSVC)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
endif(MSVC)

add_library(sqlite SHARED
  src/array.c
  src/callable.c
  src/chunk.c
  src/error.c
  src/hash.c
  src/library.c
  src/memory.c
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
  src/url.c
  src/error.c
  src/hash.c
  src/library.c
  src/memory.c
  src/string.c
  src/struct.c
  src/userdata.c
  src/value.c
  src/vm.c)

if(MSVC)
  target_link_libraries(sqlite PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(url PRIVATE CURL::libcurl)
else(MSVC)
  target_link_libraries(sqlite sqlite3)
  target_link_libraries(url curl)
endif(MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(fmt TGZ)
if(WIN32)
  set(fmt ZIP)
endif()
set(CPACK_GENERATOR ${fmt})
set(CPACK_SOURCE_GENERATOR ${fmt})

set(CPACK_OUTPUT_FILE_PREFIX ${PACKAGE_DIR})
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

string(TOLOWER "${PROJECT_NAME}" project)
string(TOLOWER "${CMAKE_SYSTEM_NAME}" sys)
string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" arch)

set(CPACK_PACKAGE_FILE_NAME "${project}-${PROJECT_VERSION}-${sys}-${arch}")
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
