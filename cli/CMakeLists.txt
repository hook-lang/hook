# ------------------------------------------------------------------------------
# CMakeLists.txt (CLI)
# ------------------------------------------------------------------------------

if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
endif()

if(NOT MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
endif()

add_executable(${PROJECT_NAME} 
  main.c)

target_link_directories(${PROJECT_NAME} PRIVATE ${LIBRARY_DIR})

target_link_libraries(${PROJECT_NAME} ${STATIC_LIB_TARGET})
