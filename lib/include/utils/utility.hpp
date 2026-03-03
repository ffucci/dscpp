#pragma once

#include <type_traits>

namespace cpplearn::utility {

// Takes a universal reference and returns an rvalue reference
template <typename T>
constexpr std::remove_reference_t<T>&& move(T&& value) noexcept
{
    return static_cast<T&&>(value);
}

// Forward instead return the exact value category that
// it was passed.
//
// In the first case if we pass an lvalue reference
// T is an lvalue -> T& when static_cast<T&&&> -> static_cast<T&>
template <typename T>
constexpr T&& forward(std::remove_reference_t<T>& value) noexcept
{
    return static_cast<T&&>(value);
}

// In the first case if we pass an lvalue reference
// T is a rvalue or prvalue -> (T&& or T) when static_cast<T&&&&> -> static_cast<T&&> when T&&
template <typename T>
constexpr T&& forward(std::remove_reference_t<T>&& value) noexcept
{
    static_assert(!std::is_lvalue_reference_v<T>);
    // when doing so we remove the name because here value is an lvalue
    return static_cast<T&&>(value);
}
}  // namespace cpplearn::utility