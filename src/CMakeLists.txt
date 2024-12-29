# ------------------------------------------------------------------------------
# CMakeLists.txt (Libraries)
# ------------------------------------------------------------------------------

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

if(NOT MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_DIR})
endif()

set(SOURCES
  "array.c"
  "builtin.c"
  "callable.c"
  "chunk.c"
  "utils.c"
  "compiler.c"
  "dump.c"
  "iterable.c"
  "iterator.c"
  "lexer.c"
  "memory.c"
  "module.c"
  "range.c"
  "record.c"
  "string.c"
  "struct.c"
  "userdata.c"
  "value.c"
  "vm.c"
)

add_library(${STATIC_LIB_TARGET} STATIC ${SOURCES})
add_library(${SHARED_LIB_TARGET} SHARED ${SOURCES})

set_target_properties(${STATIC_LIB_TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${SHARED_LIB_TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if(NOT MSVC)
  target_link_libraries(${STATIC_LIB_TARGET} m)
  target_link_libraries(${STATIC_LIB_TARGET} dl)
  target_link_libraries(${SHARED_LIB_TARGET} m)
  target_link_libraries(${SHARED_LIB_TARGET} dl)
endif()
