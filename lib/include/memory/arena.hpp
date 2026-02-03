#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace cpplearn::memory {

template <typename B = char>
class Arena
{
   public:
    explicit Arena(int size)
    {
        if (size <= 0) {
            throw std::runtime_error("Size is negative");
        }

        orig_start_ = std::malloc(size);
        begin_ = static_cast<B*>(orig_start_);
        end_ = begin_ + size;
    }

    [[nodiscard]] void* allocate(ptrdiff_t size, ptrdiff_t alignment)
    {
        ptrdiff_t pad = -(uintptr_t)begin_ & (alignment - 1);
        if (end_ - begin_ - pad < size) {
            return nullptr;
        }

        void* ptr = begin_ + pad;  // aligned pointer
        begin_ += pad + size;
        return ptr;
    }

    ~Arena() {
        if (orig_start_) {
            std::free(orig_start_);
        }
    }

   private:
    void* orig_start_;
    B* begin_;
    B* end_;
};

template <typename T>
class ArenaAllocatorV1
{
   public:
    explicit ArenaAllocatorV1(int count) : arena_(count * sizeof(T))
    {
    }

    template <typename... Args>
    T* create(Args&&... args) noexcept
    {
        void* ptr = arena_.allocate(sizeof(T), alignof(T));
        if (!ptr) {
            return nullptr;
        }

        // Wrong one according to the standard
        T* conv_ptr = static_cast<T*>(ptr);
        new (conv_ptr) T(std::forward<Args>(args)...);
        return conv_ptr;
    }

   private:
    Arena<char> arena_;
};

}  // namespace cpplearn::memory_order
