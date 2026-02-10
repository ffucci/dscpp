#pragma once

#include <memory>

namespace cpplearn::utility {

/// Close implementation to std::exception_guard
/// like in libc++ llvm
template <typename Rollback>
struct exception_guard_with_exception
{
    exception_guard_with_exception() = delete;

    constexpr exception_guard_with_exception(Rollback rollback) : rollback_(std::move(rollback)), completed_(false)
    {
    }

    exception_guard_with_exception(const exception_guard_with_exception&) = delete;
    exception_guard_with_exception& operator=(const exception_guard_with_exception&) = delete;
    exception_guard_with_exception& operator=(exception_guard_with_exception&&) noexcept = delete;

    exception_guard_with_exception(exception_guard_with_exception&& other) noexcept : rollback_(std::move(other.rollback_)), completed_(other.completed_)
    {
        // That needs to be set to true to avoid that the other rolls back and we roll back twice
        other.completed_ = true;
    }

    constexpr void complete() noexcept
    {
        completed_ = true;
    }

    constexpr ~exception_guard_with_exception()
    {
        if (!completed_) {
            rollback_();
        }
    }

   private:
    Rollback rollback_;
    bool completed_;
};

template <class Rollback>
using exception_guard = exception_guard_with_exception<Rollback>;

template <class Rollback>
constexpr exception_guard<Rollback> make_exception_guard(Rollback rollback)
{
    return exception_guard<Rollback>(std::move(rollback));
}
}  // namespace cpplearn::utility
