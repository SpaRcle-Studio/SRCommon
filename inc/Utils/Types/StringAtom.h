//
// Created by Monika on 29.09.2021.
//

#ifndef SR_ENGINE_STRING_ATOM_H
#define SR_ENGINE_STRING_ATOM_H

#include <Utils/stdInclude.h>
#include <Utils/Common/Hashes.h>

namespace SR_UTILS_NS {
    class StringHashInfo;

    /// @scriptableClass
    class SR_COMMON_DLL_API StringAtom {
        static std::string DEFAULT;
        static StringHashInfo* DEFAULT_STRING_INFO;
    public:
        /// @constructor
        StringAtom();

        StringAtom(const StringAtom& other) = default;

        StringAtom(StringHashInfo* pInfo); /// NOLINT
        /// @constructor
        StringAtom(const char* str); /// NOLINT
        StringAtom(const std::string& str); /// NOLINT
        StringAtom(std::string_view str); /// NOLINT

    public:
        operator const std::string&() const noexcept; /// NOLINT
        operator std::string_view() const noexcept; /// NOLINT
        /// @operator
        bool operator==(const StringAtom& rhs) const noexcept;
        bool operator==(const std::string& rhs) const noexcept;
        bool operator==(const std::string_view& rhs) const noexcept;
        /// @operator
        bool operator==(const char* rhs) const noexcept;
        StringAtom& operator=(const std::string& str);
        /// @operator
        StringAtom& operator=(const char* str);

        /// @operator
        bool operator<(const StringAtom& other) const noexcept;
        /// @operator
        bool operator<(uint64_t hash) const noexcept;

        SR_NODISCARD operator uint64_t() const noexcept;

        /// @operator
        SR_NODISCARD char operator[](size_t index) const noexcept;

    public:
        /// @method
        SR_NODISCARD uint64_t Size() const;
        /// @method
        SR_NODISCARD uint64_t size() const;
        SR_NODISCARD bool Contains(const char* str) const;
        /// @method
        SR_NODISCARD bool Empty() const;
        /// @method
        SR_NODISCARD bool empty() const;
        SR_NODISCARD uint64_t GetHash() const;
        SR_NODISCARD std::string ToString() const;
        /// @method
        SR_NODISCARD const char* ToCStr() const;
        /// @method
        SR_NODISCARD const char* c_str() const;
        /// @method
        SR_NODISCARD const char* data() const;
        SR_NODISCARD const std::string& ToStringRef() const;
        SR_NODISCARD std::string_view ToStringView() const;

        void clear();
        void Clear();

    private:
        StringHashInfo* m_info = nullptr;

    };

    template<typename T> constexpr bool IsString() {
        if (!IsVolatile<T>()) {
            return IsString<volatile T>();
        }

        return
            std::is_same_v<T, volatile std::string> ||
            std::is_same_v<T, volatile std::string_view> ||
            std::is_same_v<T, volatile StringAtom>;
    }
}

namespace std {
    template<> struct hash<SR_UTILS_NS::StringAtom> {
        size_t operator()(SR_UTILS_NS::StringAtom const& object) const noexcept {
            return object.GetHash();
        }
    };

    template<> struct less<SR_UTILS_NS::StringAtom> {
        bool operator()(const SR_UTILS_NS::StringAtom& lhs, const SR_UTILS_NS::StringAtom& rhs) const noexcept {
            return lhs.GetHash() < rhs.GetHash();
        }
    };
}

template<> struct fmt::formatter<SR_UTILS_NS::StringAtom>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(SR_UTILS_NS::StringAtom const& str, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", str.ToStringView());
    }
};

#endif //SR_ENGINE_STRING_ATOM_H
