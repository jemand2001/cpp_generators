
#pragma once

#include <deque>
#include <iterator>
#include <optional>
#include "concepts.hpp"

namespace base {
/**
 * @brief the return value for generator coroutines
 *
 * @tparam T the type of the generated values
 * @tparam R the generator's return type
 */
template <typename T, typename R = void>
struct generator;

namespace detail {
template <typename T>
struct _promise_base {
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always yield_value(T v) {
        _values.push_back(v);
        return {};
    }
    void unhandled_exception(){};
    T value() const { return _values.front(); }

    void increment() { _values.pop_front(); }

    bool empty() const { return _values.empty(); }

   private:
    std::deque<T> _values;
};

template <typename T, typename R>
struct _promise_type : public _promise_base<T> {
    generator<T, R> get_return_object() {
        using pt = _promise_type<T, R>;
        return {std::coroutine_handle<pt>::from_promise(*this)};
    }
    void return_value(T v) {
        ret_val = v;
    }

    std::optional<R> ret_val;
};

template <typename T>
struct _promise_type<T, void> : public _promise_base<T> {
    generator<T, void> get_return_object() {
        using pt = _promise_type<T, void>;
        return {std::coroutine_handle<pt>::from_promise(*this)};
    }
    void return_void() {}
};

static_assert(concepts::returning_promise<_promise_type<int, int>, int>);
static_assert(concepts::void_promise<_promise_type<int, void>>);
}

/**
 * @brief A coroutine return object representing a range whose values are computed during iteration
 * 
 * example usage:
 * ```cpp
 * generator<int> void_gen() {
 *   co_yield 1;
 *   co_yield 2;
 * }
 * 
 * generator<int, int> int_gen() {
 *   co_yield 1;
 *   co_yield 2;
 *   co_return 12;
 * }
 * ```
 * @tparam T the type of the yielded values (given to `co_yield`)
 * @tparam R the return type (given to `co_return`)
 */
template <typename T, typename R>
struct generator {
    struct iterator;

    using promise_type = detail::_promise_type<T, R>;
    
    static_assert(concepts::yielding_promise<promise_type, T>);

    iterator begin() { return {&handle, false}; }
    iterator end() { return {nullptr, true}; }

    std::coroutine_handle<promise_type> handle;

    R return_value() requires(!std::is_void_v<R>) {
        auto v = handle.promise().ret_val;
        if (!v)
            throw std::logic_error(
                "taking the return value of a generator that is not finished "
                "yet!");
        return std::move(v.value());
    }

    struct iterator {
        using value_type = T;
        using iterator_category = std::forward_iterator_tag;
        using reference = T &;
        using difference_type = std::ptrdiff_t;

        [[nodiscard]] T operator*() const { return handle->promise().value(); }
        bool operator==(iterator const &it) const {
            return it.is_sentinel && handle->done() &&
                   handle->promise().empty();
        }
        iterator &operator++() {
            handle->promise().increment();
            if (!handle->done())
                (*handle)();
            return *this;
        }
        iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        std::coroutine_handle<promise_type> *handle;
        bool is_sentinel;
    };

    static_assert(std::input_iterator<iterator>);

    ~generator() { handle.destroy(); }
    generator(const generator<T, R>&) = delete;
    generator(std::coroutine_handle<promise_type> h) : handle(h) {}
};
}  // namespace base

static_assert(concepts::coroutine_return_object<base::generator<int>>);
static_assert(std::ranges::forward_range<base::generator<int>>);
static_assert(concepts::coroutine_return_object<base::generator<int, int>>);
static_assert(std::ranges::forward_range<base::generator<int, int>>);
