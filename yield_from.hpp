
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

    Range &rng;

    bool await_ready() { return rng.begin() == rng.end(); }

    void await_suspend(auto h) {
        std::cout << "suspended\n";
        for (auto i : rng) {
            std::cout << "value coming through\n";
            h.promise().yield_value(i);
            std::cout << "value yielded\n";
        }
    }

    void await_resume() {}

    yield_from(yield_from<Range> &) = default;
    yield_from(Range &rng) : rng(rng) {}
    yield_from(Range &&rng) : rng(rng) {}
};

// template <typename T> yield_from(std::initializer_list<T>) -> yield_from<std::initializer_list<T>>;

// template<typename T>
// base::generator<T, void> yield_from(std::forward_iterator auto begin, std::forward_iterator auto end) {
    
// }
