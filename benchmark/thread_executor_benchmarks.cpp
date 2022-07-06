#include <active/thread_executor.hpp>
#include <benchmark/benchmark.h>

auto executor = std::unique_ptr<active::thread_executor>();

static void BM_ThreadExecutor(benchmark::State& state) {
    if (state.thread_index() == 0) {
        executor = std::make_unique<active::thread_executor>(
            active::thread_executor::construction_options{
                .max_batch_size = static_cast<std::size_t>(state.range(0))
            }
        );
    }

    const auto max_inflight_count = state.range(1);
    const auto increment_value = [](auto value) { return value + 1; };
    const auto increment_result = [](auto&& result) { return result.get() + 1; };

    auto value = 0;
    auto result = std::future<int>();
    auto inflight_count = int64_t();

    for (auto _ : state) {
        if (max_inflight_count == inflight_count) {
            value = result.get();
            inflight_count = 0;
        }
        if (!result.valid()) {
            result = executor->execute(increment_value, value);
        } else {
            result = executor->execute(increment_result, std::move(result));
        }
        ++inflight_count;
    }

    assert(result.get() == state.items_processed());

    if (state.thread_index() == 0) {
        executor.release();
    }
}

// Register the function as a benchmark
BENCHMARK(BM_ThreadExecutor)
    ->Threads(std::thread::hardware_concurrency())
    ->Ranges({{1, 1<<10}, {8, 8<<10}});

// Run the benchmark
BENCHMARK_MAIN();
