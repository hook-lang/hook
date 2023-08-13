# ------------------------------------------------------------------------------
# CMakeLists.txt (Root)
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

project(hook
  DESCRIPTION "The Hook Programming Language"
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/../bin)
set(LIBRARY_DIR ${CMAKE_BINARY_DIR}/../lib)
set(PACKAGE_DIR ${CMAKE_BINARY_DIR}/../package)

include_directories(/usr/local/include)
include_directories(include)

link_directories(/usr/local/lib)

if(MSVC)
  add_compile_options(/W4)
endif()

if(NOT MSVC)
  add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options("$<$<CONFIG:Debug>:-fsanitize=address>")
  add_compile_options("$<$<CONFIG:Debug>:-fprofile-arcs>")
  add_compile_options("$<$<CONFIG:Debug>:-ftest-coverage>")
  add_link_options("$<$<CONFIG:Debug>:-fsanitize=address>")
  add_link_options("$<$<CONFIG:Debug>:-fprofile-arcs>")
  add_link_options("$<$<CONFIG:Debug>:-ftest-coverage>")
endif()

# ------------------------------------------------------------------------------
# Libraries, CLI, Core Modules, and Extensions
# ------------------------------------------------------------------------------

set(STATIC_LIB_TARGET "static_lib")
set(SHARED_LIB_TARGET "shared_lib")

add_subdirectory(src)

add_subdirectory(cli)

add_subdirectory(core)

if(BUILD_EXTENSIONS)
  message("Building with extensions")
  add_subdirectory(extensions)
endif()

# ------------------------------------------------------------------------------
# Pack and Install
# ------------------------------------------------------------------------------

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(FORMAT TGZ)
set(CPACK_GENERATOR ${FORMAT})
set(CPACK_SOURCE_GENERATOR ${FORMAT})

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

if(MSVC)
  set(EXECUTABLE "${PROJECT_NAME}.exe")
endif()

if(NOT MSVC)
  set(EXECUTABLE ${PROJECT_NAME})
endif()

install(
  FILES "bin/${EXECUTABLE}"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
              GROUP_EXECUTE GROUP_READ
              WORLD_EXECUTE WORLD_READ
  DESTINATION bin
)

install(DIRECTORY examples/     DESTINATION examples)
install(DIRECTORY include/      DESTINATION include)
install(DIRECTORY lib/          DESTINATION lib)
install(FILES LICENSE README.md DESTINATION docs)

include(CPack)
