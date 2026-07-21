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

template<typename T, typename U, typename Compare, typename Allocator>
struct DefaultObjectMaker<std::map<T, U, Compare, Allocator>> {
    static constexpr bool value = true;

    static void MakeDefault(std::map<T, U, Compare, Allocator>& v) {
        v.clear();
    }
};

template<typename T, typename Less, typename Allocator>
struct DefaultObjectMaker<std::set<T, Less, Allocator>> {
    static constexpr bool value = true;

    static void MakeDefault(std::set<T, Less, Allocator>& v) {
        v.clear();
    }
};

template<typename T, typename U, typename Hash, typename KeyEqual, typename Allocator>
struct DefaultObjectMaker<std::unordered_map<T, U, Hash, KeyEqual, Allocator>> {
    static constexpr bool value = true;

    static void MakeDefault(std::unordered_map<T, U, Hash, KeyEqual, Allocator>& v) {
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