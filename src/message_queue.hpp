#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

// Provides delivering messages from producer to consumer in thread-safe way.
template<
  class Message,
  class Synchronize = std::mutex,
  class Container = std::queue<Message>>
class message_queue
{
public:
  typedef Message value_type;
  typedef Container container_type;
  typedef Synchronize synchronize_type;

  // Puts an incoming message into the queue.
  // Wakes up the consumer if it's necessary.
  template<class T>
  void send(T&& message)
  {
    std::lock_guard<synchronize_type> lock(sync_);
    container_.push(std::forward<T>(message));
    condition_.notify_one();
  }

  // Waits till a message will be added and then retrieves it.
  // Receiver takes the message by reference to guarantee exception safety.
  void receive(value_type &message)
  {
    std::unique_lock<synchronize_type> lock(sync_);
    condition_.wait(lock, [&]() { return !container_.empty(); });
    message = std::move(container_.front());
    container_.pop();
  }

private:
  synchronize_type sync_;
  container_type container_;
  std::condition_variable condition_;
};