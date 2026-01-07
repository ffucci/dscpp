#pragma once

#include <typeinfo>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

// A very simple variant implementation for testing

namespace cpplearn::types {

namespace detail {

template <typename T>
T* get_as(void* ptr) {
  return std::launder(reinterpret_cast<T*>(ptr));
}

template <typename... Ts>
struct mvariant_helper;

template <typename F, typename... Ts>
struct mvariant_helper<F, Ts...>
{
  static constexpr void destroy(size_t id, void* data)
  {
      if(id == typeid(F).hash_code()) {
        get_as<F>(data)->~F();
      }
      else {
        mvariant_helper<Ts...>::destroy(id, data);
      }
  }

  static constexpr void copy(size_t id, const void* src, void* dst)
  {
      if(id == typeid(F).hash_code()) {
        new (dst) F(*get_as<F>(src));
      }
      else {
        mvariant_helper<Ts...>::copy(id, src, dst);
      }
  }

  static constexpr void move(size_t id, void* src, void* dst)
  {
    if(id == typeid(F).hash_code()) {
      new (dst) F(std::move(*get_as<F>(src)));
    }
    else {
      mvariant_helper<Ts...>::move(id, src, dst);
    }
  }
};

template <>
struct mvariant_helper<>
{
    static constexpr void destroy(size_t id, void* data) {}
    static constexpr void move(size_t id, void* src, void* dst) {}
    static constexpr void copy(size_t id, const void* src, void* dst) {}
};

}

template <typename... Ts>
class mvariant
{
  public:

    mvariant() : type_id{invalid_type_id()} {}

    template <typename T>
    bool is() const noexcept
    {
        return type_id == typeid(T).hash_code();
    }

    template <typename T>
    mvariant<Ts...>& operator=(T other)
    {
        detail::mvariant_helper<Ts...>::destroy(type_id, &other);
        type_id = typeid(T).hash_code();
        new (&data) T(std::move(other));
        return *this;
    }

    template <typename T>
    T& get()
    {
        if(type_id == typeid(T).hash_code()) {
            return *detail::get_as<T>(&data);
        }else {
            throw std::out_of_range("Not a valid type");
        }
    }

    mvariant(const mvariant& other) : type_id{other.type_id}
    {
        detail::mvariant_helper<Ts...>::copy(other.type_id, &other.data, &data);
    }

    mvariant<Ts...>& operator=(const mvariant<Ts...>& other)
    {
        mvariant<Ts...> tmp{other};
        std::swap(tmp.type_id, type_id);
        std::swap(tmp.data, data);
        return *this;
    }

    mvariant(mvariant&& other) noexcept
    {
        std::exchange(type_id, other.type_id);
        std::exchange(data, other.data);
    }

    mvariant& operator=(mvariant&& other) noexcept
    {
        std::exchange(type_id, other.type_id);
        std::exchange(data, other.data);
    }

    ~mvariant()
    {
        detail::mvariant_helper<Ts...>::destroy(type_id, data);
    }

  private:
    static constexpr size_t LARGEST = std::max({sizeof(Ts)...});
    static constexpr size_t invalid_type_id() {
      return typeid(void).hash_code();
    }

    alignas(LARGEST) unsigned char data[LARGEST];
    size_t type_id;
};

}
