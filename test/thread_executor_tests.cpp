#include <active/thread_executor.hpp>
#include <catch2/catch.hpp>

using namespace active;
using namespace std;

TEST_CASE( "Multithreading", "[thread_executor]" )
{
    thread_executor executor;
    auto thread_id = executor.post([] () {
        return this_thread::get_id();
    });
    REQUIRE(thread_id.get() != this_thread::get_id());
}

void test_and_set(int& value, int expected, int desired) {
    if (value == expected) {
        value = desired;
    }
}

TEST_CASE( "Sequentially-consistent", "[thread_executor]" )
{
    int counter = 0;
    {
        thread_executor executor;
        executor.post(&test_and_set, std::ref(counter), 0, 1);
        executor.post(&test_and_set, std::ref(counter), 1, 2);
        executor.post(&test_and_set, std::ref(counter), 2, 3);
    }
    REQUIRE(3 == counter);
}

TEST_CASE( "Exception propagation", "[thread_executor]" )
{
    future<void> result;
    REQUIRE_NOTHROW(result = thread_executor().post([]() { throw 0; }));
    REQUIRE_THROWS(result.get());
}