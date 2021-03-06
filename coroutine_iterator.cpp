#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <array>
#include "generator.hpp"
#include "yield_from.hpp"

using namespace base;

generator<int> void_iterator() {
    co_yield 1;
    co_yield 2;
    co_yield 3;
    co_return;
}

generator<int, int> non_void_iterator() {
    co_yield 1;
    co_yield 2;
    co_yield 3;
    co_return 12;
}

generator<int> yield_from_test() {
    std::array a{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    co_await yield_from{a};
    co_return;
}

generator<int, int> yield_from_test2() {
    int x = co_await yield_from(non_void_iterator());
    co_return x;
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

    std::cout << '\n';

    auto g2 = yield_from_test2();
    for (int i : g2) {
        std::cout << i << '\n';
    }
    std::cout << "return value: " << g2.return_value() << '\n';

    return 0;
}
