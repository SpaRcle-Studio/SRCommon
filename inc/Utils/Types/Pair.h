//
// Created by Monika on 30.07.2026.
//

#if !defined(SR_ENGINE_COMMON_TYPES_PAIR_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_COMMON_TYPES_PAIR_H

#include <Utils/Common/AssertFwd.h>

namespace SR_UTILS_NS {
    template<typename T1, typename T2> struct Pair {
        using FirstType = T1;
        using SecondType = T2;
        using first_type = FirstType;
        using second_type = SecondType;

        FirstType first;
        SecondType second;

        Pair() = default;

        template<typename... Args1, typename... Args2>
        Pair(
            std::piecewise_construct_t,
            std::tuple<Args1...> firstArgs,
            std::tuple<Args2...> secondArgs
        )
            : first(std::make_from_tuple<FirstType>(std::move(firstArgs)))
            , second(std::make_from_tuple<SecondType>(std::move(secondArgs)))
        { }

        template<typename U1, typename U2>
        Pair(U1&& f, U2&& s)
            : first(std::forward<U1>(f))
            , second(std::forward<U2>(s))
        { }

        template<typename U1, typename U2>
        Pair(const Pair<U1, U2>& other)
                : first(other.first)
                , second(other.second)
        { }

        // Конвертирующий move-конструктор: Pair<U1,U2> → Pair<T1,T2>
        template<typename U1, typename U2>
        Pair(Pair<U1, U2>&& other)
            : first(std::forward<U1>(other.first))
            , second(std::forward<U2>(other.second))
        { }

        Pair(const Pair& other) : first(other.first), second(other.second) {}
        Pair(Pair&& other) noexcept : first(std::move(other.first)), second(std::move(other.second)) {}

        // operator= недоступен когда T1 — const (как у std::pair<const K, V>)
        Pair& operator=(const Pair& other) requires (!std::is_const_v<T1>) {
            if (this != &other) {
                first = other.first;
                second = other.second;
            }
            return *this;
        }
        Pair& operator=(Pair&& other) noexcept requires (!std::is_const_v<T1>) {
            if (this != &other) {
                first = std::move(other.first);
                second = std::move(other.second);
            }
            return *this;
        }

        template<typename U1, typename U2>
        Pair& operator=(const Pair<U1, U2>& other) requires (!std::is_const_v<T1>) {
            first  = other.first;
            second = other.second;
            return *this;
        }

        template<typename U1, typename U2>
        Pair& operator=(Pair<U1, U2>&& other) requires (!std::is_const_v<T1>) {
            first  = std::forward<U1>(other.first);
            second = std::forward<U2>(other.second);
            return *this;
        }

        // Конвертация в std::pair для совместимости со сторонними контейнерами (ska, std::map и т.д.)
        operator std::pair<T1, T2>() const& {
            return std::pair<T1, T2>(first, second);
        }
        operator std::pair<T1, T2>() && {
            return std::pair<T1, T2>(std::move(first), std::move(second));
        }

        // Конструктор из std::pair
        template<typename U1, typename U2>
        Pair(const std::pair<U1, U2>& p)
            : first(p.first)
            , second(p.second)
        { }

        template<typename U1, typename U2>
        Pair(std::pair<U1, U2>&& p)
            : first(std::forward<U1>(p.first))
            , second(std::forward<U2>(p.second))
        { }

    };

    template<typename T1, typename T2> Pair<T1, T2> MakePair(T1&& f, T2&& s) {
        return Pair<T1, T2>(std::forward<T1>(f), std::forward<T2>(s));
    }
}

#endif //SR_ENGINE_COMMON_TYPES_PAIR_H
