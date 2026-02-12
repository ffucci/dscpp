#include <memory>

// Define the result struct if the library missed it
#if !defined(__cpp_lib_allocate_at_least)
namespace std {
template<typename Pointer, typename SizeType = std::size_t>
struct allocation_result {
    Pointer ptr;
    SizeType count;
};
}
#endif

template <typename Alloc>
auto safe_allocate_at_least(Alloc& a, std::size_t n) {
#if defined(__cpp_lib_allocate_at_least)
    return std::allocator_traits<Alloc>::allocate_at_least(a, n);
#else
    // Fallback for incomplete Ubuntu 24.04 headers
    return std::allocation_result<typename std::allocator_traits<Alloc>::pointer>{
        a.allocate(n),
        n
    };
#endif
}