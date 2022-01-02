#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <array>
#include "generator.hpp"
#include "yield_from.hpp"

using namespace base;

generator<int> void_iterator() {
    // std::cout << "beginning\n";
    co_yield 1;
    // std::cout << "after 1st yield\n";
    co_yield 2;
    // std::cout << "after 2nd yield\n";
    co_yield 3;
    // std::cout << "after 3rd yield\n";
    co_return;
}

generator<int, int> non_void_iterator() {
    // std::cout << "beginning\n";
    co_yield 1;
    // std::cout << "after 1st yield\n";
    co_yield 2;
    // std::cout << "after 2nd yield\n";
    co_yield 3;
    // std::cout << "after 3rd yield\n";
    co_return 12;
}

generator<int> yield_from_test() {
    std::array a{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    co_await yield_from{a};
    co_return;
}

int main() {
    for (int i : void_iterator()) {
        std::cout << i << '\n';
    }

    std::cout << '\n';

    auto g = non_void_iterator();
    for (int i : g) {
        std::cout << i << '\n';
    }
    std::cout << "return value: " << g.return_value() << '\n';

    std::cout << '\n';

    for (int i : yield_from_test()) {
        std::cout << i << '\n';
    }

    return 0;
}
