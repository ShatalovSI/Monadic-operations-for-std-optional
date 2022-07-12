#include <optional>
#include <concepts>


namespace nonstd
{
  template <typename T, typename Result, typename ...Args>
  concept invoke_result_convertible_to = requires(T && func)
  {
    requires std::convertible_to<std::invoke_result_t<T, Args...>, Result>;
  };

  template <class T>
  class optional;

  namespace details
  {
    template <typename Invocable, typename T>
    concept and_then_invocable = requires
    {
      requires std::invocable<Invocable, T>;
    typename std::invoke_result_t<Invocable, T>::value_type;
      requires std::derived_from<optional<typename std::invoke_result_t<Invocable, T>::value_type>,
    std::invoke_result_t<Invocable, T>>;
    };
  }

  template <class T>
  class optional : public std::optional<T>
  {
  public:
    using std::optional<T>::optional;

    using Base = std::optional<T>;
    using Self = optional<T>;

    constexpr optional(const Self& obj) = default;
    constexpr optional(Self&& obj) = default;

    constexpr optional(const Base& obj) noexcept(noexcept(std::optional<T> { obj })) : std::optional<T>{ obj }
    {
    }

    constexpr optional(Base&& obj) noexcept : std::optional<T>{ std::move(obj) }
    {
    }

    constexpr optional& operator= (const Self& obj) { static_cast<Base&>(*this) = obj; }
    constexpr optional& operator= (Self&& obj) noexcept { static_cast<Base&>(*this) = std::move(obj); }

    template <std::invocable<T> Invocable>
    constexpr auto transform(Invocable&& func) const noexcept(noexcept(func(**this)))
      -> optional<std::invoke_result_t<Invocable, T>>
    {
      using ResultType = optional<std::invoke_result_t<Invocable, T>>;
      if (this->has_value())
        return ResultType{ std::forward<Invocable>(func)(**this) };
      else
        return std::nullopt;
    }

    template <details::and_then_invocable<T> Invocable>
    constexpr auto and_then(Invocable&& func) const noexcept(noexcept(func(**this)))
      -> optional<typename std::invoke_result_t<Invocable, T>::value_type>
    {
      using ResultType = optional<typename std::invoke_result_t<Invocable, T>::value_type>;
      if (this->has_value())
      {
        return ResultType{ std::forward<Invocable>(func)(**this) };
      }

      return ResultType{};
    }

    template <std::invocable Invocable>
    constexpr auto or_else(Invocable&& func) const noexcept(noexcept(func())) -> optional<T>
    {
      if (this->has_value())
      {
        return **this;
      }

      if constexpr (std::same_as<std::invoke_result_t<Invocable>, void>)
      {
        std::forward<Invocable>(func)();
        return std::nullopt;
      }
      else
      {
        using ResultType = optional<T>;
        return ResultType{ std::forward<Invocable>(func)() };
      }
    }

  };

}

int main()
{
  return 0;
}