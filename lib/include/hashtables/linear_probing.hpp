#pragma once
#include "utils/constants.hp"


#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <type_traits>

namespace cpplearn::hashtables {

template <typename K, typename T, typename H = std::hash<K>>
class LinearHashTable
{
    static_assert(std::is_pod<T>::value, "T must be POD");
public:
    constexpr explicit LinearHashTable(size_t capacity, float load_factor) : capacity_(capacity)
    {
        if (load_factor_ > 0.0) {
            load_factor_ = 100.0f/load_factor;
        }

        capacity_ = capacity;
        slots_ = static_cast<Slot*>(std::aligned_alloc(utils::CACHE_LINE_SIZE, sizeof(Slot) * capacity_));
        std::memset(slots_, 0xFF, sizeof(Slot) * capacity_);
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

        auto hash = hash_(key);
        auto index = hash % (capacity_ - 1);
        // either finds delete or empty
        while (slots_[index].key < DELETED) {
            index = (index + 1) % capacity_;
        }
        slots_[index].key = key;
        slots_[index].value = value;
        size_++;
    }

    constexpr std::pair<uint64_t, uint64_t> find(K key)
    {
        return find_from(key, hash_(key) % capacity_);
    }

    constexpr auto find_from(K key, uint64_t index)
    {
        std::pair<uint64_t, uint64_t> result;
        while (slots_[index].key != key) {
            // this can be infinite loop
            index = (index + 1) % capacity_;
            result.first++;
            // just as emergency exit
            if (result.first >= capacity_) {
                break;
            }
        }

        // found the result
        result.second = slots_[index].value;
        return result;
    }

    constexpr bool contains(K key) const
    {
        size_t steps = 0;
        const auto hash = hash_(key);
        const auto index = hash % (capacity_ - 1);
        while (slots_[index].key != key) {
            index = (index + 1) % capacity_;
            if (steps > capacity_) {
                return false;
            }
        }

        return true;
    }

    constexpr void erase(K key)
    {
        auto hash = hash_(key);
        auto index = hash % (capacity_ - 1);
        while (slots_[index].key != key) {
            index = (index + 1) % capacity_;
        }
        slots_[index].key = DELETED;
        size_--;
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
        slots_ = static_cast<Slot*>(std::aligned_alloc(utils::CACHE_LINE_SIZE, sizeof(Slot) * capacity_));
        std::memset(slots_, 0xFF, sizeof(Slot) * capacity_);
        // need to insert everything back
        for (size_t index = 0; index < old_capacity; index++) {
            if (old_data[index].key < DELETED) {
                insert(old_data[index].key, old_data[index].value);
            }
        }
        std::free(old_data);
    }

    static constexpr uint64_t EMPTY = UINT64_MAX;
    static constexpr uint64_t DELETED = UINT64_MAX - 1;
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