#include <catch.hpp>
#include "active_object.hpp"

using namespace std;

auto command_one() -> decltype(this_thread::get_id())
{
  return this_thread::get_id();
}

int command_two(int a)
{
  return a + a;
}

TEST_CASE( "Smoke test", "[active_object]" )
{
  int dummy = 0;
  active_object object;

  object.execute([&dummy]() {
    dummy = 42;
  });

  auto result_one = object.execute(command_one);
  auto result_two = object.execute(std::bind(command_two, 2));

  REQUIRE(result_one.get() != this_thread::get_id());
  REQUIRE(result_two.get() == 4);
  REQUIRE(42 == dummy);
}