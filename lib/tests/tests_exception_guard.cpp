#include "utils/exception_guard.hpp"

#include <gtest/gtest.h>

TEST(ExceptionGuard, Basic) {
    int x = 0;
    {
        cpplearn::utility::exception_guard g([&x]() { x = 1; });
        g.complete();
    }
    EXPECT_EQ(x, 0);
}

TEST(ExceptionGuard, ExceptionThrown)
{
    int x = 0;
    try{
        cpplearn::utility::exception_guard g([&x]() { x = 1; });
    }catch (...) {
    }
    std::cout << x << std::endl;
    EXPECT_EQ(x, 1);
}