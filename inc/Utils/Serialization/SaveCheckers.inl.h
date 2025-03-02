//
// Created by Monika on 01.03.2025.
//

template<typename T, typename Enable = void>
struct SaveChecker {
    static bool CanBeSaved(const T& value) {
        return true;
    }
};


template<typename T>
struct SaveChecker<T, typename std::enable_if<SerializationTraits<T>::IsSerializable>::type> {
    static bool CanBeSaved(const T& value) {
        return !value.HasSerializationFlags(SR_UTILS_NS::SerializationFlags::DontSave);
    }
};

template<typename T>
struct SaveChecker<SR_HTYPES_NS::SharedPtr<T>, std::enable_if_t<SerializationTraits<T>::IsSerializable>> {
    static bool CanBeSaved(const SR_HTYPES_NS::SharedPtr<T>& value) {
        return !value || SaveChecker<T>().CanBeSaved(*value);
    }
};
