#include <algorithm>
#include <catch.hpp>
#include <iterator>
#include "dummy.h"
#include "message_queue.hpp"

using test::dummy;

template<class... Ts>
bool is_empty(const message_queue<Ts...> &q)
{
  return q.get_container().empty();
}

template<class... Ts>
size_t get_size(const message_queue<Ts...> &q)
{
  return q.get_container().size();
}

template<class... Ts>
bool operator!=(const message_queue<Ts...> &lhs, const message_queue<Ts...> &rhs)
{
  test::disable_throw guard;
  return lhs.get_container() != rhs.get_container();
}

TEST_CASE( "First In First Out", "[message_queue]" )
{
  using pipe_type = message_queue<dummy>;
  using result_type = typename pipe_type::container_type;

  message_queue<dummy> pipe;
  REQUIRE(is_empty(pipe));

  dummy one{1}, two{2};
  pipe.send(one);
  pipe.send(two);
  REQUIRE(get_size(pipe) == 2);

  result_type result;
  pipe.receive_all(result);
  REQUIRE(result.size() == 2);
  REQUIRE(result[0] == one);
  REQUIRE(result[1] == two);
  REQUIRE(is_empty(pipe));
}

TEST_CASE( "Strong exception safety", "[message_queue]")
{
  using pipe_type = message_queue<dummy>;
  using result_type = typename pipe_type::container_type;

  pipe_type pipe;
  result_type result; 

  pipe.send(dummy{1});
  pipe.send(dummy{2});

  auto send    = [&](pipe_type &p) { p.send(dummy{3}); };
  auto receive = [&](pipe_type &p) { p.receive_all(result); };

  REQUIRE(test::strong_safety(pipe, send));
  REQUIRE(test::strong_safety(pipe, receive));
}
