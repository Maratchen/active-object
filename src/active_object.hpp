#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>
#include "message_queue.hpp"

namespace details
{
// The type of return value has an influence on the way
// how the promise should be processed.
template <class Result>
struct invoke_traits;

// Any exception must be caught and returned to the client thread.
template <class Function, class Result>
void safe_invoke(Function &command, std::promise<Result> &result)
{
  try
  {
    invoke_traits<Result>::invoke(command, result);
  }
  catch (...)
  {
    result.set_exception(std::current_exception());
  }
}

template <class Result>
struct invoke_traits
{
  template <class Function>
  static void invoke(Function &command, std::promise<Result> &result)
  {
    result.set_value(command());
  }
};

template <>
struct invoke_traits<void>
{
  template <class Function>
  static void invoke(Function &command, std::promise<void> &result)
  {
    command();
    result.set_value();
  }
};

} // namespace details

// The class decouples method execution from method invocation
// for objects that each reside in their own thread of control.
class active_object
{
public:
  active_object() : done_(false)
  {
    thread_ = std::thread([&]() { run(); });
  }

  // Sends command, which will interrupt processing routine,
  // and waits until the thread will be finished.
  ~active_object()
  {
    queue_.send([&]() { done_ = true; });
    thread_.join();
  }

  // Copying is not allowed
  active_object(const active_object &) = delete;
  active_object &operator=(const active_object &) = delete;

  // We have to use shared_ptr for hanging a promise
  // because the closure will be copied in std::function constructor anyway.
  template <class Function, class Result = typename std::result_of<Function()>::type>
  std::future<Result> execute(Function &&command)
  {
    auto result = std::make_shared<std::promise<Result>>();
    queue_.send([command, result]() mutable {
      details::safe_invoke<Function, Result>(command, *result);
    });
    return result->get_future();
  }

private:
  using command_type = std::function<void()>;

  // Commands execution routine.
  void run()
  {
    command_type command;
    while (!done_)
    {
      queue_.receive(command);
      command();
    }
  }

  message_queue<command_type> queue_;
  std::thread thread_;
  bool done_;
};