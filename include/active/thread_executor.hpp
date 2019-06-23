#pragma once

#ifndef ACTIVE_THREAD_EXECUTOR_HPP
#define ACTIVE_THREAD_EXECUTOR_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>

#include "unique_function.hpp"

namespace active
{
    /**
     * Provides a synchronyzed interface
     * to execute task on another thread.
     */
    class thread_executor
    {
    public:
        /**
         * Creates an execution thread.
         */
        thread_executor() : done_(false) {
            thread_ = std::thread([&] () {
                while (!done_) {
                    std::unique_lock<std::mutex> sync(mutex_);
                    ready_.wait(sync, [&] () { return !queue_.empty(); });

                    auto task = std::move(queue_.front());
                    queue_.pop();

                    sync.unlock();
                    task();
                }
            });
        }

        /**
         * Copying is not allowed
         */
        thread_executor(const thread_executor&) = delete;
        thread_executor& operator=(const thread_executor&) = delete;

        /**
         * Sends command to interrupt processing routine,
         * and waits until the thread will be finished.
         */
        ~thread_executor() {
            post([&] () { done_ = true; });
            thread_.join();
        }

        /**
         * Puts a task into the queue and wakes up the consumer thread.
         */
        template<class Callable, class... Args>
        std::future<typename std::result_of<Callable(Args...)>::type>
        post(Callable&& callable, Args&&... args) {
            using ResultType = typename std::result_of<Callable(Args...)>::type;
            std::packaged_task<ResultType(Args...)> task(std::forward<Callable>(callable));
            auto result = task.get_future();

            std::lock_guard<std::mutex> sync(mutex_);
            queue_.emplace(std::bind(std::move(task), std::forward<Args>(args)...));
            ready_.notify_one();

            return result;
        }

    private:
        std::queue<unique_function<void()>> queue_;
        std::condition_variable ready_;
        std::mutex mutable mutex_;
        std::thread thread_;
        bool done_;
    };
}

#endif // ACTIVE_THREAD_EXECUTOR_HPP