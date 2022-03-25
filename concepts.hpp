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

template <class T, template <class> class U>
concept instance_of = is_instance<T, U>::value;

template <typename T>
concept await_suspend_result =
    std::is_void_v<T> || std::convertible_to<T, bool> ||
    instance_of<T, std::coroutine_handle>;
}  // namespace detail


// SFINAE test for returning_promise
template <typename T>
struct has_return_value {
    // SFINAE: If T is a class with a member named `return_value`, then
    // decltype(&T::return_value) is a valid type, so this specialization exists. (and returns true_type)
    template <typename U>
    static auto test(decltype(&U::return_value)) -> std::true_type;
    // otherwise, only this specialization exists and returns false_type.
    template <typename>
    static auto test(...) -> std::false_type;
    
    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

// SFINAE test for void_promise
// see `has_return_value` for the reason why this is a separate test.
template <typename T>
struct has_return_void {
    template <typename U>
    static auto test(decltype(&U::return_void)) -> std::true_type;
    template <typename>
    static auto test(...) -> std::false_type;
    
    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

/**
 * @brief Is the first type awaitable, returning the second type, from the given promise type?
 * 
 * @tparam A the Awaitable type
 * @tparam R the return type of the `co_await` expression
 * @tparam Promise the promise type of the awaiting coroutine
 */
template <typename A, typename R = void, typename Promise = std::noop_coroutine_promise>
concept awaitable = requires(A a, std::coroutine_handle<Promise> handle) {
    { a.await_ready() } -> std::convertible_to<bool>;
    { a.await_suspend(handle) } -> detail::await_suspend_result;
    { a.await_resume() } -> std::same_as<R>;
};

// sanity check: trivial awaitables should be awaitable
static_assert(awaitable<std::suspend_always>);
static_assert(awaitable<std::suspend_never>);

/**
 * @brief is the type a valid promise type?
 * 
 * @tparam P the Promise type
 */
template <typename P>
concept valid_promise = requires(P p) {
    { p.initial_suspend() } -> awaitable;
    { p.final_suspend() }
    noexcept->awaitable;
    p.unhandled_exception();
} && (has_return_value<P>::value != has_return_void<P>::value);

/**
 * @brief is the type a promise for the given return object?
 * 
 * @tparam P the Promise type
 * @tparam T the return object type
 */
template <typename P, typename T>
concept promise_type = requires(P p) {
    { p.get_return_object() } -> std::same_as<T>;
} && valid_promise<P>;

/**
 * @brief is the type a valid coroutine return type?
 * 
 * @tparam T the type to be tested
 */
template <typename T>
concept coroutine_return_object = requires {
    promise_type<typename T::promise_type, T>;
};

/**
 * @brief is the type a promise that supports `co_yield`?
 * 
 * @tparam P the Promise type
 * @tparam V the type of the yielded expression
 */
template <typename P, typename V>
concept yielding_promise = valid_promise<P> && requires(P t, V v) {
    { t.yield_value(v) } -> awaitable;
};

/**
 * @brief is the type a promise that supports `co_return v`?
 * 
 * @tparam P the Promise type
 * @tparam V the type of the returned expression
 */
template <typename P, typename V>
concept returning_promise = valid_promise<P> && requires(P p, V v) {
    p.return_value(v);
};

/**
 * @brief is the type a promise that supports `co_return (void) v`?
 * 
 * @tparam P the Promise type
 */
template <typename P>
concept void_promise = valid_promise<P> && requires(P p) {
    p.return_void();
};
}  // namespace concepts
