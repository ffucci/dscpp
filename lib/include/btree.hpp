#pragma once

#include <x86intrin.h>

#include <iostream>
#include <concepts>
#include <limits>
#include <vector>

namespace experiments::btree {

using Reg = __m256i;

void print(__m256i v)
{
    auto t = (int*)&v;
    for (int i = 0; i < 8; i++) std::cout << t[i] << " ";
    std::cout << std::endl;
}

auto compare(Reg x, std::integral auto* node) -> Reg
{
    Reg y = _mm256_load_si256((Reg*)node);
    print(y);
    return _mm256_cmpgt_epi32(x, y);
}

template <std::integral I = int, std::size_t N = 1e5>
class Btree
{
   public:
    constexpr Btree()
    {
        for (auto& el : data) {
            el = std::numeric_limits<I>::max();
        }
    }

   private:
    alignas(64) I data[N];
};
}  // namespace experiments::btree