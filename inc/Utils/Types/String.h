//
// Created by Monika on 02.05.2026.
//

#if !defined(SR_ENGINE_COMMON_TYPES_STRING_H) && defined(SR_ENGINE_COMMON_PCH_FOR_BASE_CODE)
#define SR_ENGINE_COMMON_TYPES_STRING_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class StringView;
    class IAllocator;

    class SR_COMMON_DLL_API String {
    public:
        using value_type = char;
        static constexpr auto npos{static_cast<SizeType>(-1)};
        using const_iterator = const char*;
        using iterator = char*;

    public:
        String();

        String(std::nullptr_t) = delete;
        String(IAllocator* pAllocator);
        String(const char* str, SizeType size, IAllocator* pAllocator);
        String(StringView str, IAllocator* pAllocator);
        String(String str, IAllocator* pAllocator);
        String(const std::string& str, IAllocator* pAllocator);
        String(std::string_view str, IAllocator* pAllocator);

        String(const char* str); /// NOLINT
        String(const std::string& str); /// NOLINT
        String(std::string_view str); /// NOLINT
        String(const String& other); /// NOLINT
        String(String&& other) noexcept;
        explicit String(StringView str); /// NOLINT

        ~String();

        String& operator=(const char* str);
        String& operator=(const std::string& str);
        String& operator=(std::string_view str);
        String& operator=(std::string&& str) noexcept;
        String& operator=(const String& other);
        String& operator=(StringView str);
        String& operator=(String&& other) noexcept;

        String& operator+=(const String& rhs);
        String& operator+=(const StringView& rhs);
        String& operator+=(const char* rhs);
        String& operator+=(char rhs);
        String& operator+=(const std::string& rhs);
        String& operator+=(std::string_view rhs);

        SR_NODISCARD String operator+(const String& rhs) const;
        SR_NODISCARD String operator+(const char* rhs) const;
        SR_NODISCARD String operator+(const std::string& rhs) const;
        SR_NODISCARD String operator+(std::string_view rhs) const;

        SR_NODISCARD bool operator==(const String& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const String& rhs) const noexcept;
        SR_NODISCARD bool operator==(const StringView& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const StringView& rhs) const noexcept;
        SR_NODISCARD bool operator==(const char* rhs) const noexcept;
        SR_NODISCARD bool operator!=(const char* rhs) const noexcept;
        SR_NODISCARD bool operator==(const std::string& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const std::string& rhs) const noexcept;
        SR_NODISCARD bool operator==(std::string_view rhs) const noexcept;
        SR_NODISCARD bool operator!=(std::string_view rhs) const noexcept;
        SR_NODISCARD bool operator<(const String& other) const noexcept;
        SR_NODISCARD bool operator>(const String& other) const noexcept;

        SR_NODISCARD operator std::string() const; /// NOLINT
        SR_NODISCARD operator std::string_view() const; /// NOLINT
        SR_NODISCARD operator StringView() const; /// NOLINT

        SR_NODISCARD const char* c_str() const;
        SR_NODISCARD SizeType size() const;
        SR_NODISCARD SizeType length() const;
        SR_NODISCARD SizeType capacity() const;
        SR_NODISCARD bool empty() const;
        SR_NODISCARD const char* data() const;
        SR_NODISCARD char* data();
        SR_NODISCARD char& back();
        SR_NODISCARD const char& back() const;
        SR_NODISCARD std::string_view view() const;

        SR_NODISCARD bool contains(std::string_view str) const;
        SR_NODISCARD bool starts_with(std::string_view str) const;
        SR_NODISCARD bool ends_with(std::string_view str) const;

        SR_NODISCARD String substr(SizeType pos, SizeType count = npos) const;
        SR_NODISCARD String DetachAllocator() const;
        SR_NODISCARD bool HasAllocator() const;

        SR_NODISCARD SizeType find(const char* str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(std::string_view str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(const String& str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(char c, SizeType pos = 0) const;
        SR_NODISCARD SizeType rfind(char c, SizeType pos = npos) const;

        SR_NODISCARD const char* begin() const;
        SR_NODISCARD const char* end() const;
        SR_NODISCARD char* begin();
        SR_NODISCARD char* end();

        char operator[](size_t index) const;
        char& operator[](size_t index);

        String& append(const String& str);
        String& append(const StringView& str);
        String& append(const char* str);
        String& append(const std::string& str);
        String& append(std::string_view str);
        String& append(char c);
        String& append(const char* str, SizeType count);
        String& append(const_iterator first, const_iterator last);

        String& erase(SizeType pos, SizeType count = npos);
        String erase(SizeType pos, SizeType count = npos) const;

        template<class Elem, class Traits> friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, const String& str) {
            return os << str.c_str();
        }

        void push_back(char c);
        void reserve(SizeType newSize);
        void resize(SizeType newSize);
        void resize(SizeType newSize, char fillChar);
        void clear();

        void ToLowerInPlace();
        void ToUpperInPlace();
        void SubStrInPlace(SizeType pos, SizeType count = npos);

    private:
        void Deallocate();

    private:
        char* m_data = nullptr;
        SizeType m_size = 0;
        SizeType m_capacity = 0;
        IAllocator* m_allocator = nullptr;

    };

    class SR_COMMON_DLL_API StringView {
    public:
        static constexpr auto npos{static_cast<SizeType>(-1)};

    public:
        constexpr StringView() = default;

        constexpr StringView(const StringView& other) = default;
        constexpr StringView(StringView&& other) noexcept = default;

        template<size_t N> constexpr StringView(const char (&str)[N]) noexcept
            : m_data(str)
            , m_size(N - 1) // без '\0'
        { }

        constexpr StringView(const char* str)
            : m_data(str)
        {
            for (; m_data && m_data[m_size] != '\0'; ++m_size) { }
        }

        constexpr StringView(const char *str, SizeType size)
            : m_data(str)
            , m_size(size)
        { }

        constexpr StringView(const std::string& str)
            : m_data(str.data())
            , m_size(static_cast<SizeType>(str.size()))
        { }

        constexpr StringView(std::string_view str)
            : m_data(str.data())
            , m_size(static_cast<SizeType>(str.size()))
        { }

        explicit StringView(const String& str)
            : m_data(str.data())
            , m_size(str.size())
        { }

        void remove_prefix(SizeType n);
        void clear();

        constexpr StringView& operator=(const StringView& other) = default;
        constexpr StringView& operator=(StringView&& other) noexcept = default;

        SR_NODISCARD operator std::string_view() const; /// NOLINT
        SR_NODISCARD operator String() const; /// NOLINT

        SR_NODISCARD const char* c_str() const;
        SR_NODISCARD SizeType size() const;
        SR_NODISCARD bool empty() const;
        SR_NODISCARD const char* data() const;
        SR_NODISCARD char& back();
        SR_NODISCARD const char& back() const;
        SR_NODISCARD StringView substr(SizeType pos, SizeType count = npos) const;

        SR_NODISCARD char operator[](size_t index) const;

        SR_NODISCARD const char* begin() const;
        SR_NODISCARD const char* end() const;

        SR_NODISCARD SizeType find(const char* str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(std::string_view str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(StringView str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(const String& str, SizeType pos = 0) const;
        SR_NODISCARD SizeType find(char c, SizeType pos = 0) const;

        SR_NODISCARD bool operator==(const StringView& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const StringView& rhs) const noexcept;
        SR_NODISCARD bool operator==(const char* rhs) const noexcept;
        SR_NODISCARD bool operator!=(const char* rhs) const noexcept;
        SR_NODISCARD bool operator==(const std::string& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const std::string& rhs) const noexcept;
        SR_NODISCARD bool operator==(const String& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const String& rhs) const noexcept;
        SR_NODISCARD bool operator==(std::string_view rhs) const noexcept;
        SR_NODISCARD bool operator!=(std::string_view rhs) const noexcept;
        SR_NODISCARD bool operator<(const StringView& other) const noexcept;
        SR_NODISCARD bool operator>(const StringView& other) const noexcept;

    private:
        const char* m_data = nullptr;
        SizeType m_size = 0;

    };
}

namespace std {
    template<> struct hash<SR_UTILS_NS::String> {
        size_t operator()(SR_UTILS_NS::String const& object) const noexcept {
            return std::hash<std::string_view>()(object.view());
        }
    };

    template<> struct less<SR_UTILS_NS::String> {
        bool operator()(const SR_UTILS_NS::String& lhs, const SR_UTILS_NS::String& rhs) const noexcept {
            return lhs.view() < rhs.view();
        }
    };

    template<> struct hash<SR_UTILS_NS::StringView> {
        size_t operator()(SR_UTILS_NS::StringView const& object) const noexcept {
            return std::hash<std::string_view>()(object);
        }
    };

    template<> struct less<SR_UTILS_NS::StringView> {
        bool operator()(const SR_UTILS_NS::StringView& lhs, const SR_UTILS_NS::StringView& rhs) const noexcept {
            return lhs < rhs;
        }
    };
}

SR_COMMON_DLL_API extern SR_UTILS_NS::String operator+(const char* lhs, const SR_UTILS_NS::String& rhs);
SR_COMMON_DLL_API extern SR_UTILS_NS::String operator+(const std::string& lhs, const SR_UTILS_NS::String& rhs);
SR_COMMON_DLL_API extern SR_UTILS_NS::String operator+(std::string_view lhs, const SR_UTILS_NS::String& rhs);

template<> struct fmt::formatter<SR_UTILS_NS::String> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(SR_UTILS_NS::String const& str, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", str.view());
    }
};

template<> struct fmt::formatter<SR_UTILS_NS::StringView> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(SR_UTILS_NS::StringView const& str, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", std::string_view(str));
    }
};

#endif //SR_ENGINE_COMMON_TYPES_STRING_H
