//
// Created by Monika on 17.10.2024.
//

template<typename T, typename Enable = void>
struct DefaultObjectMaker {
    static constexpr bool value = false;

    static void MakeDefault(T&) { }
};

template<> struct DefaultObjectMaker<float> {
    static constexpr bool value = true;
    static void MakeDefault(float& v) { v = 0.f; }
};

template<> struct DefaultObjectMaker<double> {
    static constexpr bool value = true;
    static void MakeDefault(double& v) { v = 0.0; }
};

template<> struct DefaultObjectMaker<int32_t> {
    static constexpr bool value = true;
    static void MakeDefault(int32_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<uint32_t> {
    static constexpr bool value = true;
    static void MakeDefault(uint32_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<int64_t> {
    static constexpr bool value = true;
    static void MakeDefault(int64_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<uint64_t> {
    static constexpr bool value = true;
    static void MakeDefault(uint64_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<bool> {
    static constexpr bool value = true;
    static void MakeDefault(bool& v) { v = false; }
};

template<> struct DefaultObjectMaker<char> {
    static constexpr bool value = true;
    static void MakeDefault(char& v) { v = '\0'; }
};

template<> struct DefaultObjectMaker<int16_t> {
    static constexpr bool value = true;
    static void MakeDefault(int16_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<uint16_t> {
    static constexpr bool value = true;
    static void MakeDefault(uint16_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<int8_t> {
    static constexpr bool value = true;
    static void MakeDefault(int8_t& v) { v = 0; }
};

template<> struct DefaultObjectMaker<uint8_t> {
    static constexpr bool value = true;
    static void MakeDefault(uint8_t& v) { v = 0; }
};


template<class T>
struct DefaultObjectMaker<std::optional<T>>
{
    static constexpr bool value = true;

    static void MakeDefault(std::optional<T>& v) {
        v.reset();
    }
};

template<>
struct DefaultObjectMaker<SR_UTILS_NS::String> {
    static constexpr bool value = true;

    static void MakeDefault(SR_UTILS_NS::String& v) {
        v.clear();
    }
};

template<>
struct DefaultObjectMaker<SR_UTILS_NS::StringAtom> {
    static constexpr bool value = true;

    static void MakeDefault(SR_UTILS_NS::StringAtom& v) {
        v = {};
    }
};

template<class T>
struct DefaultObjectMaker<SR_UTILS_NS::Vector<T>> {
    static constexpr bool value = true;

    static void MakeDefault(SR_UTILS_NS::Vector<T>& v) {
        v.clear();
    }
};

template<typename T, typename U, typename Compare>
struct DefaultObjectMaker<SR_UTILS_NS::Map<T, U, Compare>> {
    static constexpr bool value = true;

    static void MakeDefault(SR_UTILS_NS::Map<T, U, Compare>& v) {
        v.clear();
    }
};

template<typename T, typename U>
struct DefaultObjectMaker<SR_HTYPES_NS::FlatHashMap<T, U>> {
    static constexpr bool value = true;

    static void MakeDefault(SR_HTYPES_NS::FlatHashMap<T, U>& v) {
        v.clear();
    }
};

template<typename T>
struct DefaultObjectMaker<SR_HTYPES_NS::FlatHashSet<T>> {
    static constexpr bool value = true;

    static void MakeDefault(SR_HTYPES_NS::FlatHashSet<T>& v) {
        v.clear();
    }
};

template<typename T, typename Less>
struct DefaultObjectMaker<SR_UTILS_NS::Set<T, Less>> {
    static constexpr bool value = true;

    static void MakeDefault(SR_UTILS_NS::Set<T, Less>& v) {
        v.clear();
    }
};

template<class T>
struct DefaultObjectMaker<SR_HTYPES_NS::SharedPtr<T>> {
    static constexpr bool value = true;

    static void MakeDefault(SR_HTYPES_NS::SharedPtr<T>& v) {
        v = nullptr;
    }
};