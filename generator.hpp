
#pragma once

#include <deque>
#include <iterator>
#include <optional>

namespace base {
/**
 * @brief the return value for generator coroutines
 *
 * @tparam T the type of the generated values
 * @tparam R the generator's return type
 */
template <typename T, typename R = void>
struct generator;

template <typename T, typename R>
struct promise_type;

template <typename T, typename R>
struct _promise_base {
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    std::suspend_always yield_value(T v) {
        _values.push_back(v);
        return {};
    }
    void unhandled_exception(){};
    T value() { return _values.front(); }

    void increment() { _values.pop_front(); }

    bool empty() const { return _values.empty(); }

   private:
    std::deque<T> _values;
};

template <typename T, typename R>
struct promise_type : public _promise_base<T, R> {
    generator<T, R> get_return_object() {
        using pt = promise_type<T, R>;
        return {std::coroutine_handle<pt>::from_promise(*this)};
    }
    void return_value(T v) {
        ret_val = v;
    }

    std::optional<R> ret_val;
};

template <typename T>
struct promise_type<T, void> : public _promise_base<T, void> {
    generator<T, void> get_return_object() {
        using pt = promise_type<T, void>;
        return {std::coroutine_handle<pt>::from_promise(*this)};
    }
    void return_void() {}
};

template <typename T, typename R>
struct generator {
    struct iterator;

    using promise_type = base::promise_type<T, R>;

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
        using difference_type = size_t;

        [[nodiscard]] T operator*() { return handle->promise().value(); }
        bool operator==(iterator const &it) const {
            return it.is_sentinel && handle->done() &&
                   handle->promise().empty();
        }
        void operator++() {
            handle->promise().increment();
            if (!handle->done())
                (*handle)();
        }
        void operator++(int x) {
            for (int i = 0; i < x; i++) {
                (*this)++;
            }
        }
        std::coroutine_handle<promise_type> *handle;
        bool is_sentinel;
    };

    /// TODO make the handle a shared_ptr somehow, because this isn't working

    ~generator() { handle.destroy(); }
    generator(const generator<T, R> &other)
        : handle(std::coroutine_handle<promise_type>::from_promise(
              other.handle.promise())) {}
    generator(generator<T, R> &&other)
        : handle(std::coroutine_handle<promise_type>::from_promise(
              other.handle.promise())) {}
    generator(std::coroutine_handle<promise_type> h) : handle(h) {}
};
}  // namespace base
