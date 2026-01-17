#include "simd/btree.hpp"
#include "simd/simd.hpp"
#include <iostream>

int main()
{
    alignas(64) std::array arr = {0, 1, 2, 3, 9, 9, 21, 9};
    // int arr[] = {8, 7, 6, 5, 4, 3, 2, 1};
    // experiments::types::AVXIReg x = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    // experiments::btree::print(x);
    // auto res = experiments::btree::compare(x, arr);
    // int ex[] = {9, -2, -2, -2, -2, -2, -2, -2};
    // auto v = _mm256_load_si256((experiments::types::AVXIReg*)ex);
    // experiments::btree::print(v);
    // int mask = _mm256_movemask_ps((__m256)v);

    // std::cout << std::hex << mask << std::endl;
    // experiments::btree::print(res);

    // Find example
    auto idx = experiments::simd::find_el(arr, 21);
    std::cout << idx << std::endl;
    return 0;
}