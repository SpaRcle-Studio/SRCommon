//
// Created by Monika on 02.05.2026.
//

#ifndef SR_ENGINE_COMMON_TYPES_STRING_H
#define SR_ENGINE_COMMON_TYPES_STRING_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class StringView;

    class SR_COMMON_DLL_API String {
    public:
        using value_type = char;
        static constexpr auto npos{static_cast<SizeType>(-1)};

    public:
        String();
        String(const char* str); /// NOLINT
        String(const std::string& str); /// NOLINT
        String(std::string_view str); /// NOLINT
        String(const String& other); /// NOLINT
        String(String&& other) noexcept;
        ~String();

        String& operator=(const char* str);
        String& operator=(const std::string& str);
        String& operator=(std::string_view str);
        String& operator=(std::string&& str) noexcept;
        String& operator=(const String& other);
        String& operator=(StringView str);
        String& operator=(String&& other) noexcept;

        SR_NODISCARD String operator+(const String& rhs) const;
        SR_NODISCARD String operator+(const char* rhs) const;
        SR_NODISCARD String operator+(const std::string& rhs) const;
        SR_NODISCARD String operator+(std::string_view rhs) const;

        SR_NODISCARD bool operator==(const String& rhs) const noexcept;
        SR_NODISCARD bool operator!=(const String& rhs) const noexcept;
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

        SR_NODISCARD const char* c_str() const;
        SR_NODISCARD uint64_t size() const;
        SR_NODISCARD bool empty() const;
        SR_NODISCARD const char* data() const;
        SR_NODISCARD char* data();
        SR_NODISCARD char& back();
        SR_NODISCARD const char& back() const;
        SR_NODISCARD std::string_view view() const;

        SR_NODISCARD bool contains(std::string_view str) const;
        SR_NODISCARD bool starts_with(std::string_view str) const;
        SR_NODISCARD bool ends_with(std::string_view str) const;

        SR_NODISCARD String substr(uint64_t pos, uint64_t count = SR_UINT64_MAX) const;

        SR_NODISCARD uint64_t find(const char* str, uint64_t pos = 0) const;
        SR_NODISCARD uint64_t find(std::string_view str, uint64_t pos = 0) const;
        SR_NODISCARD uint64_t find(const String& str, uint64_t pos = 0) const;
        SR_NODISCARD uint64_t find(char c, uint64_t pos = 0) const;
        SR_NODISCARD uint64_t rfind(char c, uint64_t pos = SR_UINT64_MAX) const;

        SR_NODISCARD const char* begin() const;
        SR_NODISCARD const char* end() const;
        SR_NODISCARD char* begin();
        SR_NODISCARD char* end();

        char operator[](size_t index) const;
        char& operator[](size_t index);

        void push_back(char c);
        void reserve(uint64_t newSize);
        void resize(uint64_t newSize);
        void clear();

        void ToLowerInPlace();
        void ToUpperInPlace();
        void SubStrInPlace(uint64_t pos, uint64_t count = SR_UINT64_MAX);

    private:
        char* m_data = nullptr;
        uint64_t m_size = 0;
        uint64_t m_capacity = 0;

    };

    class SR_COMMON_DLL_API StringView {
    public:
        static constexpr auto npos{static_cast<SizeType>(-1)};

    public:
        StringView();
        StringView(const char* str, SizeType size); /// NOLINT
        StringView(const char* str); /// NOLINT
        StringView(const std::string& str); /// NOLINT
        StringView(std::string_view str); /// NOLINT
        StringView(const String& str); /// NOLINT

        void remove_prefix(uint64_t n);

        SR_NODISCARD operator std::string_view() const; /// NOLINT

        SR_NODISCARD const char* c_str() const;
        SR_NODISCARD uint64_t size() const;
        SR_NODISCARD bool empty() const;
        SR_NODISCARD const char* data() const;
        SR_NODISCARD char& back();
        SR_NODISCARD const char& back() const;
        SR_NODISCARD StringView substr(uint64_t pos, uint64_t count = SR_UINT64_MAX) const;

        SR_NODISCARD char operator[](size_t index) const;

        SR_NODISCARD const char* begin() const;
        SR_NODISCARD const char* end() const;

        SR_NODISCARD SizeType find(const char* str, uint64_t pos = 0) const;
        SR_NODISCARD SizeType find(std::string_view str, uint64_t pos = 0) const;
        SR_NODISCARD SizeType find(StringView str, uint64_t pos = 0) const;
        SR_NODISCARD SizeType find(const String& str, uint64_t pos = 0) const;
        SR_NODISCARD SizeType find(char c, uint64_t pos = 0) const;

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
        uint64_t m_size = 0;

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

#endif //SR_ENGINE_COMMON_TYPES_STRING_H
