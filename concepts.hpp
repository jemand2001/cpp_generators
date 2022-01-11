#pragma once

#include <concepts>
#include <coroutine>
#include <type_traits>

namespace concepts {
namespace detail {

template <class, template <class> class>
struct is_instance : public std::false_type {};

template <class T, template <class> class U>
struct is_instance<U<T>, U> : public std::true_type {};

template <typename T>
concept await_suspend_result =
    std::is_void_v<T> || std::convertible_to<T, bool> ||
    is_instance<T, std::coroutine_handle>::value;
}  // namespace detail

template <typename A, typename R = void, typename Promise = std::noop_coroutine_promise>
concept awaitable = requires(A a, std::coroutine_handle<Promise> handle) {
    { a.await_ready() } -> std::convertible_to<bool>;
    { a.await_suspend(handle) } -> detail::await_suspend_result;
    { a.await_resume() } -> std::same_as<R>;
};

// sanity check: trivial awaitables should be awaitable
static_assert(awaitable<std::suspend_always>);
static_assert(awaitable<std::suspend_never>);

template <typename P, typename T>
concept promise_type_c = requires(P p) {
    { p.get_return_object() } -> std::same_as<T>;
    { p.initial_suspend() } -> awaitable;
    { p.final_suspend() }
    noexcept->awaitable;
    { p.unhandled_exception() } -> awaitable;
};

template <typename T>
concept coroutine_return_object = requires {
    promise_type_c<typename T::promise_type, T>;
};

template <typename P, typename V>
concept yielding_promise = requires(P t, V v) {
    { t.yield_value(v) } -> awaitable;
};

template <typename P, typename V>
concept returning_promise = requires(P p, V v) {
    { p.return_value(v) } -> std::same_as<void>;
};

template <typename P>
concept void_promise = requires(P p) {
    { p.return_void() } -> std::same_as<void>;
};
}  // namespace concepts
