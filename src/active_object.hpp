#include <functional>
#include <future>
#include <memory>
#include <thread>
#include "message_queue.hpp"

class active_object
{
public:
  typedef std::function<void()> command_type;

  active_object() : done_(false)
  {
    thread_ = std::thread([&]() { run(); });
  }

  // Sends command, which will interrupt processing routine,
  // and waits until the thread will be finished.
  ~active_object()
  {
    command_queque_.send([&]() { done_ = true; });
    thread_.join();
  }

  // Copying is not allowed
  active_object(const active_object&) = delete;
  active_object& operator= (const active_object&) = delete;

  // TODO: Make up how to get rid of carrying promise by shared_ptr.
  // The problem is copying of it is not allowed but movint it into closure doesn't help
  template<typename Fun>
  std::future<void> execute(Fun command)
  {
    auto result = std::make_shared<std::promise<void>>();
    auto future = result->get_future();
    command_queque_.send([result, command]() mutable {
      try {
        command();
        result->set_value();
      }
      catch(...) {
        result->set_exception(std::current_exception());
      }
    });
    return future;
  }

private:
  // Commands execution routine.
  void run()
  {
    while(!done_) {
      const auto command = command_queque_.receive();
      command();
    }
  }

  message_queue<command_type> command_queque_;
  std::thread thread_;
  bool done_;
};