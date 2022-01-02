
#pragma once

#include <coroutine>
#include "generator.hpp"

#include <iostream>
#include <iterator>

/**
 * @brief yield the elements of a range in order
 * this should behave similarly to python's `yield from`
 * 
 * usage: `co_await yield_from(my_range);`
 * @tparam Range the type of the range to yield from
 */
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
