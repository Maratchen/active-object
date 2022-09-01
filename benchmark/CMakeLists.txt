set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)

add_executable(benchmarks
    thread_executor_benchmarks.cpp
)

target_link_libraries(benchmarks PRIVATE
    benchmark::benchmark
    active
)