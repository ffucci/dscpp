#include "memory/arena.hpp"

#include <iostream>

struct A {
    int x;
    int y;
};

int main() {
    cpplearn::memory::ArenaAllocatorV1<A> arena(5);

    A* a = arena.create(1, 2);
    std::cout << a->x << ", " << a->y << std::endl;

    A* b = arena.create(3, 4);
    std::cout << b->x << ", " << b->y << std::endl;
    return 0;
}