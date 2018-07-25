#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>
#include "message_queue.hpp"

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
    using task_type = std::packaged_task<Result()>;
    auto task = std::make_shared<task_type>(std::forward<Function>(command));
    queue_.send([task]() { (*task)(); });
    return task->get_future();
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