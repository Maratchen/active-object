cmake_minimum_required(VERSION 3.8)
project(active_object)

option(USE_SMALL_OBJECT_OPTIMIZATION "Use a SOO implementation of unique function" ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()


include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(REQUIRES benchmark/1.6.1 catch2/3.0.1
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

add_library(active INTERFACE)

target_include_directories(active INTERFACE
    "${PROJECT_SOURCE_DIR}/include/"
)

target_link_libraries(active INTERFACE
    Threads::Threads
)

target_compile_features(active INTERFACE
    cxx_std_20
)

target_compile_definitions(active INTERFACE
    $<$<BOOL:${USE_SMALL_OBJECT_OPTIMIZATION}>:SOO_UNIQUE_FUNCTION=1>
)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
