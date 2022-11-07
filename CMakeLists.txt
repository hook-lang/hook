cmake_minimum_required(VERSION 3.13)

project(hook
  DESCRIPTION "The Hook interpreter"
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE OFF)

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

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options("$<$<CONFIG:Debug>:-fsanitize=address>")
  add_compile_options("$<$<CONFIG:Debug>:-fprofile-arcs>")
  add_compile_options("$<$<CONFIG:Debug>:-ftest-coverage>")
  add_link_options("$<$<CONFIG:Debug>:-fsanitize=address>")
  add_link_options("$<$<CONFIG:Debug>:-fprofile-arcs>")
  add_link_options("$<$<CONFIG:Debug>:-ftest-coverage>")
endif()

add_executable(${PROJECT_NAME}
  src/hk_array.c
  src/hk_builtin.c
  src/hk_callable.c
  src/hk_chunk.c
  src/hk_utils.c
  src/hk_compiler.c
  src/hk_dump.c
  src/hk_error.c
  src/hk_iterable.c
  src/hk_iterator.c
  src/main.c
  src/hk_memory.c
  src/hk_module.c
  src/hk_range.c
  src/hk_scanner.c
  src/hk_string_map.c
  src/hk_string.c
  src/hk_struct.c
  src/hk_userdata.c
  src/hk_value.c
  src/hk_vm.c)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} m)
  target_link_libraries(${PROJECT_NAME} dl)
endif()

add_subdirectory(core)

if(BUILD_EXTENSIONS)
  message("Building with extensions")
  add_subdirectory(extensions)
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

if(MSVC)
  set(EXECUTABLE "${PROJECT_NAME}.exe")
else()
  set(EXECUTABLE ${PROJECT_NAME})
endif()

install(
  FILES "bin/${EXECUTABLE}"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
              GROUP_EXECUTE GROUP_READ
              WORLD_EXECUTE WORLD_READ
  DESTINATION bin
)

install(DIRECTORY examples/ DESTINATION examples)
install(DIRECTORY lib/ DESTINATION lib)
install(FILES ${CPACK_RESOURCE_FILE_README} ${CPACK_RESOURCE_FILE_LICENSE}
  DESTINATION docs)

include(CPack)
