#include <gtest/gtest.h>

#include "variants/variant_union.hpp"

TEST(VariantStorageUnion, CanAllocate) {
    cpplearn::types::variant_storage<int, float> v(std::integral_constant<size_t, 1>{}, 1.2);
    v.template get<1>() = 2.4;

    EXPECT_EQ(v.template get<1>(), 2.4f);
}


TEST(VariantUnion, CanAllocate) {
    cpplearn::types::variant_union<int, float> v(1.2f);
    EXPECT_EQ(v.get<1>(), 1.2f);

    cpplearn::types::variant_union<int, float, double> v2(3.5);
    EXPECT_EQ(v2.get<2>(), 3.5);
}
