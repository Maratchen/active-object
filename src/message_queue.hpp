#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

// Provides delivering messages from many producers to one consumer
// in thread-safe way.
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

  // Makes empty queue
  message_queue() = default;

  // Makes copies messages only, not synchronization objects.
  message_queue(const message_queue &other)
  {
    container_ = other.get_container();
  }

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
  // unique_lock is required to get unlock while waiting.
  void receive(value_type &message)
  {
    std::unique_lock<synchronize_type> lock(sync_);
    condition_.wait(lock, [&]() { return !container_.empty(); });
    message = std::move(container_.front());
    container_.pop();
  }

  // Makes a copy of the internal container (for unit tests)
  container_type get_container() const
  {
    std::lock_guard<synchronize_type> lock(sync_);
    return container_;
  }

private:
  synchronize_type mutable sync_;
  container_type container_;
  std::condition_variable condition_;
};