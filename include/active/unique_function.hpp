#pragma once

#ifndef ACTIVE_UNIQUE_FUNCTION_HPP
#define ACTIVE_UNIQUE_FUNCTION_HPP

namespace active
{
    /**
     * general template form
     */
    template<class Fn>
    class unique_function;

    /**
     * Represents noncopyable std::function
     */
    template<class Result, class... Args>
    class unique_function<Result(Args...)>
    {
    public:
        template<class Fn>
        unique_function(Fn&& fn) noexcept
         : callable_(new callable_impl<Fn>(std::forward<Fn>(fn))) {}

        unique_function(unique_function&&) = default;
        unique_function& operator=(unique_function&&) = default;

        unique_function(const unique_function&) = delete;
        unique_function& operator=(const unique_function&) = delete;

        Result operator() (Args&&... args) const {
            return callable_->invoke(std::forward<Args>(args)...);
        }

    private:
        struct callable
        {
            virtual Result invoke(Args&&... args) = 0;
            virtual ~callable() = default;
        };

        template<class Fn>
        struct callable_impl : callable
        {
            callable_impl(Fn&& fn) : fn_(std::forward<Fn>(fn)) {}

            Result invoke(Args&&... args) override {
                return fn_(std::forward<Args>(args)...);
            }

            typename std::decay<Fn>::type fn_;
        };

        std::unique_ptr<callable> callable_;
    };
}

#endif // ACTIVE_UNIQUE_FUNCTION_HPP