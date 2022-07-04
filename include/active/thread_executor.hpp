#pragma once

#ifndef ACTIVE_THREAD_EXECUTOR_HPP
#define ACTIVE_THREAD_EXECUTOR_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
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
            execute([this]() { done_ = true; });
            thread_.join();
        }

        /**
         * Puts a task into the queue and wakes up the consumer thread.
         */
        template<class Function, class... Args>
        std::future<typename std::invoke_result<Function, Args...>::type>
        execute(Function&& fn, Args&&... args) {
            using ResultType = typename std::invoke_result<Function, Args...>::type;
            auto task = std::packaged_task<ResultType(Args...)>{std::forward<Function>(fn)};
            auto result = task.get_future();
            bool notify = false;

            {
                std::lock_guard<std::mutex> sync(mutex_);
                notify = queue_.empty();
                queue_.emplace(
                    [
                        task = std::move(task), 
                        params = std::make_tuple(std::forward<Args>(args)...)
                    ]() mutable { 
                        std::apply([&](auto&... args) {
                            task(std::forward<Args>(args)...);
                        }, params);
                    }
                );
            }

            if (notify) {
                ready_.notify_one();
            }

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