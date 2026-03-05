#pragma once

#include <atomic>
#include <cstdint>
#include <concepts>

namespace cpplearn::concurrency {

template <typename T>
class DoubleBuffer
{
public:
    DoubleBuffer() noexcept
        : published_ptr_(&buffer_[0])
    {
    }

    void write(const T& value) noexcept
    {
        T* front_ptr = published_ptr_.load(std::memory_order_acquire);
        T* back_ptr = (front_ptr == &buffer_[0]) ? &buffer_[1] : &buffer_[0];
        *back_ptr = value;
        published_ptr_.store(back_ptr, std::memory_order_release);
    }

    template <std::invocable<T&> OnRead>
    void read(OnRead&& on_read)
    {
        T* published_ptr = published_ptr_.load(std::memory_order_acquire);
        on_read(*published_ptr);
    }

private:
    alignas(64) T buffer_[2];
    alignas(64) std::atomic<T*> published_ptr_;
};

template <typename T>
class DoubleBufferStrict
{
public:
    void write(const T& value) noexcept
    {
       while (true) {
           auto s = state_.load(std::memory_order_acquire);
           if (s & BACK_READY) {
               continue;
           }

           auto front = s & FRONT_MASK;
           auto back = front ^ 1;
           buffer_[back] = value;

           auto desired = static_cast<uint8_t>(s | BACK_READY);
           if (state_.compare_exchange_weak(s, desired, std::memory_order_acq_rel)) {
               return;
           }
       }
    }

    template <std::invocable<T&> OnRead>
    void read(OnRead&& on_read)
    {
        while (true) {
            auto s = state_.load(std::memory_order_acquire);
            uint8_t front = s & FRONT_MASK;

            if (s & BACK_READY) {
                uint8_t new_front = front ^ 1;
                uint8_t desired = new_front;
                if (!state_.compare_exchange_weak(s, desired, std::memory_order_acq_rel)) {
                    continue;
                }

                front = new_front;
            }

            on_read(buffer_[front]);
            return;
        }
    }

private:
    static constexpr uint8_t FRONT_MASK = 0x01;
    static constexpr uint8_t BACK_READY = 0x02;

    alignas(64) std::atomic<uint8_t> state_{0};
    alignas(64) T buffer_[2];
};

template <typename T>
class DoubleBufferInitial
{
public:
    void write(const T& value) noexcept
    {
        auto write_idx = read_index_.load(std::memory_order_acquire) ^ 1u;
        buffer_[write_idx] = value;
    }

    template <std::invocable<T&> OnRead>
    void read(OnRead&& on_read)
    {
        auto read_idx = read_index_.load(std::memory_order_relaxed) ^ 1u;
        on_read(buffer_[read_idx]);
        read_index_.store(read_idx, std::memory_order_release);
    }

private:
    alignas(64) std::atomic<size_t> read_index_{0};
    alignas(64) T buffer_[2];
};

}
