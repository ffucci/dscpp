#pragma once

#include <memory>
#include <cassert>

#include "utils/allocation_utils.hpp"
#include "utils/exception_guard.hpp"
#include "utils/iter.hpp"

namespace cpplearn::containers {

template <typename T, typename Allocator = std::allocator<T>>
class mvector
{
public:
    // Member types
    using value_type = T;
    using allocator_type = Allocator;
    using _allocator_traits = std::allocator_traits<allocator_type>;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = _allocator_traits::pointer;
    using const_pointer = _allocator_traits::const_pointer;
    using size_type = _allocator_traits::size_type;
    using difference_type = _allocator_traits::difference_type;

    constexpr explicit mvector(size_type n)
    {
        auto guard = cpplearn::utility::make_exception_guard(__destroy_vector(*this));
        if (n > 0) {
            this->_allocate(n);
            this->construct_at_end(n);
        }
        guard.complete();
    }

    constexpr explicit mvector() = default;

    // // This is still wrong
    // constexpr mvector(const mvector& m) : allocator_(m.allocator_) {
    //     this->_allocate(m.size());
    //     this->construct_at_end(m.size());
    // }

    // mvector& operator=(const mvector&)
    // {
    //
    // }

    constexpr mvector(size_type n, const allocator_type& allocator) : allocator_(allocator) {
        this->_allocate(n);
    }

    template <class... Args>
    constexpr reference emplace_back(Args&&... args);

    template <class... Args>
    constexpr void emplace_back_assume_capacity(Args&&... args)
    {
        assert(size() < capacity() && "Size is greater than capacity");
        _allocator_traits::construct(this->allocator_, this->end_, std::forward<Args>(args)...);
        // in libstdc++ there is a RAII guard that makes sure that there is a commit transaction done
        ++this->end_;
    }

    constexpr void assign(size_type n, const_reference value);

    constexpr void reserve(size_type n);

    constexpr void clear()
    {
        // The new end will be the begin, cost is O(n)
        this->destruct_at_end(this->begin_);
    }

    [[nodiscard]] constexpr value_type* data() noexcept
    {
        return this->begin_;
    }

    [[nodiscard]] constexpr pointer end() noexcept
    {
        return this->end_;
    }

