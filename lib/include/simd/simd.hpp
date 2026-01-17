
#include <algorithm>
#include <cstddef>
#include "types.hpp"
namespace experiments::simd {

template <std::size_t N>
[[nodiscard]] auto find_el(const std::array<int, N>& arr, int el) -> int
{
    types::AVXIReg x = _mm256_set1_epi32(el);

    for (size_t i = 0; i < N; i += 8) {
        types::AVXIReg y = _mm256_load_si256((types::AVXIReg*)(arr.data() + i));
        types::AVXIReg res = _mm256_cmpeq_epi32(x, y);

        // extracts the sign of each element of the vector
        int mask = _mm256_movemask_ps((__m256)res);
        if (mask != 0) {
            return i + __builtin_ctz(mask);
        }
    }
    return -1;
}

}  // namespace experiments::simd