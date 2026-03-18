#include "benchmark_utils.hpp"
#include "hashtables/linear_probing.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

using Key = std::uint64_t;
using Value = std::uint64_t;
using Entry = std::pair<Key, Value>;

constexpr std::size_t kInitialCapacity = 1u << 17;
constexpr float kLoadFactorPercent = 80.0f;
constexpr std::size_t kBatchSize = 1u << 14;

std::vector<Key> make_keys(std::size_t count, Key seed) {
    std::vector<Key> keys;
    keys.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        keys.push_back(seed + static_cast<Key>(i * 2 + 1));
    }
    std::shuffle(keys.begin(), keys.end(), cpplearn::rng());
    return keys;
}

std::vector<Entry> make_entries(std::size_t count, Key seed) {
    auto keys = make_keys(count, seed);
    std::vector<Entry> entries;
    entries.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        entries.push_back({keys[i], keys[i] ^ 0x9e3779b97f4a7c15ULL});
    }
    return entries;
}

const std::vector<Entry>& fixture_entries() {
    static const auto entries = make_entries(kBatchSize, 1'000);
    return entries;
}

const std::vector<Entry>& reinsert_entries() {
    static const auto entries = make_entries(kBatchSize, 10'000'000);
    return entries;
}

template <typename Adapter>
void populate_table(typename Adapter::Table& table, const std::vector<Entry>& entries) {
    for (const auto& [key, value] : entries) {
        Adapter::insert(table, key, value);
    }
}

template <typename Adapter>
bool validate_roundtrip(const std::string& name) {
    auto table = Adapter::make_empty(kInitialCapacity, kLoadFactorPercent);
    const auto& entries = fixture_entries();

    populate_table<Adapter>(table, entries);

    for (const auto& [key, value] : entries) {
        std::uint64_t steps = 0;
        const auto found = Adapter::find(table, key, steps);
        cpplearn::do_not_optimize(steps);
        if (found != value) {
            return cpplearn::check_failed(name, "find returned an unexpected value");
        }
    }

    return true;
}

template <typename Adapter>
bool validate_erase_reinsert(const std::string& name) {
    auto table = Adapter::make_empty(kInitialCapacity, kLoadFactorPercent);
    const auto& entries = fixture_entries();
    const auto& reinserts = reinsert_entries();

    populate_table<Adapter>(table, entries);

    for (const auto& [key, _] : entries) {
        Adapter::erase(table, key);
    }

    for (const auto& [key, value] : reinserts) {
        Adapter::insert(table, key, value);
    }

    for (const auto& [key, value] : reinserts) {
        std::uint64_t steps = 0;
        const auto found = Adapter::find(table, key, steps);
        cpplearn::do_not_optimize(steps);
        if (found != value) {
            return cpplearn::check_failed(name, "reinserted key was not found");
        }
    }

    return true;
}

template <typename Adapter>
std::uint64_t benchmark_insert_batch(int iterations) {
    const auto& entries = fixture_entries();
    std::uint64_t total_cycles = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        auto table = Adapter::make_empty(kInitialCapacity, kLoadFactorPercent);
        cpplearn::clobber();
        const auto start = cpplearn::cycle_start();
        for (const auto& [key, value] : entries) {
            Adapter::insert(table, key, value);
        }
        const auto end = cpplearn::cycle_end();
        cpplearn::do_not_optimize(table);
        total_cycles += end - start;
    }

    return total_cycles;
}

template <typename Adapter>
std::uint64_t benchmark_insert_single(int iterations) {
    const auto& entries = fixture_entries();
    std::uint64_t total_cycles = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        const auto& [key, value] = entries[static_cast<std::size_t>(iter) % entries.size()];
        auto table = Adapter::make_empty(kInitialCapacity, kLoadFactorPercent);
        cpplearn::clobber();
        const auto start = cpplearn::cycle_start();
        Adapter::insert(table, key, value);
        const auto end = cpplearn::cycle_end();
        cpplearn::do_not_optimize(table);
        total_cycles += end - start;
    }

    return total_cycles;
}

