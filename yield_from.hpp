
#pragma once

#include <coroutine>
#include "generator.hpp"

#include <iostream>
#include <ranges>
#include <iterator>

template <typename Range>
struct yield_from {
    using generator =
        base::generator<std::remove_reference_t<typename std::iterator_traits<typename Range::iterator>::value_type>,
                        void>;
    using _generator_promise = generator::promise_type;

    using _iterator = Range::iterator;

    _iterator begin;
    _iterator end;

    std::coroutine_handle<_generator_promise> dst_handle;

    bool await_ready() { return begin != end; }

    void await_suspend(auto h) {
        dst_handle = h;
        for (auto it = begin; it != end; it++) {
            std::cout << "what\n";
            dst_handle.promise().yield_value(*it);
        }
    }

    void await_resume() {}

    yield_from(yield_from<Range> &) = default;
    yield_from(Range &rng) : begin(rng.begin()), end(rng.end()) {}
    yield_from(Range &&rng) : begin(rng.begin()), end(rng.end()) {}
};
