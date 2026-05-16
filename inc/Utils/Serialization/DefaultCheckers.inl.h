//
// Created by Monika on 17.10.2024.
//

template<class T> struct DefaultChecker<T, typename std::enable_if_t<std::is_default_constructible_v<T> && CheckOperatorUsableV<CheckerEqualityComparable, T, T> && !SerializationTraits<T>::HasEmpty && !std::is_same_v<T, SR_UTILS_NS::StringAtom>>> {
    static bool IsDefault(const T& value, const T* defaultValue) {
        if constexpr (std::is_same_v<RemoveQualifiersT<T>, float> || std::is_same_v<RemoveQualifiersT<T>, float_t>) {
            return std::isnan(value) || std::isinf(value) || SR_EQUALS(value, defaultValue ? *defaultValue : 0.0f);
        }
        else if constexpr (std::is_same_v<RemoveQualifiersT<T>, double> || std::is_same_v<RemoveQualifiersT<T>, double_t>) {
           return std::isnan(value) || std::isinf(value) || SR_EQUALS(value, defaultValue ? *defaultValue : 0.0);
        }
        else {
            if (defaultValue) {
                return SR_EQUALS(value, *defaultValue);
            }
            return SR_EQUALS(value, T());
        }
    }
};

template<> struct DefaultChecker<SR_UTILS_NS::StringAtom> {
    static bool IsDefault(const SR_UTILS_NS::StringAtom& value, const SR_UTILS_NS::StringAtom* defaultValue) {
        if (defaultValue) {
            return value == *defaultValue;
        }
        return value.Empty();
    }
};

template<> struct DefaultChecker<SR_UTILS_NS::Path> {
    static bool IsDefault(const SR_UTILS_NS::Path& value, const SR_UTILS_NS::Path* defaultValue) {
        if (defaultValue) {
            return value == *defaultValue;
        }
        return value.empty();
    }
};

template<> struct DefaultChecker<std::string> {
    static bool IsDefault(const std::string& value, const std::string* defaultValue) {
        if (defaultValue) {
            return value == *defaultValue;
        }
        return value.empty();
    }
};

template<> struct DefaultChecker<SR_HTYPES_NS::UnicodeString> {
    static bool IsDefault(const SR_HTYPES_NS::UnicodeString& value, const SR_HTYPES_NS::UnicodeString* defaultValue) {
        if (defaultValue) {
            return value == *defaultValue;
        }
        return value.empty();
    }
};

template<class T> struct DefaultChecker<std::optional<T>> {
    static bool IsDefault(const std::optional<T>& value, const std::optional<T>* defaultValue) {
        if (defaultValue) {
            return value == *defaultValue;
        }
        return !value.has_value();
    }
};

/// Specialization for types with "empty" method (std::vector, std::string, std::array, etc...)
template<class T>
struct DefaultChecker<T, std::enable_if_t<SerializationTraits<T>::HasEmpty && !IsTypeFromStdArrayTemplateV<T>>> {
    static bool IsDefault(const T& value, const T* defaultValue) {
        if (defaultValue) {
            if (value.size() != defaultValue->size()) {
                return false;
            }

            /// is std vector and not bitset
            constexpr bool isStdVector = std::is_same_v<RemoveQualifiersT<T>, std::vector<typename T::value_type>>
                || std::is_same_v<RemoveQualifiersT<T>, SR_UTILS_NS::Vector<typename T::value_type>>;

            if constexpr (isStdVector && !std::is_same_v<typename T::value_type, bool>) {
                for (size_t i = 0; i < value.size(); ++i) {
                    if (!DefaultChecker<typename T::value_type>::IsDefault(value[i], &(*defaultValue)[i])) {
                        return false;
                    }
                }
                return true;
            }

            return false;
        }
        return value.empty();
    }
};

template<class T, size_t N>
struct DefaultChecker<std::array<T, N>> {
    using IsAlwaysFalse = std::bool_constant<(N > 0)>;

    static constexpr bool IsDefault(const std::array<T, N>& value, const T*) {
        return value.empty();
    }
};
