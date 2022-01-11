
#pragma once

#include <coroutine>
#include "generator.hpp"
#include "concepts.hpp"

#include <iostream>
#include <iterator>

namespace base {
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

    Range &rng;

    bool await_ready() { return rng.begin() == rng.end(); }

    template <concepts::yielding_promise<value_type> Promise>
    bool await_suspend(std::coroutine_handle<Promise> h) {
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

template <typename T>
concept not_void = !std::is_void_v<T>;

template <typename T, not_void R>
struct yield_from<generator<T, R>> {
    using value_type = std::remove_reference_t<T>;
    using generator = base::generator<T, R>;

    generator &gen;
    bool await_ready() { return gen.begin() == gen.end(); }

    template <concepts::yielding_promise<value_type> Promise>
    bool await_suspend(std::coroutine_handle<Promise> h) {
        for (auto i : gen) {
            h.promise().yield_value(i);
        }
        return true;
    }

    R await_resume() {
        return gen.return_value();
    }

    yield_from(generator &gen) : gen(gen) {}
    yield_from(generator &&gen) : gen(gen) {}
};

static_assert(concepts::awaitable<yield_from<std::array<int, 12>>, void, base::detail::_promise_type<int, void>>);
static_assert(concepts::awaitable<yield_from<generator<int, int>>,
                                  int,
                                  base::detail::_promise_type<int, void>>);
}
