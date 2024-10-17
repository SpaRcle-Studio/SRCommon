//
// Created by Monika on 17.10.2024.
//

template<class T> struct DefaultChecker<T, typename std::enable_if_t<std::is_default_constructible_v<T> && CheckOperatorUsableV<CheckerEqualityComparable, T, T> && !SerializationTraits<T>::HasEmpty && !std::is_same_v<T, SR_UTILS_NS::StringAtom>>> {
    static bool IsDefault(const T& value) {
        return SR_EQUALS(value, T {});
    }
};

template<> struct DefaultChecker<SR_UTILS_NS::StringAtom> {
    static bool IsDefault(const SR_UTILS_NS::StringAtom& value) {
        return value.Empty();
    }
};

template<class T> struct DefaultChecker<std::optional<T>> {
    static bool IsDefault(const std::optional<T>& value) {
        return !value.is_initialized();
    }
};

/// Specialization for types with "empty" method (std::vector, std::string, std::array, etc...)
template<class T>
struct DefaultChecker<T, std::enable_if_t<SerializationTraits<T>::HasEmpty && !IsTypeFromStdArrayTemplateV<T>>> {
    static bool IsDefault(const T& value) {
        return value.empty();
    }
};

template<class T, size_t N>
struct DefaultChecker<std::array<T, N>> {
    using IsAlwaysFalse = std::bool_constant<(N > 0)>;

    static constexpr bool IsDefault(const std::array<T, N>& value) {
        return value.empty();
    }
};
