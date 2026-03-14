#include <thread>
#include <string_view>
#include <iostream>
#include <type_traits>

#define NO_FALSE_SHARING true

#include "concurrency/array_ringbuffer.hpp"
#include "concurrency/rigtorp_ringbuffer.hpp"
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
void bench(const char* name, int cpu1, int cpu2)
{
    constexpr size_t queueSize = 100000;
    constexpr int64_t iters = 100000000;

    T q = [] {
        if constexpr (std::is_constructible_v<T, size_t>) {
            return T(queueSize);
        } else {
            return T{};
        }
    }();
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
    std::cout << name << ": "
              << iters * 1000000000 / std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count()
              << " ops/s" << std::endl;
}

void printUsage(const char* program)
{
    std::cerr << "Usage: " << program << " [all|array|simple-fs|rigtorp] [consumer_cpu producer_cpu]" << std::endl;
}

int main(int argc, char *argv[])
{
    std::string_view selection = "all";
    int cpu1 = -1;
    int cpu2 = -1;

    if (argc == 2) {
        selection = argv[1];
    } else if (argc == 3) {
        cpu1 = std::stoi(argv[1]);
        cpu2 = std::stoi(argv[2]);
    } else if (argc == 4) {
        selection = argv[1];
        cpu1 = std::stoi(argv[2]);
        cpu2 = std::stoi(argv[3]);
    } else if (argc > 4) {
        printUsage(argv[0]);
        return 1;
    }

    bool ran = false;

    if (selection == "all" || selection == "array") {
        bench<cpplearn::concurrency::ArrayRingBuffer<int, 1 << 20>>("array", cpu1, cpu2);
        ran = true;
    }
    if (selection == "all" || selection == "simple-fs") {
        bench<cpplearn::concurrency::SimpleRingBufferFS<int>>("simple-fs", cpu1, cpu2);
        ran = true;
    }
    if (selection == "all" || selection == "rigtorp") {
        bench<cpplearn::concurrency::RigtorpRingBuffer<int>>("rigtorp", cpu1, cpu2);
        ran = true;
    }

    if (!ran) {
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
