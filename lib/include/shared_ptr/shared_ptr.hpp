#pragma once

#include "shared_ptr/control_block_pointer.hpp"

#include <cstddef>
#include <memory>
#include <utility>

namespace cpplearn::memory {

template <typename T>
class WeakPtr;

    template<typename T>
    class SharedPtr {
    public:
        using element_type = T;

        constexpr SharedPtr() noexcept = default;

        explicit SharedPtr(T *ptr)
            : ptr_(ptr), ctrl_(ptr != nullptr ? new control_block_pointer<T>(ptr) : nullptr) {
        }

        template<typename Deleter>
        SharedPtr(T *ptr, Deleter deleter)
            : ptr_(ptr),
              ctrl_(ptr != nullptr
                        ? new control_block_pointer<T, Deleter>(ptr, std::move(deleter))
                        : nullptr) {
        }

        SharedPtr(const SharedPtr &other) noexcept : ptr_(other.ptr_), ctrl_(other.ctrl_) {
            if (ctrl_ != nullptr) {
                ctrl_->add_shared();
            }
        }

        SharedPtr(SharedPtr &&other) noexcept
            : ptr_(std::exchange(other.ptr_, nullptr)), ctrl_(std::exchange(other.ctrl_, nullptr)) {
        }

        SharedPtr &operator=(const SharedPtr &other) noexcept {
            // TODO: implement copy assignment.
            // Common shape:
            // 1. guard self-assignment
            // 2. copy-and-swap, or increment new control block before releasing old
            SharedPtr(other).swap(*this);
            return *this;
        }

        SharedPtr &operator=(SharedPtr &&other) noexcept {
            // TODO: implement move assignment.
            SharedPtr(std::move(other)).swap(*this);
            return *this;
        }

        ~SharedPtr() {
            if (ctrl_ != nullptr) {
                ctrl_->release_shared();
            }
        }

        void reset() noexcept {
            // TODO: release one strong reference if ctrl_ is non-null, then clear
            // ptr_ and ctrl_ on this object.
            SharedPtr().swap(*this);
        }

        void reset(T *ptr) {
            SharedPtr replacement(ptr);
            swap(replacement);
        }

        template<typename Deleter>
        void reset(T *ptr, Deleter deleter) {
            SharedPtr replacement(ptr, std::move(deleter));
            swap(replacement);
        }

        void swap(SharedPtr &other) noexcept {
            std::swap(ptr_, other.ptr_);
            std::swap(ctrl_, other.ctrl_);
        }

        T *get() const noexcept { return ptr_; }

        T &operator*() const noexcept { return *ptr_; }

        T *operator->() const noexcept { return ptr_; }

        long use_count() const noexcept { return ctrl_ != nullptr ? ctrl_->use_count() : 0; }

        explicit operator bool() const noexcept { return ptr_ != nullptr; }

        template <typename Ptr>
        friend class WeakPtr;

    private:
        T *ptr_{nullptr};
        shared_weak_count *ctrl_{nullptr};
    };
} // namespace cpplearn::memory
