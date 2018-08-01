#pragma once

namespace test
{
  extern int g_throw_counter;

  inline void this_can_throw()
  {
    if(g_throw_counter == 0)
      throw 0;

    if(g_throw_counter >= 0)
      g_throw_counter--;
  }

  // Turns off throwing an exception during operation
  class disable_throw
  {
  public:
    disable_throw() : count_(g_throw_counter)
    {
      g_throw_counter = -1;
    }

    ~disable_throw()
    {
      g_throw_counter = count_;
    }

  private:
    int count_;
  };

  template <class T, class Operation>
  bool strong_safety(const T &value, Operation operate)
  {
    disable_throw gurard;
    T duplicate = value;
    for(int next_throw_count = 0;; ++next_throw_count)
    {
      try
      {
        g_throw_counter = next_throw_count;
        operate(duplicate);
        break;
      }
      catch(...)
      {
        if(value != duplicate)
          return false;
      }
    }
    return true;
  }
}