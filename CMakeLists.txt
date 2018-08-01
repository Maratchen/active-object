cmake_minimum_required(VERSION 3.8)

project(active_object)
enable_testing()

if(EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

add_subdirectory(src)
add_subdirectory(test)
