#include "btree.hpp"

#include <iostream>

int main()
{
    int arr[] = {0, 1, 2, 3, 9, 9, 9, 9};
    // int arr[] = {8, 7, 6, 5, 4, 3, 2, 1};
    experiments::btree::Reg x = _mm256_set_epi32(1, 2, 3, 4, 5, 6, 7, 8);
    experiments::btree::print(x);
    auto res = experiments::btree::compare(x, arr);

    experiments::btree::print(res);
    return 0;
}