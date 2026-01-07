#include <cstddef>
#include <iostream>
#include <chrono>

#include "mvariant/mvariant.hpp"

int main()
{
    cpplearn::types::mvariant<int, float, double> v;
    v = 10;
    std::cout << v.get<int>() << std::endl;

    v = 11.5f;
    std::cout << v.get<float>() << std::endl;

    v = 22.67;
    std::cout << v.get<double>() << std::endl;
    return 0;
}