
#pragma once

#include <coroutine>
#include "generator.hpp"

#include <iostream>
#include <iterator>
// #include <ranges>

// template <typename T, typename R>
// base::generator<int> yield_1(std::coroutine_handle<typename generator<T, R>::promise_type> out) {

// }

template <typename Range>
struct yield_from {
    using value_type = std::remove_reference_t<
        typename std::iterator_traits<typename Range::iterator>::value_type>;
    using generator = base::generator<value_type, void>;
    using _generator_promise = generator::promise_type;

    Range &rng;

    bool await_ready() { return rng.begin() == rng.end(); }

    bool await_suspend(std::coroutine_handle<_generator_promise> h) {
        for (auto i : rng) {
            h.promise().yield_value(i);
        }
        return true;
    }

    void await_resume() {}

    yield_from(yield_from<Range> &) = default;
    yield_from(Range &rng) : rng(rng) {}
    yield_from(Range &&rng) : rng(rng) {}
};

// template <typename T> yield_from(std::initializer_list<T>) ->
// yield_from<std::initializer_list<T>>;

// template<typename T>
// base::generator<T, void> yield_from(std::forward_iterator auto begin,
// std::forward_iterator auto end) {

// }
