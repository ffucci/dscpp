#pragma once

#include "shared_ptr/shared_count.hpp"

namespace cpplearn::memory {
    class shared_weak_count : public shared_count {
    public:
        explicit shared_weak_count(long refs = 1) noexcept
            : shared_count(refs), weak_owners_(refs) {
        }

        void add_weak() noexcept { increment_count(weak_owners_); }

        void release_shared() noexcept {
            // reaches zero, drop the implicit weak reference held by the control block.
            if (shared_count::release_shared()) {
                release_weak();
            }
        }

        void release_weak() noexcept {
            // TODO: implement the weak-release transition.
            // Expected shape:
            // 1. decrement weak_owners_
            // 2. when it reaches zero, destroy the control block
            if (decrement_count(weak_owners_) == 0) {
                on_zero_shared_weak();
            }
        }

        long weak_count() const noexcept { return weak_owners_; }

        shared_weak_count *lock() noexcept {
            if (use_count() == 0) {
                return nullptr;
            }

            // Now we have an additional shared_pointer reading the control block
            add_shared();
            return this;
        }

    protected:
        ~shared_weak_count() override = default;

    private:
        virtual void on_zero_shared_weak() noexcept = 0;

        long weak_owners_{1};
    };
} // namespace cpplearn::memory
