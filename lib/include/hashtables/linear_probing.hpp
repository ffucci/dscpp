#pragma once
#include "utils/constants.hp"


#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>

namespace cpplearn::hashtables {

template <typename K, typename T, typename H = std::hash<K>>
class LinearHashTable
{
    static_assert(std::is_integral_v<K>, "K must be integral");
    static_assert(std::is_trivially_copyable_v<K>, "K must be trivially copyable");
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
public:
    constexpr explicit LinearHashTable(size_t capacity, float load_factor)
    {
        capacity_ = capacity > 0 ? capacity : 1;
        if (load_factor > 0.0f && load_factor < 1.0f) {
            load_factor_ = load_factor;
        } else if (load_factor > 1.0f && load_factor <= 100.0f) {
            load_factor_ = load_factor / 100.0f;
        }
        allocate_slots(capacity_);
    }

    constexpr ~LinearHashTable()
    {
        std::free(slots_);
    }

    constexpr void insert(K key, T value)
    {
        if (size_ >= (capacity_ * load_factor_)) {
            resize(2*capacity_);
        }

        auto index = start_index(key);
        while (slots_[index].key != EMPTY && slots_[index].key != DELETED && slots_[index].key != key) {
            index = (index + 1) % capacity_;
        }
        if (slots_[index].key == key) {
            slots_[index].value = value;
            return;
        }
        slots_[index].key = key;
        slots_[index].value = value;
        ++size_;
    }

    constexpr std::pair<uint64_t, uint64_t> find(K key)
    {
        return find_from(key, start_index(key));
    }

    constexpr auto find_from(K key, uint64_t index)
    {
        std::pair<uint64_t, uint64_t> result{0, 0};
        while (result.first < capacity_) {
            if (slots_[index].key == EMPTY) {
                break;
            }
            if (slots_[index].key == key) {
                result.second = slots_[index].value;
                return result;
            }
            index = (index + 1) % capacity_;
            ++result.first;
        }
        return result;
    }

    constexpr bool contains(K key) const
    {
        size_t steps = 0;
        auto index = start_index(key);
        while (steps < capacity_) {
            if (slots_[index].key == EMPTY) {
                return false;
            }
            if (slots_[index].key == key) {
                return true;
            }
            index = (index + 1) % capacity_;
            ++steps;
        }
        return false;
    }

    constexpr bool erase(K key)
    {
        auto index = start_index(key);
        size_t steps = 0;
        while (steps < capacity_) {
            if (slots_[index].key == EMPTY) {
                return false;
            }
            if (slots_[index].key == key) {
                slots_[index].key = DELETED;
                --size_;
                return true;
            }
            index = (index + 1) % capacity_;
            ++steps;
        }
        return false;
    }

    constexpr void clear()
    {
        size_ = 0;
        std::memset(slots_, 0xFF, sizeof(Slot) * capacity_);
    }

private:
    constexpr void resize(size_t capacity)
    {
        auto old_data = slots_;
        auto old_capacity = capacity_;
        capacity_ = capacity;
        allocate_slots(capacity_);
        size_ = 0;
        // need to insert everything back
        for (size_t index = 0; index < old_capacity; index++) {
            if (old_data[index].key != EMPTY && old_data[index].key != DELETED) {
                insert(old_data[index].key, old_data[index].value);
            }
        }
        std::free(old_data);
    }

    constexpr auto start_index(K key) const -> size_t
    {
        return hash_(key) % capacity_;
    }

    constexpr void allocate_slots(size_t capacity)
    {
        const size_t alloc_bytes =
            ((sizeof(Slot) * capacity) + (utils::CACHE_LINE_SIZE - 1)) &
            ~(utils::CACHE_LINE_SIZE - 1);
        slots_ = static_cast<Slot*>(std::aligned_alloc(utils::CACHE_LINE_SIZE, alloc_bytes));
        if (slots_ == nullptr) {
            throw std::bad_alloc();
        }
        std::memset(slots_, 0xFF, alloc_bytes);
    }

    static constexpr K EMPTY = static_cast<K>(~K{0});
    static constexpr K DELETED = static_cast<K>(~K{0} - 1);
    struct Slot
    {
        K key;
        T value;
    };

    H hash_;
    Slot* slots_;
    size_t capacity_{0};
    size_t size_{0};
    float load_factor_{1.0};
};

}
