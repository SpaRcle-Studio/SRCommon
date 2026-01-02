//
// Created by Monika on 01.03.2025.
//

#ifndef SR_ENGINE_UTILS_SERIALIZATION_SAVE_CHECKERS_INL_H
#define SR_ENGINE_UTILS_SERIALIZATION_SAVE_CHECKERS_INL_H

template<typename T, typename Enable = void>
struct SaveChecker {
    static bool CanBeSaved(const T& value) {
        return true;
    }
};

SR_COMMON_DLL_API extern bool SerializableCanBeSavedImpl(const Serializable& value);

template<typename T>
struct SaveChecker<T, typename std::enable_if<SerializationTraits<T>::IsSerializable>::type> {
    static bool CanBeSaved(const T& value) {
        return SerializableCanBeSavedImpl(static_cast<const Serializable&>(value));
    }
};

template<typename T>
struct SaveChecker<SR_HTYPES_NS::SharedPtr<T>, std::enable_if_t<SerializationTraits<T>::IsSerializable>> {
    static bool CanBeSaved(const SR_HTYPES_NS::SharedPtr<T>& value) {
        return !value || SaveChecker<T>().CanBeSaved(*value);
    }
};

#endif //SR_ENGINE_UTILS_SERIALIZATION_SAVE_CHECKERS_INL_H