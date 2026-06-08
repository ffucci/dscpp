#pragma once

#include <cstddef>
#include <utility>

namespace cpplearn::types {
    template<typename Type, typename... Types>
    union variant_storage;

    template<typename Type, typename... Types>
    union variant_storage {
        template<typename Other>
        constexpr variant_storage(std::integral_constant<size_t, 0>,
                                  Other &&value) : value(std::forward<Other>(value)) {
        }

        template<std::size_t Index, typename Other> requires (Index > 0)
        constexpr variant_storage(std::integral_constant<size_t, Index>, Other &&value) : storage(
            std::integral_constant<size_t, Index - 1>{}, std::forward<Other>(value)) {
        }

        template<size_t Index>
        constexpr auto &get() noexcept {
            if constexpr (Index == 0) {
                return value;
            } else {
                return storage.template get<Index - 1>();
            }
        }

        template<size_t Index>
        void destroy() {
            if constexpr (Index == 0) {
                value.~Type();
                return;
            } else {
                storage.template destroy<Index - 1>();
            }
        }

    private:
        Type value;
        variant_storage<Types...> storage;
    };

    template<typename Type>
    union variant_storage<Type> {
        template<typename Other>
        constexpr variant_storage(std::integral_constant<size_t, 0>,
                                  Other &&value) : value(std::forward<Other>(value)) {
        }

        template<size_t Index>
        constexpr auto &get() noexcept {
            return value;
        }

        void destroy() noexcept {
            value.~Type();
        }

    private:
        Type value;
    };

    template<typename T, typename... Types>
    struct first_type_of {
        using type = T;
    };

    template<typename T>
    struct first_type_of<T, T> {
        using type = T;
    };

    template<typename T, typename First, typename... Rest>
    constexpr auto get_type_index() noexcept {
        if constexpr (std::is_same_v<T, First>) {
            return 0;
        } else {
            return 1 + get_type_index<T, Rest...>();
        }
    }


    template<typename... Types>
    class variant_union {
    public:
        using first_type = typename first_type_of<Types...>::type;

        constexpr variant_union() : index(0), storage(std::integral_constant<size_t, 0>{}, first_type{}) {
        }

        template<typename T>
        constexpr variant_union(T &&value) : index(get_type_index<T, Types...>()),
                                             storage(std::integral_constant<size_t, get_type_index<T, Types...>()>{},
                                                     std::forward<T>(value)) {
        }

        template<size_t Index>
        auto &get() noexcept {
            return storage.template get<Index>();
        }

    private:
        size_t index;
        variant_storage<Types...> storage;
    };
}
