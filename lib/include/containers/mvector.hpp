#pragma once
#include <memory>

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
        // At the moment this is not providing strong exception guarantee
        if (n > 0) {
            this->_allocate(n);
            this->construct_at_end(n);
        }
    }

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

    ~mvector()
    {
        // Move to a destroy vector class
        this->allocator_.deallocate(this->begin_, this->size());
    }

private:
    pointer begin_{nullptr}; // start of the dynamic vector
    pointer end_{nullptr};   // contains the current state of the vector with
                    // all the initialized elements
    [[no_unique_address]] pointer cap_{nullptr}; // points to the end of the allocated memory
    [[no_unique_address]] allocator_type allocator_; // allocator used to allocate memory

    constexpr void _allocate(size_type n)
    {
        if (n > max_size()) {
            throw std::length_error("max_size() exceeded");
        }

        // Try to allocate
        auto allocation = _allocator_traits::allocate_at_least(this->allocator_, n);
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
};

}