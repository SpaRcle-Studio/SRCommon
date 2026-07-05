//
// Created by Monika on 02.05.2026.
//

#include <Utils/Types/String.h>
#include <Utils/Common/AssertFwd.h>

namespace SR_UTILS_NS {
    String::String() = default;

    String::String(const char* str, uint64_t size, IAllocator* pAllocator)
        : m_allocator(pAllocator)
    {
        resize(size);
        if (m_data) {
            memcpy(m_data, str, m_size);
        }
    }

    String::String(StringView str, IAllocator* pAllocator)
        : String(str.data(), str.size(), pAllocator)
    { }

    String::String(String str, IAllocator* pAllocator)
        : String(str.m_data, str.m_size, pAllocator)
    { }

    String::String(const std::string& str, IAllocator* pAllocator)
        : String(str.data(), static_cast<uint64_t>(str.size()), pAllocator)
    { }

    String::String(std::string_view str, IAllocator* pAllocator)
        : String(str.data(), static_cast<uint64_t>(str.size()), pAllocator)
    { }

    String::String(const char* str)
        : String(std::string_view(str))
    { }

    String::String(const std::string& str)
        : String(std::string_view(str))
    { }

    String::String(std::string_view str) {
        resize(static_cast<uint64_t>(str.size()));
        if (m_data) {
            memcpy(m_data, str.data(), m_size);
        }
    }

    String::String(const String& other)
        : String(other.m_data, other.m_size, other.m_allocator)
    { }

