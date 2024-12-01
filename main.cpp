#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <ranges>
#include "fibonacci_heap.cpp"

template<class T>
auto time(auto&& fun) {
    const auto start = std::chrono::steady_clock::now();
    fun.template operator()<T>();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
}

int main() {
    std::vector<int> test(10'000'000);
    std::ranges::generate(test, [&] {
        static  std::random_device rd;
        static std::mt19937 gen(rd());
        static std::normal_distribution<> distrib(0, 10);
        return distrib(gen);
    });
    fibonacci_heap<int> heap{std::ranges::views::iota(0, 100)};
    heap.pop();
    heap.printRoots();
    for(auto&& i: heap) {
        std::cout << i << " ";
    }
    auto it = heap.insert(1000);
    heap.decrease_key(it, -1);
    std::cout << heap.top() << '\n';

    return 0;
}
