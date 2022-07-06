#pragma once

#ifndef ACTIVE_THREAD_EXECUTOR_HPP
#define ACTIVE_THREAD_EXECUTOR_HPP

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

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
         * A type erasure for tasks to execute.
         */
        using task_type = unique_function<void()>;

        /**
         * Creates an execution thread.
         */
        explicit thread_executor(std::size_t max_batch_size = 64) {
            thread_ = std::thread([this] (std::size_t max_batch_size) {
                auto tasks = std::vector<task_type>();
                tasks.reserve(std::max(max_batch_size, std::size_t(1)));

                while (!done_) {
                    std::unique_lock<std::mutex> sync(mutex_);
                    ready_.wait(sync, [&] () { return !queue_.empty(); });

                    const auto count = std::min(queue_.size(), tasks.capacity());
                    std::move(queue_.begin(), queue_.begin() + count, std::back_inserter(tasks));
                    queue_.erase(queue_.begin(), queue_.begin() + count);

                    sync.unlock();

                    for (auto& task : tasks) {
                        task();
                    }

                    tasks.clear();
                }
            }, max_batch_size);
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
            schedule([this]() { done_ = true; });
            thread_.join();
        }

        /**
         * Puts a task into the queue and wakes up the consumer thread.
         */
        void schedule(task_type task) {
            bool there_is_need_to_notify = false;
            {
                std::lock_guard<std::mutex> sync(mutex_);
                there_is_need_to_notify = queue_.empty();
                queue_.push_back(std::move(task));
            }
            if (there_is_need_to_notify) {
                ready_.notify_one();
            }
        }

        /**
         * Schedules function for execution on the consumer thread
         * and returns a future object as the result.
         */
        template<class Function, class... Args>
        std::future<typename std::invoke_result<Function, Args...>::type>
        execute(Function&& fn, Args&&... args) {
            using ResultType = typename std::invoke_result<Function, Args...>::type;
            auto task = std::packaged_task<ResultType(Args...)>{std::forward<Function>(fn)};
            auto result = task.get_future();

            schedule([
                task = std::move(task), 
                args = std::make_tuple(std::forward<Args>(args)...)
            ]() mutable { 
                std::apply([&](auto&... args) {
                    task(std::forward<Args>(args)...);
                }, args);
            });

            return result;
        }

    private:
        std::deque<task_type> queue_;
        std::condition_variable ready_;
        std::mutex mutable mutex_;
        std::thread thread_;
        bool done_ = false;
    };
}

#endif // ACTIVE_THREAD_EXECUTOR_HPP