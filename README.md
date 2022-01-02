# cpp_generators
This is my implementation of generators using C++20 coroutines

For example uses, see [./coroutine_iterator.cpp]

## `generator<T, R>`
The return object type of a generator.
- `T` is the type of the values the generator will yield.
- `R` represents the return type of the generator (the type `co_return` will be called with). Defaults to `void`.

A `generator` object is a forward-iterable range, so if `f` returns one, the following loop head is valid:
```cpp
for (auto x : f())
```

To implement a generator, simply declare a function that returns one and use `co_yield` in its body, like this:
```cpp
generator<int> f() {
    co_yield 1;
    co_yield 2;
    co_return;
}
```
Note that the `co_return` is necessary because otherwise there would be no return statement at the end of a non-void function.

## `yield_from<Range>`
An awaitable that will insert the values of the given range into the awaiting coroutine.
Note that the return object must support yielding multiple values at once (for example, by placing them in a queue),
or else this will only yield the last value.
To use, just insert a call to `co_await yield_from(my_range);` into your coroutine.
