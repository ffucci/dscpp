#pragma once

#include <type_traits>
#include <vector>
#include <atomic>

namespace cpplearn::concurrency {

template <typename T>
class RigtorpRingBuffer
{
   public:
    static_assert(std::is_trivially_constructible_v<T>);

    RigtorpRingBuffer(size_t capacity, 0) : buffer_(capacity)
    {
    }

    [[nodiscard]] bool push(const T& value) noexcept
    {
        const auto write_idx = write_idx_.load(std::memory_order_relaxed);
        auto next_write_idx = write_idx + 1;
        if (next_write_idx == buffer_.size()) {
            next_write_idx = 0;
        }

        if (next_write_idx == read_idx_cached_) {
            read_idx_cached_ = read_idx_.load(std::memory_order_acquire);
            if (next_write_idx == read_idx_cached_) {
                return false;
            }
        }

        buffer_[write_idx] = value;
        write_idx_.store(next_write_idx, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool pop(T& value)
    {
        const auto read_idx = read_idx_.load(std::memory_order_acquire);
        if (read_idx == write_idx_cached_) {
            read_idx_cached_ = read_idx_.load(std::memory_order_acquire);
            if (read_idx_cached_ == write_idx_cached_) {
                return false;
            }
        }

        value = buffer_[read_idx];
        auto next_idx = read_idx + 1;
        if (next_idx == buffer_.size()) {
            next_idx = 0;
        }
        read_idx_.store(next_idx, std::memory_order_release);
        return true;
    }

   private:
    std::vector<T> buffer_;
    alignas(64) std::atomic<size_t> read_idx_{0};
    alignas(64) size_t write_idx_cached_{0};
    alignas(64) std::atomic<size_t> write_idx_{0};
    alignas(64) size_t read_idx_cached_{0};
};

}  // namespace cpplearn::concurrency
