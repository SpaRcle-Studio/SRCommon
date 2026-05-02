//
// Created by Monika on 02.05.2026.
//

#include <Utils/Types/String.h>

namespace SR_UTILS_NS {
    String::String() = default;

    String::String(const char* str)
        : String(std::string_view(str))
    { }

    String::String(const std::string& str)
        : String(std::string_view(str))
    { }

    String::String(std::string_view str) {
        resize(static_cast<uint32_t>(str.size()));
        if (m_data) {
            memcpy(m_data, str.data(), m_size);
        }
    }

    String::String(const String& other)
        : String(std::string_view(other.m_data, other.m_size))
    { }

    String::~String() {
        if (m_data) {
            SRFree(m_data);
        }
    }

    String& String::operator=(const char* str) {
        return *this = std::string_view(str);
    }

    String& String::operator=(const std::string& str) {
        return *this = std::string_view(str);
    }

    String& String::operator=(std::string&& str) noexcept {
        return *this = std::string_view(str);
    }

    String& String::operator=(const String& other) {
        if (this == &other) {
            return *this;
        }
        return *this = std::string_view(other.m_data, other.m_size);
    }

    String& String::operator=(String&& other) noexcept {
        if (this != &other) {
            if (m_data) {
                SRFree(m_data);
            }
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    String String::operator+(const String& rhs) const { return *this + std::string_view(rhs.m_data, rhs.m_size); }
    String String::operator+(const char* rhs) const { return *this + std::string_view(rhs); }
    String String::operator+(const std::string& rhs) const { return *this + std::string_view(rhs); }

    String String::operator+(std::string_view rhs) const {
        if (m_size == 0) {
            return String(rhs);
        }
        if (rhs.empty()) {
            return *this;
        }
        String result;
        uint32_t newSize = m_size + static_cast<uint32_t>(rhs.size());
        result.resize(newSize);
        memcpy(result.m_data, m_data, m_size);
        memcpy(result.m_data + m_size, rhs.data(), rhs.size());
        result.m_data[newSize] = '\0';
        return result;
    }

    bool String::operator==(const String& rhs) const noexcept {
        if (m_size != rhs.m_size) {
            return false;
        }

        if (m_size == 0) {
            return true;
        }

        return memcmp(m_data, rhs.m_data, m_size) == 0;
    }
    bool String::operator!=(const String& rhs) const noexcept { return !(*this == rhs); }
    bool String::operator==(const char* rhs) const noexcept {
        if (!rhs) {
            return m_size == 0;
        }
        size_t rhsSize = strlen(rhs);
        if (m_size != rhsSize) {
            return false;
        }
        return m_data && memcmp(m_data, rhs, m_size) == 0;
    }
    bool String::operator!=(const char* rhs) const noexcept { return !(*this == rhs); }
    bool String::operator==(const std::string& rhs) const noexcept { return *this == std::string_view(rhs); }
    bool String::operator!=(const std::string& rhs) const noexcept { return !(*this == rhs); }
    bool String::operator==(std::string_view rhs) const noexcept {
        if (m_size != rhs.size()) {
            return false;
        }
        return m_data && memcmp(m_data, rhs.data(), m_size) == 0;
    }
    bool String::operator!=(std::string_view rhs) const noexcept { return !(*this == rhs); }
    bool String::operator<(const String& other) const noexcept {
        if (m_data == other.m_data) {
            return false; // same pointer, considered equal
        }
        if (!m_data) {
            return true; // null is less than non-null
        }
        if (!other.m_data) {
            return false; // non-null is greater than null
        }
        int cmp = memcmp(m_data, other.m_data, std::min(m_size, other.m_size));
        if (cmp == 0) {
            return m_size < other.m_size; // shorter string is less if prefixes are equal
        }
        return cmp < 0;
    }
    bool String::operator>(const String& other) const noexcept { return other < *this; }

    char String::operator[](size_t index) const {
        if (index >= m_size) {
            SRHalt("String::operator[]() : index out of bounds!");
            return '\0';
        }
        return m_data[index];
    }

    char& String::operator[](size_t index) {
        if (index >= m_size) {
            SRHalt("String::operator[]() : index out of bounds!");
            static char dummy = '\0';
            return dummy;
        }
        return m_data[index];
    }

    String::operator std::string() const {
        return std::string(m_data ? m_data : "", m_size);
    }

    String::operator std::string_view() const {
        return std::string_view(m_data ? m_data : "", m_size);
    }

    const char* String::c_str() const { return m_data ? m_data : nullptr; }

    uint32_t String::size() const { return m_size; }

    bool String::empty() const { return m_size == 0; }

    const char* String::data() const { return m_data; }

    const char* String::begin() const { return m_data; }
    const char* String::end() const { return m_data ? m_data + m_size : nullptr; }
    char* String::begin() { return m_data; }
    char* String::end() { return const_cast<char*>(m_data ? m_data + m_size : nullptr); }

    uint64_t String::find(const char* str, uint32_t pos) const { return find(std::string_view(str), pos); }
    uint64_t String::find(const String& str, uint32_t pos) const { return find(std::string_view(str.m_data, str.m_size), pos); }
    uint64_t String::find(std::string_view str, uint32_t pos) const {
        if (pos >= m_size || empty()) {
            return SR_UINT64_MAX;
        }
        const char* found = std::search(m_data + pos, m_data + m_size, str.data(), str.data() + str.size());
        if (found != m_data + m_size) {
            return static_cast<uint64_t>(found - m_data);
        }
        return SR_UINT64_MAX;
    }
    uint64_t String::find(char c, uint32_t pos) const {
        if (pos >= m_size || empty()) {
            return SR_UINT64_MAX;
        }
        const char* found = static_cast<const char*>(memchr(m_data + pos, c, m_size - pos));
        if (found) {
            return static_cast<uint64_t>(found - m_data);
        }
        return SR_UINT64_MAX;
    }
    uint64_t String::rfind(char c, uint32_t pos) const {
        if (empty()) {
            return SR_UINT64_MAX;
        }
        uint32_t searchStart = pos < m_size ? pos : m_size - 1;
        for (int64_t i = searchStart; i >= 0; --i) {
            if (m_data[i] == c) {
                return static_cast<uint64_t>(i);
            }
        }
        return SR_UINT64_MAX;
    }

    char& String::back() {
        if (m_size == 0) {
            SRHalt("String::back() : string is empty!");
            static char dummy = '\0';
            return dummy;
        }
        return m_data[m_size - 1];
    }
    const char& String::back() const { return const_cast<String*>(this)->back(); }

    std::string_view String::view() const {
        return std::string_view(m_data ? m_data : "", m_size);
    }

    String String::substr(uint32_t pos, uint32_t count) const {
        if (pos >= m_size || empty()) {
            return String();
        }
        uint32_t maxCount = m_size - pos;
        if (count > maxCount) {
            count = maxCount;
        }
        return String(std::string_view(m_data + pos, count));
    }

    void String::reserve(uint32_t newSize) {
        if (newSize <= m_capacity) {
            return;
        }
        char* newData = (char*)SRMalloc(newSize + 1);
        if (m_data) {
            memcpy(newData, m_data, m_size);
            newData[m_size] = '\0';
            SRFree(m_data);
        }
        m_data = newData;
        m_capacity = newSize;
    }

    String::String(String&& other) noexcept
        : m_data(other.m_data)
        , m_size(other.m_size)
        , m_capacity(other.m_capacity)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    void String::resize(uint32_t newSize) {
        reserve(newSize);
        if (m_data) {
            m_data[newSize] = '\0';
        }
        m_size = newSize;
    }

    void String::clear() {
        if (m_data) {
            m_data[0] = '\0';
        }
        m_size = 0;
    }
}