    [[nodiscard]] constexpr pointer end() const noexcept
    {
        return this->end_;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return static_cast<size_type>(this->end_ - this->begin_);
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept
    {
        return static_cast<size_type>(this->cap_ - this->begin_);
    }

    [[nodiscard]] value_type& operator[](size_type n) noexcept
    {
        return this->begin_[n];
    }

    [[nodiscard]] const value_type& operator[](size_type n) const noexcept
    {
        return this->begin_[n];
    }

    [[nodiscard]] constexpr reference at(size_type n)
    {
        if (n >= this->size()) {
            throw_out_of_range();
        }
        return this->begin_[n];
    }

    [[nodiscard]] constexpr const_reference at(size_type n) const
    {
        if (n >= this->size()) {
            throw_out_of_range();
        }
        return this->begin_[n];
    }

    // Front and back methods
    [[nodiscard]] constexpr reference front() { return *this->begin_; }
    [[nodiscard]] constexpr const_reference front() const { return *this->begin_; }
    [[nodiscard]] constexpr reference back() { return *(this->end_ - 1); }
    [[nodiscard]] constexpr const_reference back() const { return *(this->end_ - 1); }

    /// Destroys the vector and releases the memory
    ~mvector()
    {
       __destroy_vector(*this)();
    }

private:
    pointer begin_{nullptr}; // start of the dynamic vector
    pointer end_{nullptr};   // contains the current state of the vector with
                    // all the initialized elements
    [[no_unique_address]] pointer cap_{nullptr}; // points to the end of the allocated memory
    [[no_unique_address]] allocator_type allocator_; // allocator used to allocate memory

    class __destroy_vector
    {
    public:
        constexpr __destroy_vector(mvector& mvec) : mvec_(mvec) {}

        constexpr void operator()()
        {
            if (mvec_.begin_ != nullptr) {
                // clear the vector
                mvec_.clear();
                _allocator_traits::deallocate(mvec_.allocator_, mvec_.begin_, mvec_.capacity());
            }
        }

    private:
        mvector& mvec_;
    };


    constexpr void _allocate(size_type n)
    {
        if (n > max_size()) {
            throw_length_error();
        }

        // Try to allocate
        auto allocation = safe_allocate_at_least(this->allocator_, n);

        // At this stage begin_ and end_ are pointing at the start of the memory
        // and cap_ is pointing at the end of the allocated memory
        begin_ = allocation.ptr;
        end_ = allocation.ptr;
        cap_ = begin_ + allocation.count;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return std::min<size_type>(
            _allocator_traits::max_size(allocator_),
            std::numeric_limits<difference_type>::max()
        );
    }

    constexpr void construct_at_end(size_type n)
    {
        const_pointer new_end = this->end_ + n;
        for (; this->end_ != new_end; ++this->end_) {
            _allocator_traits::construct(this->allocator_, this->end_);
        }
    }

    constexpr void construct_at_end(size_type n, const_reference value)
    {
        const_pointer new_end = this->end_ + n;
        for (; this->end_ != new_end; ++this->end_) {
            _allocator_traits::construct(this->allocator_, this->end_, value);
        }
    }

    constexpr void destruct_at_end(pointer new_last)
    {
        auto soon_to_be_end = this->end_;
        // End contains an element after the last one
        while (soon_to_be_end != new_last) {
            _allocator_traits::destroy(this->allocator_, std::to_address(--soon_to_be_end));
        }
        this->end_ = new_last;
    }

    constexpr void throw_length_error()
    {
        throw std::length_error("max_size() exceeded");
    }

    constexpr void throw_out_of_range()
    {
        throw std::out_of_range("index out of range");
    }

    constexpr void _deallocate() noexcept;
    constexpr size_type _recommend(size_type _new_size) const;
};

template <typename T, typename Allocator>
template <class... Args>
constexpr mvector<T, Allocator>::reference mvector<T, Allocator>::emplace_back(Args&&... args)
{
    if (this->end_ < this->cap_) [[likely]] {
        // fast allocation no allocation is needed
        this->emplace_back_assume_capacity(std::forward<Args>(args)...);
    }else {
        // slow allocation: reallocation is needed
        throw std::runtime_error("not implemented");
    }

    return *(this->end_ - 1); // before end is the last element
}

template <typename T, typename Allocator>
constexpr void mvector<T, Allocator>::reserve(size_type n)
{
    if (n > capacity()) {
        if (n > max_size()) {
            throw_length_error();
        }

        // perform strong exception guaranteed reallocation
        // TODO: implement split buffer for now basic reallocation
        // this is not exception safe

        auto exception_guard = cpplearn::utility::make_exception_guard(__destroy_vector(*this));
        auto allocation = safe_allocate_at_least(this->allocator_, n);
        auto old_size = this->size();
        if (this->begin_ != nullptr) {
            // we need to copy the elements to the new memory
            std::copy(this->begin_, this->end_, allocation.ptr);

            this->clear();
            _allocator_traits::deallocate(this->allocator_, this->begin_, this->capacity());
        }

        this->begin_ = allocation.ptr;
        this->end_ = allocation.ptr + old_size;
        this->cap_ = this->begin_ + allocation.count;
        exception_guard.complete();
    }
}

template <class T, class Allocator>
constexpr mvector<T,Allocator>::size_type mvector<T,Allocator>::_recommend(size_type _new_size) const
{
    const auto _max_size = max_size();
    if (_new_size > _max_size) {
        this->throw_length_error();
    }

    const size_type _cap = capacity();
    // We can't recommend more than max size
    if (_cap >= _max_size/2 ) {
        return _max_size;
    }

    return std::max<size_type>(2* cap_, _new_size);
}

template <typename T, typename Allocator>
constexpr void mvector<T, Allocator>::_deallocate() noexcept
{
    if (begin_ != nullptr) {
        clear();
        _allocator_traits::deallocate(this->allocator_, this->begin_, this->capacity());
        this->begin_ = nullptr;
        this->end_ = nullptr;
        this->cap_ = nullptr;
    }
}

template <typename T, typename Allocator>
constexpr void mvector<T, Allocator>::assign(size_type n, const_reference value)
{
    if (n <= this->capacity()) {
        size_type s = this->size();
        // [ v v v v v v v v v ---|
        // the length should be the min(s, n)
        std::fill_n(this->begin_, std::min(s, n), value);
        if (n > s) {
            this->construct_at_end(n - s, value);
        }else {
            // here n <= s it means that we don't need to construct but we can
            // potentially deallocate
            this->destruct_at_end(this->begin_ + n);
        }
    } else {
        // rellocation is needed
        _deallocate();
        this->_allocate(n);
        this->construct_at_end(n, value);
    }
}

}