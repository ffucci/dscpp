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

}  // namespace cpplearn::utility
