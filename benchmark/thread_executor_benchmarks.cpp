#include <active/thread_executor.hpp>
#include <benchmark/benchmark.h>

 auto executor = std::unique_ptr<active::thread_executor>();

static void BM_ThreadExecutor(benchmark::State& state) {
    if (state.thread_index() == 0) {
        executor = std::make_unique<active::thread_executor>();
    }

    auto sum_numbers = [](auto a, auto b) { return a + b; };
    auto sum_futures = [](auto a, auto b) { return a.get() + b.get(); };

    for (auto _ : state) {
        auto f1 = executor->execute(sum_numbers, 1, 2);
        auto f2 = executor->execute(sum_numbers, 3, 4);
        auto f3 = executor->execute(sum_futures, std::move(f1), std::move(f2));
        assert(f3.get() == 10);
    }

    if (state.thread_index() == 0) {
        executor.release();
    }
}

// Register the function as a benchmark
BENCHMARK(BM_ThreadExecutor)->Threads(std::thread::hardware_concurrency());

// Run the benchmark
BENCHMARK_MAIN();
