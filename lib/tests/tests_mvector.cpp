#include "containers/mvector.hpp"

#include <gtest/gtest.h>

TEST(MVector, Basic) {
    cpplearn::containers::mvector<int> v(10);
    std::cout << v.size() << std::endl;
    EXPECT_EQ(v.size(), 10);
}

TEST(MVector, EmplaceBack) {
    cpplearn::containers::mvector<int> v;
    v.reserve(10);
    v.emplace_back(5);
    std::cout << v[0] << std::endl;
    EXPECT_EQ(v.size(), 1);

    v.emplace_back(10);
    std::cout << v[1] << std::endl;
    EXPECT_EQ(v.size(), 2);
}

TEST(MVector, Reserve) {
    cpplearn::containers::mvector<int> v;
    v.reserve(10);
    EXPECT_EQ(v.capacity(), 10);
    v.emplace_back(10);

    v.reserve(20);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v.size(), 1);
    v.emplace_back(20);

    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v.capacity(), 20);
}

