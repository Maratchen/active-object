#include <iostream>
#include "active_object.hpp"

void command_one()
{
  std::cout << "command one" << std::endl;
}

int command_two(int a)
{
  std::cout << "command two" << std::endl;
  return a + a;
}

int main(int argc, const char* argv[])
{
  active_object object;
  auto result_one = object.execute(command_one);
  //auto result_two = object.execute(std::bind(command_one, 42));
  result_one.get();

  return 0;
}