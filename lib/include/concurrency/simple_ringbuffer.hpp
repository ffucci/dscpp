#include <atomic>
#include <vector>

namespace cpplearn::concurrency {
template <typename T>
class SimpleRingBufferFS
{
   public:
    explicit SimpleRingBufferFS(const size_t capacity) : buffer_(capacity)
    {
    }

    template <typename U>
    bool push(U&& value) noexcept
    {
        auto write_idx = write_index_.load(std::memory_order_relaxed);
        auto next_idx = write_idx + 1;

        // Maybe just a mod
        if (next_idx == buffer_.size()) {
            next_idx = 0;
        }

        if (next_idx == read_index_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[write_idx] = std::forward<U>(value);
        write_index_.store(next_idx, std::memory_order_release);
        return true;
    }

    bool pop(T& val) noexcept
    {
        auto read_idx = read_index_.load(std::memory_order_relaxed);

        if (read_idx == write_index_.load(std::memory_order_acquire)) {
            return false;
        }

        val = buffer_[read_idx];

        auto next_idx = read_idx + 1;
        if (next_idx == buffer_.size()) {
            next_idx = 0;
        }
        read_index_.store(next_idx, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return buffer_.empty();
    }
    [[nodiscard]] auto read_idx() const noexcept
    {
        return read_index_.load(std::memory_order_relaxed);
    }
    [[nodiscard]] auto write_idx() const noexcept
    {
        return write_index_.load(std::memory_order_relaxed);
    }

   private:
    alignas(64) std::vector<T> buffer_;
#ifdef NO_FALSE_SHARING
    alignas(64) std::atomic<size_t> read_index_{0};
    alignas(64) std::atomic<size_t> write_index_{0};
#else
    std::atomic<size_t> read_index_{0};
    std::atomic<size_t> write_index_{0};
#endif
};
}  // namespace cpplearn::concurrency