#include <algorithm>
#include <thread>
#include <type_traits>
#include "message_queue.hpp"
#include "universal_call.hpp"

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

  // We take future before moving packaged task into queue.
  template <class Callable, class Result = typename std::result_of<Callable()>::type>
  std::future<Result> execute(Callable &&callable)
  {
    std::packaged_task<Result()> task{std::forward<Callable>(callable)};
    std::future<Result> result = task.get_future();
    queue_.send(make_universal_call(std::move(task)));
    return result;
  }

private:
  using command_type = universal_call<void()>;
  using container_type = typename message_queue<command_type>::container_type;

  // Commands execution routine.
  void run()
  {
    container_type commands;
    while (!done_)
    {
      queue_.receive_all(commands);
      for_each(commands.begin(), commands.end(),
               [](const command_type &command) {
                 command();
               });
    }
  }

  message_queue<command_type> queue_;
  std::thread thread_;
  bool done_;
};