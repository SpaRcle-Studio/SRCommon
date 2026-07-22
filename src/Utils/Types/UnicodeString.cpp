//
// Created by Monika on 23.10.2022.
//

#include <Utils/Types/UnicodeString.h>
#include <Utils/Localization/Encoding.h>

namespace SR_UTILS_NS {
    UnicodeString::UnicodeString(const UnicodeString& other)
        : m_internal(other.m_internal)
    { }

    UnicodeString::UnicodeString(UnicodeString &&other) noexcept
        : m_internal(std::exchange(other.m_internal, { }))
    { }

    UnicodeString &UnicodeString::operator=(const UnicodeString& other) {
        m_internal = other.m_internal;
        return *this;
    }

    UnicodeString &UnicodeString::operator=(UnicodeString&& other) noexcept {
        m_internal = std::exchange(other.m_internal, { });
        return *this;
    }

    UnicodeString &UnicodeString::operator=(const std::u32string& other) {
        m_internal = other;
        return *this;
    }

    UnicodeString &UnicodeString::operator=(std::u32string&& other) noexcept {
        m_internal = std::exchange(other, { });
        return *this;
    }

    UnicodeString::UnicodeString(const std::string& str)
        : m_internal(std::begin(str), std::end(str))
    { }

    UnicodeString::UnicodeString(const std::u16string& str)
        : m_internal(std::begin(str), std::end(str))
    { }

    UnicodeString::UnicodeString(const std::u32string &str)
        : m_internal(str)
    { }

    UnicodeString::CharType& UnicodeString::operator[](size_t position) noexcept {
        return m_internal[position];
    }

    const UnicodeString::CharType& UnicodeString::operator[](size_t position) const noexcept {
        return m_internal[position];
    }

    void UnicodeString::resize(SizeType size) {
        m_internal.resize(size);
    }

    void UnicodeString::reserve(SizeType size) {
        m_internal.reserve(size);
    }

    void UnicodeString::clear() noexcept {
        m_internal.clear();
    }
}
