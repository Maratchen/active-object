#pragma once

#ifndef ACTIVE_UNIQUE_FUNCTION_HPP
#define ACTIVE_UNIQUE_FUNCTION_HPP

#include <functional>
#include <memory>
#include <type_traits>

namespace active
{
#if __cpp_lib_move_only_function >= 202110L

    template<class... Ts>
    using unique_function = std::move_only_function<Ts...>;

#else

    /**
     * general template form
     */
    template<class... Ts>
    class unique_function;

#   if SOO_UNIQUE_FUNCTION

    namespace detail
    {
        enum class handle_command
        {
            create_from_value,
            create_from_other,
            invoke,
            destroy
        };

        using handle_function = void (*)(handle_command, void*, void*);

        template<class Function, class... Args>
        using params_pack = std::conditional_t<
                std::is_void_v<std::invoke_result_t<Function, Args...>>,
                std::tuple<std::add_lvalue_reference_t<Args>...>,
                std::tuple<
                    std::add_lvalue_reference_t<std::invoke_result_t<Function, Args...>>,
                    std::add_lvalue_reference_t<Args>...>>;

        template<class Function>
        struct function_inplace_impl
        {
            function_inplace_impl(Function&& fn) : function_(std::move(fn)) {}
            function_inplace_impl(const function_inplace_impl&) = delete;
            function_inplace_impl(function_inplace_impl&&) = default;
            ~function_inplace_impl() = default;

            Function function_;
        };

        template<class Function>
        struct function_pointer_impl
        {
            function_pointer_impl(Function&& fn) : function_(std::make_unique<Function>(std::move(fn))) {}
            function_pointer_impl(const function_pointer_impl&) = delete;
            function_pointer_impl(function_pointer_impl&&) = default;
            ~function_pointer_impl() = default;

            std::unique_ptr<Function> function_;
        };

        template<class Function, class... Args>
        void invoke_function(Function& fn, params_pack<Function, Args...>& params) {
            if constexpr (std::is_void_v<std::invoke_result_t<Function, Args...>>) {
                std::apply([&](auto&... args) {
                    std::invoke(fn, std::forward<Args>(args)...);
                }, params);
            } else {
                std::apply([&](auto& result, auto&... args) {
                    result = std::invoke(fn, std::forward<Args>(args)...);
                }, params);
            }
        }

        template<class Function, class... Args>
        void invoke_function(const std::unique_ptr<Function>& fn, params_pack<Function, Args...>& params) {
            invoke_function(*fn, params);
        }

        template<std::size_t BufferSize, class Function, class... Args>
        handle_function get_function_handle() {
            using function_impl = std::conditional_t<
                sizeof(Function) <= BufferSize,
                function_inplace_impl<Function>,
                function_pointer_impl<Function>>;

            static_assert(sizeof(function_impl) <= BufferSize, "Internal buffer is too small");
            
            return [](handle_command command, void* storage, void* param) {
                switch (command) {
                    case handle_command::create_from_value: {
                        auto* value = static_cast<Function*>(param);
                        new(storage) function_impl(std::move(*value));
                        break;
                    }
                    case handle_command::create_from_other: {
                        auto* other = static_cast<function_impl*>(param);
                        new(storage) function_impl(std::move(*other));
                        break;
                    }
                    case handle_command::invoke: {
                        auto* self = static_cast<function_impl*>(storage);
                        auto* params = static_cast<params_pack<Function, Args...>*>(param);
                        invoke_function<Function, Args...>(self->function_, *params);
                        break;
                    }
                    case handle_command::destroy: {
                        auto* self = static_cast<function_impl*>(storage);
                        std::destroy_at(self);
                        break;
                    }
                }
            };
        }

        constexpr static handle_function noop_handle = [](handle_command, void*, void*) {};

    } // namespace detail

    /**
     * Represents noncopyable std::function
     */
    template<class Result, class... Args>
    class unique_function<Result(Args...)>
    {
    public:
        using result_type = Result;
        constexpr static auto buffer_size = 4 * sizeof(void*);

        unique_function() noexcept
            : handle_(detail::noop_handle)
        {
        }

        template<class Function, class = std::enable_if_t<
            std::is_invocable_r<Result, Function, Args...>::value &&
            std::is_nothrow_move_constructible<Function>::value>>
        unique_function(Function&& value)
            : handle_(detail::get_function_handle<sizeof(storage_), Function, Args...>())
        {
            std::invoke(handle_, detail::handle_command::create_from_value, &storage_, &value);
        }

        unique_function(unique_function&& other) noexcept
            : handle_(other.handle_)
        {
            std::invoke(handle_, detail::handle_command::create_from_other, &storage_, &other.storage_);
        }

        unique_function& operator=(unique_function&& other) noexcept {
            std::invoke(handle_, detail::handle_command::destroy, &storage_, nullptr);
            std::invoke(other.handle_, detail::handle_command::create_from_other, &storage_, &other.storage_);
            handle_ = other.handle_;
            return *this;
        }

        unique_function(const unique_function&) = delete;
        unique_function& operator=(const unique_function&) = delete;

        ~unique_function() {
            std::invoke(handle_, detail::handle_command::destroy, &storage_, nullptr);
        }

        explicit operator bool() const noexcept {
            return handle_ != detail::noop_handle;
        }

        result_type operator() (Args&&... args) {
            if constexpr (std::is_void<result_type>::value) {
                auto params = std::tie(args...);
                handle_(detail::handle_command::invoke, &storage_, &params);
            } else {
                auto result = result_type();
                auto params = std::tie(result, args...);
                handle_(detail::handle_command::invoke, &storage_, &params);
                return result;
            }
        }

        void swap(unique_function& other) noexcept {
            auto temp = unique_function(std::move(*this));
            *this = std::move(other);
            other = std::move(temp);
        }

    private:
        detail::handle_function handle_;
        std::aligned_storage_t<buffer_size> storage_;
    };

#   else // #if SOO_UNIQUE_FUNCTION

    /**
     * Represents noncopyable std::function
     */
    template<class Result, class... Args>
    class unique_function<Result(Args...)>
    {
    public:
        unique_function() = default;

        template<class Function, class = std::enable_if_t<
            std::is_nothrow_move_constructible<Function>::value>>
        unique_function(Function&& fn)
         : callable_(std::make_unique<callable_impl<Function>>(std::forward<Function>(fn))) {}

        unique_function(unique_function&&) = default;
        unique_function& operator=(unique_function&&) = default;

        unique_function(const unique_function&) = delete;
        unique_function& operator=(const unique_function&) = delete;

        explicit operator bool() const noexcept {
            return static_cast<bool>(callable_);
        }

        Result operator() (Args&&... args) const {
            return callable_->invoke(std::forward<Args>(args)...);
        }

        void swap(unique_function& other) noexcept {
            using std::swap;
            swap(callable_, other.callable_);
        }

    private:
        struct callable
        {
            virtual Result invoke(Args&&... args) = 0;
            virtual ~callable() = default;
        };

        template<class Function>
        struct callable_impl : callable
        {
            callable_impl(Function&& fn) : fn_(std::forward<Function>(fn)) {}

            Result invoke(Args&&... args) override {
                return std::invoke(fn_, std::forward<Args>(args)...);
            }

            typename std::decay<Function>::type fn_;
        };

        std::unique_ptr<callable> callable_;
    };

#   endif // #if SOO_UNIQUE_FUNCTION

#endif // #if __cpp_lib_move_only_function >= 202110L

}

#endif // ACTIVE_UNIQUE_FUNCTION_HPP