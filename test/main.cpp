#include <iostream>
#include "active_object.hpp"

void command_one()
{
  std::cout << std::this_thread::get_id();
  std::cout << ": command one" << std::endl;
}

int command_two(int a)
{
  std::cout << std::this_thread::get_id();
  std::cout << ": command two" << std::endl;
  return a + a;
}

int main(int argc, const char *argv[])
{
  active_object object;

  auto result_one = object.execute(command_one);
  auto result_two = object.execute(std::bind(command_two, 42));

  std::cout << std::this_thread::get_id();
  std::cout << ": other work" << std::endl;

  result_one.get();
  std::cout << "result: " << result_two.get();

  return 0;
}