template <typename Adapter>
std::uint64_t benchmark_find_hit_batch(int iterations) {
    const auto& entries = fixture_entries();
    auto table = Adapter::make_empty(kInitialCapacity, kLoadFactorPercent);
    populate_table<Adapter>(table, entries);
    std::uint64_t total_cycles = 0;
    Value checksum = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        cpplearn::clobber();
        const auto start = cpplearn::cycle_start();
        for (const auto& [key, _] : entries) {
            std::uint64_t steps = 0;
            checksum += Adapter::find(table, key, steps) + steps;
        }
        const auto end = cpplearn::cycle_end();
        total_cycles += end - start;
    }

    cpplearn::do_not_optimize(checksum);
    return total_cycles;
}

template <typename Adapter>
std::uint64_t benchmark_erase_reinsert_batch(int iterations) {
    const auto& entries = fixture_entries();
    const auto& reinserts = reinsert_entries();
    std::uint64_t total_cycles = 0;

    for (int iter = 0; iter < iterations; ++iter) {
        auto table = Adapter::make_empty(kInitialCapacity, kLoadFactorPercent);
        populate_table<Adapter>(table, entries);
        cpplearn::clobber();
        const auto start = cpplearn::cycle_start();
        for (const auto& [key, _] : entries) {
            Adapter::erase(table, key);
        }
        for (const auto& [key, value] : reinserts) {
            Adapter::insert(table, key, value);
        }
        const auto end = cpplearn::cycle_end();
        cpplearn::do_not_optimize(table);
        total_cycles += end - start;
    }

    return total_cycles;
}

template <typename Adapter>
struct HashtableBenchmarkRegistrar {
    explicit HashtableBenchmarkRegistrar(std::string prefix)
        : roundtrip_validation(
              prefix + "_roundtrip",
              [name = prefix + "_roundtrip"] { return validate_roundtrip<Adapter>(name); })
        , erase_reinsert_validation(
              prefix + "_erase_reinsert",
              [name = prefix + "_erase_reinsert"] { return validate_erase_reinsert<Adapter>(name); })
        , insert_batch_benchmark(
              prefix + "_insert_batch",
              kBatchSize,
              benchmark_insert_batch<Adapter>)
        , insert_single_benchmark(
              prefix + "_insert_single",
              1,
              benchmark_insert_single<Adapter>)
        , find_hit_benchmark(
              prefix + "_find_hit_batch",
              kBatchSize,
              benchmark_find_hit_batch<Adapter>)
        , erase_reinsert_benchmark(
              prefix + "_erase_reinsert_batch",
              kBatchSize * 2,
              benchmark_erase_reinsert_batch<Adapter>) {}

    cpplearn::RegisterValidation roundtrip_validation;
    cpplearn::RegisterValidation erase_reinsert_validation;
    cpplearn::RegisterBenchmark insert_batch_benchmark;
    cpplearn::RegisterBenchmark insert_single_benchmark;
    cpplearn::RegisterBenchmark find_hit_benchmark;
    cpplearn::RegisterBenchmark erase_reinsert_benchmark;
};

struct LinearProbingAdapter {
    using Table = cpplearn::hashtables::LinearHashTable<Key, Value>;

    static Table make_empty(std::size_t capacity, float load_factor_percent) {
        return Table(capacity, load_factor_percent);
    }

    static void insert(Table& table, Key key, Value value) {
        table.insert(key, value);
    }

    static Value find(Table& table, Key key, std::uint64_t& steps) {
        auto [probe_steps, value] = table.find(key);
        steps = probe_steps;
        return value;
    }

    static void erase(Table& table, Key key) {
        table.erase(key);
    }
};

// Add new hashtables by implementing an adapter with the same four static methods
// as LinearProbingAdapter and instantiating another registrar below.
HashtableBenchmarkRegistrar<LinearProbingAdapter> linear_probing_benchmarks("linear_probing");

}  // namespace

int main() {
    return cpplearn::run_benchmarks();
}
