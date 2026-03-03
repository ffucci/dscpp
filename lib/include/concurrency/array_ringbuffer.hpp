#include <array>
#include <atomic>
#include <cstddef>

namespace cpplearn::concurrency {

template <typename T, std::size_t N>
class ArrayRingBuffer
{
   public:
    static_assert(N > 0, "Array buffer needs to have at least one slot");
    static_assert((N & (N - 1)) == 0, "Array buffer needs to be a power of 2");

    bool push(const T& value) noexcept
    {
        auto index = write_index_.load(std::memory_order_relaxed);
        auto next_index = (index + 1) & (N - 1);
        if (next_index == read_index_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[index] = value;
        write_index_.store(next_index, std::memory_order_release);
        return true;
    }

    bool pop(T& value) noexcept
    {
        auto tail = read_index_.load(std::memory_order_relaxed);
        if (tail == write_index_.load(std::memory_order_acquire)) {
            return false;
        }

        value = buffer_[tail];
        read_index_.store((tail + 1) & (N - 1), std::memory_order_release);
        return true;
    }

    [[nodiscard]] size_t read_idx() const noexcept
    {
        return read_index_.load(std::memory_order_relaxed);
    }

    [[nodiscard]] size_t write_idx() const noexcept
    {
        return write_index_.load(std::memory_order_relaxed);
    }

   private:
    alignas(64) std::atomic<size_t> read_index_{0};
    alignas(64) T buffer_[N];
    alignas(64) std::atomic<size_t> write_index_{0};
};
}  // namespace cpplearn::concurrency