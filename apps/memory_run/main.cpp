#include <iostream>

#include "memory/arena.hpp"
#include "containers/mvector.hpp"

#pragma pack(push, 1)
struct A {
    int x;
    int y;
    short z;
};
#pragma pack(pop)

struct B
{
    int a{1};
    int b{2};
    int c{3};
};

std::ostream& operator<<(std::ostream& os, const B& b) {
    os << b.a << ", " << b.b << ", " << b.c;
    return os;
}

int main() {
    cpplearn::memory::ArenaAllocatorV2<A> arena(5);

    A* a = arena.create(1, 2, 4);
    std::cout << a->x << ", " << a->y << ", " << a->z << std::endl;

    A* b = arena.create(3, 4, 5);
    std::cout << b->x << ", " << b->y << ", " << b->z << std::endl;

    cpplearn::containers::mvector<B> vec(5);
    std::println(std::cout, "Vector size is {}", vec.size());
    std::cout << vec[1] << std::endl;
    return 0;
}