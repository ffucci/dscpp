#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "concurrency/double_buffer.hpp"

namespace {

constexpr std::size_t kCacheLineBytes = 64;
constexpr std::size_t kPayloadCacheLines = 128;
constexpr std::size_t kPayloadBytes = kCacheLineBytes * kPayloadCacheLines;
constexpr std::size_t kU64Count = kPayloadBytes / sizeof(std::uint64_t);

struct alignas(64) Payload
{
    std::array<std::uint64_t, kU64Count> words{};
};

struct RunStats
{
    std::uint64_t writes{0};
    std::uint64_t reads{0};
    std::uint64_t errors{0};
    std::uint64_t lastSeq{0};
    std::uint64_t writerRateOpsPerSec{0};
};

bool pinThreadToCpu(int cpu)
{
    if (cpu < 0) {
        return true;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        std::cerr << "Failed to pin thread to CPU " << cpu << ": " << std::strerror(errno) << '\n';
        return false;
    }
    return true;
}

std::uint64_t computeChecksum(const Payload& payload)
{
    std::uint64_t hash = 1469598103934665603ULL;
    for (std::size_t i = 2; i < payload.words.size(); ++i) {
        hash ^= payload.words[i] + 0x9e3779b97f4a7c15ULL;
        hash *= 1099511628211ULL;
    }
    return hash;
}

void fillPayload(Payload& payload, std::uint64_t seq)
{
    payload.words[0] = seq;
    payload.words[1] = 0;

    for (std::size_t i = 2; i < payload.words.size(); ++i) {
        payload.words[i] = (seq * 1315423911ULL) ^ (i * 11400714819323198485ULL);
    }

    payload.words[1] = computeChecksum(payload);
}

bool validatePayload(const Payload& payload)
{
    return payload.words[1] == computeChecksum(payload);
}

template <typename Buffer>
RunStats runScenario(const std::string& label, int writerCpu, int readerCpu, int durationSec)
{
    Buffer buffer;
    std::atomic<bool> stop{false};
    std::atomic<std::uint64_t> writes{0};
    std::atomic<std::uint64_t> reads{0};
    std::atomic<std::uint64_t> errors{0};
    std::atomic<std::uint64_t> lastSeq{0};

    std::thread writer([&] {
        if (!pinThreadToCpu(writerCpu)) {
            stop.store(true, std::memory_order_relaxed);
            return;
        }

        Payload payload{};
        std::uint64_t seq = 1;
        while (!stop.load(std::memory_order_relaxed)) {
            fillPayload(payload, seq);
            buffer.write(payload);
            writes.fetch_add(1, std::memory_order_relaxed);
            ++seq;
        }
    });

    std::thread reader([&] {
        if (!pinThreadToCpu(readerCpu)) {
            stop.store(true, std::memory_order_relaxed);
            return;
        }

        bool firstSample = true;
        auto next = std::chrono::steady_clock::now() + std::chrono::seconds(1);
        while (!stop.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_until(next);
            next += std::chrono::seconds(1);

            buffer.read([&](Payload& payload) {
                if (!firstSample && !validatePayload(payload)) {
                    errors.fetch_add(1, std::memory_order_relaxed);
                }
                firstSample = false;
                lastSeq.store(payload.words[0], std::memory_order_relaxed);
            });
            reads.fetch_add(1, std::memory_order_relaxed);
        }
    });

    const auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(durationSec));
    stop.store(true, std::memory_order_relaxed);

    writer.join();
    reader.join();

    const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               std::chrono::steady_clock::now() - start)
                               .count();

    RunStats stats;
    stats.writes = writes.load(std::memory_order_relaxed);
    stats.reads = reads.load(std::memory_order_relaxed);
    stats.errors = errors.load(std::memory_order_relaxed);
    stats.lastSeq = lastSeq.load(std::memory_order_relaxed);
    if (elapsedNs > 0) {
        stats.writerRateOpsPerSec = stats.writes * 1000000000ULL / static_cast<std::uint64_t>(elapsedNs);
    }

    std::cout << "mode=" << label << '\n';
    std::cout << "writes=" << stats.writes << " reads=" << stats.reads
              << " last_seq=" << stats.lastSeq << " validation_errors=" << stats.errors << '\n';
    std::cout << "writer_rate_ops_per_sec=" << stats.writerRateOpsPerSec << '\n';

    return stats;
}

} // namespace

int main(int argc, char* argv[])
{
    int writerCpu = 0;
    int readerCpu = 1;
    int durationSec = 10;
    std::string mode = "compare";

    if (argc >= 3) {
        writerCpu = std::stoi(argv[1]);
        readerCpu = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        durationSec = std::stoi(argv[3]);
    }
    if (argc >= 5) {
        mode = argv[4];
    }

    if (writerCpu == readerCpu && writerCpu >= 0) {
        std::cerr << "Writer and reader CPUs must be different\n";
        return 1;
    }

    const auto hw = std::thread::hardware_concurrency();
    if (hw > 0) {
        if ((writerCpu >= 0 && writerCpu >= static_cast<int>(hw))
            || (readerCpu >= 0 && readerCpu >= static_cast<int>(hw))) {
            std::cerr << "CPU id out of range. hardware_concurrency=" << hw << '\n';
            return 1;
        }
    }

    std::cout << "double_buf_run\n";
    std::cout << "writer_cpu=" << writerCpu << " reader_cpu=" << readerCpu
              << " duration_sec=" << durationSec << " mode=" << mode << '\n';
    std::cout << "payload_bytes=" << kPayloadBytes << " (" << kPayloadCacheLines << " cache lines)\n";

    if (mode == "latest") {
        const auto stats = runScenario<cpplearn::concurrency::DoubleBuffer<Payload>>(
            "latest", writerCpu, readerCpu, durationSec);
        return stats.errors == 0 ? 0 : 2;
    }

    if (mode == "strict") {
        const auto stats = runScenario<cpplearn::concurrency::DoubleBufferStrict<Payload>>(
            "strict", writerCpu, readerCpu, durationSec);
        return stats.errors == 0 ? 0 : 2;
    }

    if (mode == "initial") {
        const auto stats = runScenario<cpplearn::concurrency::DoubleBufferInitial<Payload>>(
            "initial", writerCpu, readerCpu, durationSec);
        return stats.errors == 0 ? 0 : 2;
    }

    if (mode == "compare") {
        const auto latestStats = runScenario<cpplearn::concurrency::DoubleBuffer<Payload>>(
            "latest", writerCpu, readerCpu, durationSec);
        const auto strictStats = runScenario<cpplearn::concurrency::DoubleBufferStrict<Payload>>(
            "strict", writerCpu, readerCpu, durationSec);
        const auto initialStats = runScenario<cpplearn::concurrency::DoubleBufferInitial<Payload>>(
            "initial", writerCpu, readerCpu, durationSec);

        std::cout << "summary latest_errors=" << latestStats.errors
                  << " strict_errors=" << strictStats.errors
                  << " initial_errors=" << initialStats.errors
                  << " latest_rate=" << latestStats.writerRateOpsPerSec
                  << " strict_rate=" << strictStats.writerRateOpsPerSec
                  << " initial_rate=" << initialStats.writerRateOpsPerSec << '\n';
        return (latestStats.errors == 0 && strictStats.errors == 0 && initialStats.errors == 0) ? 0 : 2;
    }

    std::cerr << "Unknown mode. Use: latest | strict | initial | compare\n";
    return 1;
}
