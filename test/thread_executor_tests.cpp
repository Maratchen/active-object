#include <active/thread_executor.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace active;
using namespace std;

TEST_CASE( "Multithreading", "[thread_executor]" )
{
    thread_executor executor;
    auto thread_id = executor.execute([] () {
        return this_thread::get_id();
    });
    REQUIRE(thread_id.get() != this_thread::get_id());
}

TEST_CASE( "Sequentially-consistent", "[thread_executor]" )
{
    thread_executor executor;
    auto f1 = executor.execute([](auto a, auto b) { return a + b; }, 1, 2);
    auto f2 = executor.execute([](auto&& f1, auto c) { return f1.get() + c; }, std::move(f1), 3);
    REQUIRE(f2.get() == 6);
}

TEST_CASE( "Exception propagation", "[thread_executor]" )
{
    future<void> result;
    REQUIRE_NOTHROW(result = thread_executor().execute([]() { throw std::exception(); }));
    REQUIRE_THROWS(result.get());
}

TEST_CASE( "Prevent infinite loop with zero batch size", "[thread_executor]" )
{
    auto executor = thread_executor({ .max_batch_size = 0 });
    auto result = executor.execute([]() { return 42; });
    REQUIRE(result.wait_for(std::chrono::seconds(1)) != std::future_status::timeout);
    REQUIRE(result.get() == 42);
}