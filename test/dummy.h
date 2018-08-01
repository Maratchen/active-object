#pragma once

namespace test
{
  struct dummy
  {
    dummy() : value_(0) {}
    dummy(int value) : value_(value) {}
    dummy(const dummy &other) : value_(other.value_) {}

    int value_;
  };

  inline bool operator==(const dummy &lhs, const dummy &rhs)
  {
    return lhs.value_ == rhs.value_;
  }
}
