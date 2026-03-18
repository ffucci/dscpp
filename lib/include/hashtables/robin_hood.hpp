#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>

template <typename K, typename V, typename H = std::hash<K>>
class RobinHoodHashTable {
  static_assert(std::is_integral_v<K>);
  static_assert(std::is_trivially_copyable_v<K>);
  static_assert(std::is_trivially_copyable_v<V>);

public:
  explicit RobinHoodHashTable(size_t capacity) {
    capacity_ = next_power_of_two(std::max<size_t>(capacity, 2));
    const size_t alloc_bytes =
        ((sizeof(Slot) * capacity_) + 63ULL) & ~size_t{63};
    slots_ = static_cast<Slot *>(std::aligned_alloc(64, alloc_bytes));
    if (slots_ == nullptr) {
      throw std::bad_alloc();
    }
    std::memset(slots_, 0xFF, alloc_bytes);
  }

  ~RobinHoodHashTable() { std::free(slots_); }

  void insert(K key, V value) {
    uint64_t index = compute_index(key);
    uint64_t distance = 0;

    while (true) {
      if (slots_[index].key == EMPTY) {
        slots_[index].key = key;
        slots_[index].value = value;
        max_probe_ = std::max(max_probe_, distance);
        ++size_;
        return;
      }

      if (slots_[index].key == key) {
        slots_[index].value = value;
        return;
      }

      const auto desired = compute_index(slots_[index].key);
      const auto current_distance =
          (index + capacity_ - desired) & (capacity_ - 1);

      if (current_distance < distance) {
        std::swap(key, slots_[index].key);
        std::swap(value, slots_[index].value);
        max_probe_ = std::max(max_probe_, distance);
        distance = current_distance;
      }

      ++distance;
      index = (index + 1) & (capacity_ - 1);
    }
  }

  V *find(K key) { return lookup(key); }
  const V *find(K key) const { return lookup(key); }

  bool erase(K key) {
    uint64_t index = compute_index(key);
    uint64_t distance = 0;

    while (true) {
      if (slots_[index].key == EMPTY) {
        return false;
      }

      if (slots_[index].key == key) {
        backward_shift_erase(index);
        --size_;
        return true;
      }

      if (distance > probe_distance(slots_[index].key, index)) {
        return false;
      }

      index = (index + 1) & (capacity_ - 1);
      ++distance;
    }
  }

private:
  static auto next_power_of_two(size_t value) -> size_t {
    size_t power = 1;
    while (power < value) {
      power <<= 1;
    }
    return power;
  }

  V *lookup(K key) {
    uint64_t index = compute_index(key);
    uint64_t distance = 0;
    while (true) {
      if (slots_[index].key == EMPTY) {
        return nullptr;
      }

      if (slots_[index].key == key) {
        return &slots_[index].value;
      }

      if (distance > probe_distance(slots_[index].key, index)) {
        return nullptr;
      }

      index = (index + 1) & (capacity_ - 1);
      ++distance;
    }
  }

  const V *lookup(K key) const {
    uint64_t index = compute_index(key);
    uint64_t distance = 0;
    while (true) {
      if (slots_[index].key == EMPTY) {
        return nullptr;
      }

      if (slots_[index].key == key) {
        return &slots_[index].value;
      }

      if (distance > probe_distance(slots_[index].key, index)) {
        return nullptr;
      }

      index = (index + 1) & (capacity_ - 1);
      ++distance;
    }
  }

  void backward_shift_erase(uint64_t erase_index) {
    uint64_t current = erase_index;
    uint64_t next = (current + 1) & (capacity_ - 1);

    while (slots_[next].key != EMPTY &&
           probe_distance(slots_[next].key, next) > 0) {
      slots_[current] = slots_[next];
      current = next;
      next = (next + 1) & (capacity_ - 1);
    }

    slots_[current].key = EMPTY;
    slots_[current].value = V{};
  }

  static constexpr K EMPTY = static_cast<K>(~K{0});

  constexpr auto compute_index(K key) const -> size_t {
    return hasher_(key) & (capacity_ - 1);
  }

  constexpr auto probe_distance(K key, uint64_t slot_idx) const -> uint64_t {
    return (slot_idx + capacity_ - compute_index(key)) & (capacity_ - 1);
  }

  struct Slot {
    K key;
    V value;
  };

  H hasher_{};
  Slot *slots_{nullptr};
  size_t size_{0};
  uint64_t capacity_{0};
  uint64_t max_probe_{0};
};
