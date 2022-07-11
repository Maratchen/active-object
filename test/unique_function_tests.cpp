#include <active/unique_function.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace active;
using namespace std;

TEST_CASE( "Default-constructible", "[unique_function]" )
{
    auto fun = unique_function<void()>();
    REQUIRE(!fun);
}

TEST_CASE( "Move-constructible", "[unique_function]" )
{
    auto unique = std::make_unique<int>(42);
    auto lambda = [unique = std::move(unique)]() { return *unique; };
    auto fun = unique_function<int()>(std::move(lambda));
    REQUIRE(fun() == 42);
}

TEST_CASE( "Move-assignable", "[unique_function]" )
{
    auto fun = unique_function<int(int)>();
    fun = [](int x) { return x + 1; };
    REQUIRE(fun(2) == 3);
}

TEST_CASE( "Swapable", "[unique_function]" )
{
    auto f1 = unique_function<int()>([]() { return 1; });
    auto f2 = unique_function<int()>([]() { return 2; });
    using namespace std;
    swap(f1, f2);
    REQUIRE((f1() == 2 && f2() == 1));
}