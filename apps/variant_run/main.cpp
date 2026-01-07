#include <cstddef>
#include <iostream>
#include <chrono>

#include "mvariant/mvariant.hpp"

struct A
{
    int x;
    int y;
};

struct B
{
    int z;
};

int main()
{
    cpplearn::types::mvariant<int, float, double> v;
    v = 10;
    std::cout << v.get<int>() << std::endl;

    v = 11.5f;
    std::cout << v.get<float>() << std::endl;

    v = 22.67;
    std::cout << v.get<double>() << std::endl;

    cpplearn::types::mvariant<A, B> ex2;

    ex2 = A{1, 2};
    auto& a = ex2.get<A>();
    std::cout << a.x << ", " << a.y << std::endl;

    ex2 = B{5};
    auto& b = ex2.get<B>();
    std::cout << b.z << std::endl;
    return 0;
}