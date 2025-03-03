#include "btree.hpp"
#include "simd.hpp"
#include <cstddef>
#include <iostream>
#include <chrono>

int main()
{
    constexpr int N = 1 << 12;

    std::vector<size_t> measurements;
    measurements.reserve(N);
    std::array<int, N> arr{};

    std::ranges::iota(arr, 0);

    for (int i = 0; i < 1e5; i++) {
        const auto el_to_find = rand() % N;
        std::cout << el_to_find << std::endl;
        auto steady_time_start = std::chrono::steady_clock::now();
        // [[maybe_unused]] auto idx = experiments::simd::find_el(arr, el_to_find);
        [[maybe_unused]] auto idx = std::find(arr.begin(), arr.end(), el_to_find);
        auto steady_time_end = std::chrono::steady_clock::now();
        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(steady_time_end - steady_time_start);
        std::cout << duration_ns.count() << std::endl;
        // std::cout << "Elapsed time (steady_clock): " << duration_ns.count() << " nanoseconds\n";
    }

    // Find example

    return 0;
}