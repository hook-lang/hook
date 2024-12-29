# ------------------------------------------------------------------------------
# CMakeLists.txt (Core Modules)
# ------------------------------------------------------------------------------

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_DIR})
endif()

if(NOT MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR})
endif()

add_library(math_mod SHARED
  "math.c"
)

add_library(os_mod SHARED
  "os.c"
)

add_library(io_mod SHARED
  "io.c"
)

add_library(numbers_mod SHARED
  "numbers.c"
)

add_library(strings_mod SHARED
  "strings.c"
)

add_library(arrays_mod SHARED
  "arrays.c"
)

add_library(utf8_mod SHARED
  "utf8.c"
)

add_library(hashing_mod SHARED
  "hashing.c"
  "deps/crc32.c"
  "deps/crc64.c"
  "deps/md5.c"
  "deps/ripemd160.c"
  "deps/sha1.c"
  "deps/sha2.c"
  "deps/sha3.c"
)

add_library(encoding_mod SHARED
  "encoding.c"
  "deps/ascii85.c"
  "deps/base32.c"
  "deps/base58.c"
  "deps/base64.c"
)

add_library(socket_mod SHARED
  "socket.c"
)

add_library(json_mod SHARED
  "json.c"
  "deps/cJSON.c"
)

add_library(lists_mod SHARED
  "lists.c"
)

add_library(ini_mod SHARED
  "ini.c"
  "deps/ini.c"
)

add_library(selectors_mod SHARED
  "selectors.c"
)

target_link_libraries(math_mod      ${STATIC_LIB_TARGET})
target_link_libraries(os_mod        ${STATIC_LIB_TARGET})
target_link_libraries(io_mod        ${STATIC_LIB_TARGET})
target_link_libraries(numbers_mod   ${STATIC_LIB_TARGET})
target_link_libraries(strings_mod   ${STATIC_LIB_TARGET})
target_link_libraries(arrays_mod    ${STATIC_LIB_TARGET})
target_link_libraries(utf8_mod      ${STATIC_LIB_TARGET})
target_link_libraries(hashing_mod   ${STATIC_LIB_TARGET})
target_link_libraries(encoding_mod  ${STATIC_LIB_TARGET})
target_link_libraries(socket_mod    ${STATIC_LIB_TARGET})
target_link_libraries(json_mod      ${STATIC_LIB_TARGET})
target_link_libraries(lists_mod     ${STATIC_LIB_TARGET})
target_link_libraries(ini_mod       ${STATIC_LIB_TARGET})
target_link_libraries(selectors_mod ${STATIC_LIB_TARGET})

if(WIN32)
  target_link_libraries(socket_mod ws2_32)
  target_link_libraries(selectors_mod ws2_32)
endif()

set_target_properties(math_mod      PROPERTIES PREFIX "")
set_target_properties(os_mod        PROPERTIES PREFIX "")
set_target_properties(io_mod        PROPERTIES PREFIX "")
set_target_properties(numbers_mod   PROPERTIES PREFIX "")
set_target_properties(strings_mod   PROPERTIES PREFIX "")
set_target_properties(arrays_mod    PROPERTIES PREFIX "")
set_target_properties(utf8_mod      PROPERTIES PREFIX "")
set_target_properties(hashing_mod   PROPERTIES PREFIX "")
set_target_properties(encoding_mod  PROPERTIES PREFIX "")
set_target_properties(socket_mod    PROPERTIES PREFIX "")
set_target_properties(json_mod      PROPERTIES PREFIX "")
set_target_properties(lists_mod     PROPERTIES PREFIX "")
set_target_properties(ini_mod       PROPERTIES PREFIX "")
set_target_properties(selectors_mod PROPERTIES PREFIX "")
