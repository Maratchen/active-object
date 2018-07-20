#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

// Provides delivering messages from producer to consumer in thread-safe way
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
  void send(value_type message)
  {
    std::lock_guard<synchronize_type> lock(sync_);
    container_.push(std::move(message));
    condition_.notify_one();
  }

  // Takes away the first message.
  // When there is nothing it will wait for an incoming message.
  value_type receive()
  {
    std::unique_lock<synchronize_type> lock(sync_);
    condition_.wait(lock, [&]() { return !container_.empty(); });
    value_type message = container_.front();
    container_.pop();
    return message;
  }

private:
  synchronize_type sync_;
  container_type container_;
  std::condition_variable condition_;
};