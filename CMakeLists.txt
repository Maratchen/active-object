cmake_minimum_required(VERSION 3.8)
project(active_object)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES catch2/2.13.0
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

add_library(active INTERFACE)

target_include_directories(active INTERFACE
    "${PROJECT_SOURCE_DIR}/include/"
)

target_link_libraries(active INTERFACE
    Threads::Threads
)

target_compile_features(active INTERFACE
    cxx_std_11
)

enable_testing()
add_subdirectory(test)