    String::String(String&& other) noexcept
        : m_data(other.m_data)
        , m_size(other.m_size)
        , m_capacity(other.m_capacity)
        , m_allocator(other.m_allocator)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
        other.m_allocator = nullptr;
    }

    String::~String() {
        if (m_data) {
            if (m_allocator) {
                m_allocator->Free(m_data, m_capacity + 1);
            }
            else {
                SRFree(m_data);
            }
        }
    }

    String& String::operator=(const char* str) {
        return *this = std::string_view(str);
    }

    String& String::operator=(const std::string& str) {
        return *this = std::string_view(str);
    }

    String& String::operator=(std::string_view str) {
        resize(static_cast<uint64_t>(str.size()));
        if (m_data) {
            memcpy(m_data, str.data(), m_size);
        }
        return *this;
    }

    String& String::operator=(StringView str) {
        return *this = std::string_view(str.data(), str.size());
    }

    String& String::operator=(std::string&& str) noexcept {
        return *this = std::string_view(str);
    }

    String& String::operator=(const String& other) {
        if (this == &other) {
            return *this;
        }
        return *this = String(other.m_data, other.m_size, other.m_allocator);
    }

    String& String::operator=(String&& other) noexcept {
        if (this != &other) {
            if (m_data) {
                if (m_allocator) {
                    m_allocator->Free(m_data, m_capacity + 1);
                }
                else {
                    SRFree(m_data);
                }
            }
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_allocator = other.m_allocator;

            other.m_allocator = nullptr;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    String& String::operator+=(const String& rhs) { return *this += std::string_view(rhs.m_data, rhs.m_size); }
    String& String::operator+=(const StringView& rhs) { return *this += std::string_view(rhs.data(), rhs.size()); }
    String& String::operator+=(const char* rhs) { return *this += std::string_view(rhs); }
    String& String::operator+=(const std::string& rhs) { return *this += std::string_view(rhs); }
    String& String::operator+=(std::string_view rhs) {
        if (rhs.empty()) {
            return *this;
        }
        uint64_t newSize = m_size + static_cast<uint64_t>(rhs.size());
        if (newSize > m_capacity) {
            reserve(SR_MAX(newSize, m_capacity * 2));
        }
        m_size = newSize;
        memcpy(m_data + m_size - rhs.size(), rhs.data(), rhs.size());
        m_data[newSize] = '\0';
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
        uint64_t newSize = m_size + static_cast<uint64_t>(rhs.size());
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

    uint64_t String::size() const { return m_size; }
    uint64_t String::capacity() const { return m_capacity; }

    bool String::empty() const { return m_size == 0; }

    const char* String::data() const { return m_data; }
    char* String::data() { return m_data; }

    const char* String::begin() const { return m_data; }
    const char* String::end() const { return m_data ? m_data + m_size : nullptr; }
    char* String::begin() { return m_data; }
    char* String::end() { return const_cast<char*>(m_data ? m_data + m_size : nullptr); }

    uint64_t String::find(const char* str, uint64_t pos) const { return find(std::string_view(str), pos); }
    uint64_t String::find(const String& str, uint64_t pos) const { return find(std::string_view(str.m_data, str.m_size), pos); }
    uint64_t String::find(std::string_view str, uint64_t pos) const {
        if (pos >= m_size || empty()) {
            return SR_UINT64_MAX;
        }
        const char* found = std::search(m_data + pos, m_data + m_size, str.data(), str.data() + str.size());
        if (found != m_data + m_size) {
            return static_cast<uint64_t>(found - m_data);
        }
        return SR_UINT64_MAX;
    }
    uint64_t String::find(char c, uint64_t pos) const {
        if (pos >= m_size || empty()) {
            return SR_UINT64_MAX;
        }
        const char* found = static_cast<const char*>(memchr(m_data + pos, c, m_size - pos));
        if (found) {
            return static_cast<uint64_t>(found - m_data);
        }
        return SR_UINT64_MAX;
    }
    uint64_t String::rfind(char c, uint64_t pos) const {
        if (empty()) {
            return SR_UINT64_MAX;
        }
        uint64_t searchStart = pos < m_size ? pos : m_size - 1;
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

    bool String::starts_with(std::string_view prefix) const {
        if (prefix.size() > m_size) {
            return false;
        }
        return std::equal(prefix.begin(), prefix.end(), m_data);
    }

    bool String::ends_with(std::string_view suffix) const {
        const size_t n = suffix.size();
        return n <= m_size && std::memcmp(m_data + m_size - n, suffix.data(), n) == 0;
    }

    bool String::contains(std::string_view str) const {
        return find(str) != SR_UINT64_MAX;
    }

    String String::substr(uint64_t pos, uint64_t count) const {
        if (pos >= m_size || empty()) {
            return String();
        }
        uint64_t maxCount = m_size - pos;
        if (count > maxCount) {
            count = maxCount;
        }
        return String(std::string_view(m_data + pos, count));
    }

    void String::reserve(uint64_t newSize) {
        if (newSize <= m_capacity) {
            return;
        }
        char* newData = (char*)(m_allocator ? m_allocator->Allocate(newSize + 1) : SRMalloc(newSize + 1));
        if (m_data) {
            memcpy(newData, m_data, m_size);
            newData[m_size] = '\0';
            if (m_allocator) {
                m_allocator->Free(m_data, m_capacity + 1);
            }
            else {
                SRFree(m_data);
            }
        }
        m_data = newData;
        m_capacity = newSize;
    }

    void String::append(const String& str) {
        append(std::string_view(str.m_data, str.m_size));
    }
    void String::append(const StringView& str) {
        append(std::string_view(str.data(), str.size()));
    }
    void String::append(const char* str) {
        append(std::string_view(str));
    }
    void String::append(const std::string& str) {
        append(std::string_view(str));
    }

    void String::append(std::string_view str) {
        if (str.empty()) {
            return;
        }
        uint64_t newSize = m_size + static_cast<uint64_t>(str.size());
        if (newSize > m_capacity) {
            reserve(SR_MAX(newSize, m_capacity * 2));
        }
        memcpy(m_data + m_size, str.data(), str.size());
        m_size = newSize;
        m_data[newSize] = '\0';
    }

    void String::append(char c) {
        if (m_size + 1 > m_capacity) {
            reserve(m_capacity > 0 ? m_capacity * 2 : 8);
        }
        m_data[m_size] = c;
        m_size++;
        m_data[m_size] = '\0';
    }

    void String::append(const char* str, uint64_t count) {
        if (count == 0) {
            return;
        }
        uint64_t newSize = m_size + count;
        if (newSize > m_capacity) {
            reserve(SR_MAX(newSize, m_capacity * 2));
        }
        memcpy(m_data + m_size, str, count);
        m_size = newSize;
        m_data[newSize] = '\0';
    }

    void String::push_back(char c) {
        if (m_size + 1 > m_capacity) {
            reserve(m_capacity > 0 ? m_capacity * 2 : 8);
        }
        m_data[m_size] = c;
        m_size++;
        m_data[m_size] = '\0';
    }

    void String::resize(uint64_t newSize) {
        reserve(newSize);
        if (m_data) {
            m_data[newSize] = '\0';
        }
        m_size = newSize;
    }

    void String::resize(uint64_t newSize, char fillChar) {
        if (newSize > m_size) {
            reserve(newSize);
            if (m_data) {
                memset(m_data + m_size, fillChar, newSize - m_size);
            }
        }
        m_size = newSize;
        if (m_data) {
            m_data[m_size] = '\0';
        }
    }

    void String::clear() {
        if (m_data) {
            m_data[0] = '\0';
        }
        m_size = 0;
    }

    void String::ToLowerInPlace() {
        for (uint64_t i = 0; i < m_size; ++i) {
            m_data[i] = static_cast<char>(tolower(static_cast<unsigned char>(m_data[i])));
        }
    }

    void String::ToUpperInPlace() {
        for (uint64_t i = 0; i < m_size; ++i) {
            m_data[i] = static_cast<char>(toupper(static_cast<unsigned char>(m_data[i])));
        }
    }

    void String::SubStrInPlace(uint64_t pos, uint64_t count) {
        if (pos >= m_size || empty()) {
            clear();
            return;
        }
        uint64_t maxCount = m_size - pos;
        if (count > maxCount) {
            count = maxCount;
        }
        memmove(m_data, m_data + pos, count);
        m_size = count;
        m_data[m_size] = '\0';
    }

    String String::DetachAllocator() const {
        return String(m_data, m_size, nullptr);
    }

    bool String::HasAllocator() const {
        return m_allocator;
    }

    void StringView::remove_prefix(uint64_t n) {
        if (n > m_size) {
            SRHalt("StringView::remove_prefix() : prefix size exceeds string size!");
            m_data = nullptr;
            m_size = 0;
            return;
        }
        m_data += n;
        m_size -= n;
    }

    StringView::operator std::string_view() const {
        return std::string_view(m_data ? m_data : "", m_size);
    }

    const char* StringView::c_str() const { return m_data ? m_data : ""; }
    uint64_t StringView::size() const { return m_size; }
    bool StringView::empty() const { return m_size == 0; }
    const char* StringView::data() const { return m_data; }
    char& StringView::back() {
        if (m_size == 0) {
            SRHalt("StringView::back() : string is empty!");
            static char dummy = '\0';
            return dummy;
        }
        return const_cast<char*>(m_data)[m_size - 1];
    }
    const char& StringView::back() const { return const_cast<StringView*>(this)->back(); }

    StringView StringView::substr(uint64_t pos, uint64_t count) const {
        if (pos >= m_size || empty()) {
            return StringView();
        }
        uint64_t maxCount = m_size - pos;
        if (count > maxCount) {
            count = maxCount;
        }
        return StringView(std::string_view(m_data + pos, count));
    }


    char StringView::operator[](size_t index) const {
        if (index >= m_size) {
            SRHalt("StringView::operator[]() : index out of bounds!");
            return '\0';
        }
        return m_data[index];
    }

    const char* StringView::begin() const { return m_data; }
    const char* StringView::end() const { return m_data ? m_data + m_size : nullptr; }

    SizeType StringView::find(const char* str, uint64_t pos) const {
        return find(StringView(str), pos);
    }

    SizeType StringView::find(std::string_view str, uint64_t pos) const {
        return find(StringView(str));
    }

    SizeType StringView::find(StringView str, uint64_t pos) const {
        if (pos >= m_size || empty()) {
            return SR_UINT64_MAX;
        }
        const char* found = std::search(m_data + pos, m_data + m_size, str.data(), str.data() + str.size());
        if (found != m_data + m_size) {
            return static_cast<uint64_t>(found - m_data);
        }
        return SR_UINT64_MAX;
    }

    SizeType StringView::find(const String& str, uint64_t pos) const {
        return find(StringView(str));
    }

    SizeType StringView::find(char c, uint64_t pos) const {
        if (pos >= m_size || empty()) {
            return SR_UINT64_MAX;
        }
        const char* found = static_cast<const char*>(memchr(m_data + pos, c, m_size - pos));
        if (found) {
            return static_cast<uint64_t>(found - m_data);
        }
        return SR_UINT64_MAX;
    }

    bool StringView::operator==(const StringView& rhs) const noexcept {
        if (m_size != rhs.m_size) {
            return false;
        }
        if (m_data == rhs.m_data) {
            return true; // same pointer, considered equal
        }
        return m_data && rhs.m_data && memcmp(m_data, rhs.m_data, m_size) == 0;
    }
    bool StringView::operator!=(const StringView& rhs) const noexcept { return !(*this == rhs); }
    bool StringView::operator==(const char* rhs) const noexcept { return *this == std::string_view(rhs); }
    bool StringView::operator!=(const char* rhs) const noexcept { return !(*this == rhs); }
    bool StringView::operator==(const std::string& rhs) const noexcept { return *this == std::string_view(rhs); }
    bool StringView::operator!=(const std::string& rhs) const noexcept { return !(*this == rhs); }
    bool StringView::operator==(const String& rhs) const noexcept { return *this == std::string_view(rhs.data(), rhs.size()); }
    bool StringView::operator!=(const String& rhs) const noexcept { return !(*this == rhs); }
    bool StringView::operator==(std::string_view rhs) const noexcept { return *this == StringView(rhs); }
    bool StringView::operator!=(std::string_view rhs) const noexcept { return !(*this == rhs); }
    bool StringView::operator<(const StringView& other) const noexcept {
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
    bool StringView::operator>(const StringView& other) const noexcept { return other < *this; }
}

SR_UTILS_NS::String operator+(const char* lhs, const SR_UTILS_NS::String& rhs) {
    return SR_UTILS_NS::String(lhs) + rhs;
}
SR_UTILS_NS::String operator+(const std::string& lhs, const SR_UTILS_NS::String& rhs) {
    return SR_UTILS_NS::String(lhs) + rhs;
}
SR_UTILS_NS::String operator+(std::string_view lhs, const SR_UTILS_NS::String& rhs) {
    return SR_UTILS_NS::String(lhs) + rhs;
}