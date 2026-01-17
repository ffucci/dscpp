#pragma once

#include "types.hpp"
#include <iostream>
#include <concepts>
#include <limits>
#include <vector>

namespace experiments::btree {

void print(__m256i v)
{
    auto t = (int*)&v;
    for (int i = 0; i < 8; i++) std::cout << t[i] << " ";
    std::cout << std::endl;
}

auto compare(types::AVXIReg x, std::integral auto* node) -> types::AVXIReg
{
    types::AVXIReg y = _mm256_load_si256((types::AVXIReg*)node);
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