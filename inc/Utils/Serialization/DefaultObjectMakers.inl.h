//
// Created by Monika on 17.10.2024.
//

template<typename T, typename Enable = void>
    struct DefaultObjectMaker {
    static constexpr bool value = false;

    static void MakeDefault(T&) { }
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
struct DefaultObjectMaker<std::string> {
    static constexpr bool value = true;

    static void MakeDefault(std::string& v) {
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

template<class T, class... TOther>
struct DefaultObjectMaker<std::vector<T, TOther...>> {
    static constexpr bool value = true;

    static void MakeDefault(std::vector<T, TOther...>& v) {
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
