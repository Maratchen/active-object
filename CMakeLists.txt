cmake_minimum_required(VERSION 3.14)
project(active_object)

option(USE_SMALL_OBJECT_OPTIMIZATION "Use Small Object Optimization for unique function" ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

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

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        605a34765aa5d5ecbf476b4598a862ada971b0cc # v3.0.1
)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        0d98dba29d66e93259db7daa53a9327df767a415 # v1.6.1
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
    set(PROJECT_IS_TOP_LEVEL TRUE)
else()
    set(PROJECT_IS_TOP_LEVEL FALSE)
endif()

option(ACTIVE_BUILD_BENCHMARK "Build active library benchmark" ${PROJECT_IS_TOP_LEVEL})
option(ACTIVE_BUILD_TESTS "Build active library unit tests" ${PROJECT_IS_TOP_LEVEL})

if(${ACTIVE_BUILD_BENCHMARK})
  add_subdirectory(benchmark)
endif()

if(${ACTIVE_BUILD_TESTS})
  enable_testing()
  add_subdirectory(test)
endif()
