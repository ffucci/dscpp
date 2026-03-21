#pragma once
#include <memory>

#include "control_block_weak.hpp"
#include "shared_ptr.hpp"
#include "shared_weak_count.hpp"

namespace cpplearn::memory {
    template<typename Ptr>
    class WeakPtr {
    public:
        WeakPtr() noexcept = default;

        WeakPtr(const WeakPtr &) noexcept;

        WeakPtr &operator=(const WeakPtr &) noexcept;

        WeakPtr &operator=(WeakPtr &&) noexcept;

        template<class YPtr>
        WeakPtr &operator=(const SharedPtr<YPtr> &) noexcept;

        void swap(WeakPtr &) noexcept;

        void reset() noexcept;

        long use_count() const noexcept { return cntrl_ ? cntrl_->use_count() : 0; }
        bool expired() const noexcept { return cntrl_ == nullptr || (cntrl_->use_count() == 0); }

        SharedPtr<Ptr> lock() const noexcept;

        template<class _Up>
        friend class _LIBCPP_TEMPLATE_VIS SharedPtr;

        template<class _Up>
        friend class _LIBCPP_TEMPLATE_VIS SharedPtr;

    private:
        Ptr *ptr_{nullptr};
        shared_weak_count *cntrl_{nullptr};
    };

    template<typename Ptr>
    WeakPtr<Ptr>::WeakPtr(const WeakPtr &other) noexcept : ptr_(other.ptr_), cntrl_(other.cntrl_) {
        if (cntrl_ != nullptr) {
            cntrl_->add_weak();
        }
    }

    template<typename Ptr>
    SharedPtr<Ptr> WeakPtr<Ptr>::lock() const noexcept {
        SharedPtr<Ptr> res;
        // Either assigns the control block
        res.ctrl_ = cntrl_ ? cntrl_->lock() : cntrl_;
        // Assign the pointer only if it exists the control block
        if (res.cntrl_ != nullptr) {
            res.ptr_ = ptr_;
        }
        return res;
    }

    template<typename Ptr>
    void WeakPtr<Ptr>::swap(WeakPtr &other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(cntrl_, other.cntrl_);
    }

    template<typename Ptr>
    void WeakPtr<Ptr>::reset() noexcept {
        WeakPtr().swap(*this);
    }

    template<typename Ptr>
    WeakPtr<Ptr> &WeakPtr<Ptr>::operator=(const WeakPtr &other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }

    template<typename Ptr>
    WeakPtr<Ptr> &WeakPtr<Ptr>::operator=(WeakPtr &&other) noexcept {
        WeakPtr(std::move(other)).swap(*this);
        return *this;
    }

    template<typename Ptr>
    template<typename YPtr>
    WeakPtr<Ptr> &WeakPtr<Ptr>::operator=(const SharedPtr<YPtr> &other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }
}
