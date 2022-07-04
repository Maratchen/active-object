find_package(benchmark REQUIRED)

add_executable(benchmarks
    thread_executor_benchmarks.cpp
)

target_link_libraries(benchmarks PRIVATE
    benchmark::benchmark
    active
)