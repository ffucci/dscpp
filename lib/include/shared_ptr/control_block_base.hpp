#pragma once

#include <cstddef>

namespace cpplearn::memory {
    class shared_count {
    public:
        explicit shared_count(long refs = 1) noexcept : shared_owners_(refs) {
        }

        virtual ~shared_count() = default;

        void add_shared() noexcept { increment_count(shared_owners_); }

        bool release_shared() noexcept {
            // Expected shape:
            // 1. decrement shared_owners_
            // 2. when it reaches zero, call on_zero_shared()
            // 3. report whether the strong count hit zero
            if (decrement_count(shared_owners_) == 0) {
                on_zero_shared();
                return true;
            }

            return false;
        }

        long use_count() const noexcept { return shared_owners_; }

    protected:
        static void increment_count(long &count) noexcept { ++count; }

        static long decrement_count(long &count) noexcept { return --count; }

    private:
        shared_count(const shared_count &) = delete;

        shared_count &operator=(const shared_count &) = delete;

        virtual void on_zero_shared() noexcept = 0;

        // This intentionally stays non-atomic for now so the ownership flow remains
        // easy to inspect. Later we can swap these helpers to atomics.
        long shared_owners_{1};
    };
} // namespace cpplearn::memory
