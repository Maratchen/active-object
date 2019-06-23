cmake_minimum_required(VERSION 3.8)

project(active_object)
enable_testing()

if(EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

add_library(active INTERFACE)

target_include_directories(active INTERFACE
    "${PROJECT_SOURCE_DIR}/include/"
)

target_compile_features(active INTERFACE
    cxx_std_11
)

add_subdirectory(test)
