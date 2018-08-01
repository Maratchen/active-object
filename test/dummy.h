#pragma once
#include "exception_safety.h"

namespace test
{
  struct dummy
  {
    dummy()
    {
      this_can_throw();
    }

    dummy(int value)
    {
      this_can_throw();
      value_ = value;
    }

    dummy(dummy &&other)
    {
      this_can_throw();
      std::swap(value_, other.value_);
    }

    dummy(const dummy &other)
    {
      this_can_throw();
      value_ = other.value_;
    }

    dummy& operator=(const dummy &other)
    {
      this_can_throw();
      value_ = other.value_;
      return *this;
    }

    dummy& operator=(dummy &&other)
    {
      this_can_throw();
      std::swap(value_, other.value_);
      return *this;
    }

    int value_ = 0;
  };

  inline bool operator==(const dummy &lhs, const dummy &rhs)
  {
    return lhs.value_ == rhs.value_;
  }
}
