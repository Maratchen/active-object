#include <future>
#include <memory>

// The relevant constructor of function requires its argument to be CopyConstructible.
// It's thanks to Richard Hodges that I found the decision 
// https://stackoverflow.com/a/30879004

// general template form
template<class Callable>
class universal_call;

// partial specialisation to cover most cases
template<class R, class... Args>
class universal_call<R(Args...)>
{
public:
  template<class Callable>
  universal_call(Callable&& callable)
    : impl_(std::make_shared<delegate_impl<Callable>>(std::forward<Callable>(callable)))
  {
  }

  R operator()(Args&&... args) const
  {
    return impl_->invoke(std::forward<Args>(args)...);
  }

private:
  struct delegate
  {
    virtual R invoke(Args&&... args) = 0;
    virtual ~delegate() = default;
  };

  template<class Callable>
  struct delegate_impl : delegate
  {
    delegate_impl(Callable&& callable)
      : callable_(std::forward<Callable>(callable))
    {
    }

    R invoke(Args&&... args) override
    {
      return callable_(std::forward<Args>(args)...);
    }

    Callable callable_;
  };

  std::shared_ptr<delegate> impl_;
};

// pathalogical specialisation for std::packaged_task - 
// erases the return type from the signature
template<class R, class... Args>
class universal_call<std::packaged_task<R(Args...)>>
  : public universal_call<void(Args...)>
{
  using universal_call<void(Args...)>::universal_call;
};

// (possibly) helpful function
template<class Callable>
universal_call<Callable> make_universal_call(Callable&& callable)
{
  return universal_call<Callable>(std::forward<Callable>(callable));
}