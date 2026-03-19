#pragma once

#include "shared_ptr/control_block_weak.hpp"

#include <memory>
#include <utility>

namespace cpplearn::memory {
    template<typename T, typename Deleter = std::default_delete<T> >
    class control_block_pointer final : public shared_weak_count {
    public:
        control_block_pointer(T *ptr, Deleter deleter = Deleter())
            : shared_weak_count(1), ptr_(ptr), deleter_(std::move(deleter)) {
        }

        T *get() const noexcept { return ptr_; }

    private:
        void on_zero_shared() noexcept override {
            // TODO: destroy the managed object through deleter_ and clear ptr_.
            if (ptr_) {
                deleter_(ptr_);
                ptr_ = nullptr;
            }
        }

        void on_zero_shared_weak() noexcept override {
            // destroy the control block itself.
            delete this;
        }

        T *ptr_{nullptr};
        [[no_unique_address]] Deleter deleter_;
    };
} // namespace cpplearn::memory
