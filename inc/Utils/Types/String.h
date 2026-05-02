//
// Created by Monika on 02.05.2026.
//

#ifndef SR_ENGINE_COMMON_TYPES_STRING_H
#define SR_ENGINE_COMMON_TYPES_STRING_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API String {
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
        String& operator=(std::string&& str) noexcept;
        String& operator=(const String& other);
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
        SR_NODISCARD uint32_t size() const;
        SR_NODISCARD bool empty() const;
        SR_NODISCARD const char* data() const;
        SR_NODISCARD char& back();
        SR_NODISCARD const char& back() const;
        SR_NODISCARD std::string_view view() const;

        SR_NODISCARD String substr(uint32_t pos, uint32_t count = SR_UINT32_MAX) const;

        SR_NODISCARD uint64_t find(const char* str, uint32_t pos = 0) const;
        SR_NODISCARD uint64_t find(std::string_view str, uint32_t pos = 0) const;
        SR_NODISCARD uint64_t find(const String& str, uint32_t pos = 0) const;
        SR_NODISCARD uint64_t find(char c, uint32_t pos = 0) const;
        SR_NODISCARD uint64_t rfind(char c, uint32_t pos = SR_UINT32_MAX) const;

        SR_NODISCARD const char* begin() const;
        SR_NODISCARD const char* end() const;
        SR_NODISCARD char* begin();
        SR_NODISCARD char* end();

        char operator[](size_t index) const;
        char& operator[](size_t index);

        void reserve(uint32_t newSize);
        void resize(uint32_t newSize);
        void clear();

    private:
        char* m_data = nullptr;
        uint32_t m_size = 0;
        uint32_t m_capacity = 0;

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
}

SR_UTILS_NS::String operator+(const char* lhs, const SR_UTILS_NS::String& rhs) { return SR_UTILS_NS::String(lhs) + rhs; }
SR_UTILS_NS::String operator+(const std::string& lhs, const SR_UTILS_NS::String& rhs) { return SR_UTILS_NS::String(lhs) + rhs; }
SR_UTILS_NS::String operator+(std::string_view lhs, const SR_UTILS_NS::String& rhs) { return SR_UTILS_NS::String(lhs) + rhs; }

template<> struct fmt::formatter<SR_UTILS_NS::String> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(SR_UTILS_NS::String const& str, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", str.view());
    }
};

#endif //SR_ENGINE_COMMON_TYPES_STRING_H
