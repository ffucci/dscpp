#include <thread>
#include <iostream>

#define NO_FALSE_SHARING true

#include "concurrency/array_ringbuffer.hpp"
#include "concurrency/simple_ringbuffer.hpp"

void pinThread(int cpu)
{
    if (cpu < 0) {
        return;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == -1) {
        perror("pthread_setaffinity_no");
        exit(1);
    }
}

template <typename T>
void bench(int cpu1, int cpu2)
{
    constexpr size_t queueSize = 100000;
    constexpr int64_t iters = 100000000;

    T q(queueSize);
    auto t = std::thread([&] {
        pinThread(cpu1);
        for (int i = 0; i < iters; ++i) {
            int val;
            while (!q.pop(val));
            if (val != i) {
                throw std::runtime_error("Unexpected value");
            }
        }
    });

    pinThread(cpu2);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i) {
        while (!q.push(i));
    }
    while (q.read_idx() != q.write_idx());
    auto stop = std::chrono::steady_clock::now();
    t.join();
    std::cout << iters * 1000000000 / std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count()
              << " ops/s" << std::endl;
}

int main(int argc, char *argv[])
{
    int cpu1 = -1;
    int cpu2 = -1;

    if (argc == 3) {
        cpu1 = std::stoi(argv[1]);
        cpu2 = std::stoi(argv[2]);
    }

    // bench<ringbuffer>(cpu1, cpu2);
    // bench<cpplearn::concurrency::ArrayRingBuffer<int, 1 << 20>>(cpu1, cpu2);
    bench<cpplearn::concurrency::SimpleRingBufferFS<int>>(cpu1, cpu2);
    return 0;
}