//
// Created by Monika on 22.01.2023.
//

#include <Utils/Types/Regex.h>
#include <Utils/Debug.h>

namespace SR_HTYPES_NS {
    Regex::Regex(const SR_UTILS_NS::String& regex)
        : m_regex(std::regex(regex.data(), regex.size()))
    { }

    Regex::Regex(Regex&& other) noexcept
        : m_regex(SR_EXCHANGE(other.m_regex, std::regex()))
        , m_match(SR_EXCHANGE(other.m_match, std::cmatch()))
        , m_input(SR_EXCHANGE(other.m_input, SR_UTILS_NS::String()))
    { }

    Regex &Regex::operator=(Regex&& other) noexcept {
        m_regex = SR_EXCHANGE(other.m_regex, std::regex());
        m_match = SR_EXCHANGE(other.m_match, std::cmatch());
        m_input = SR_EXCHANGE(other.m_input, SR_UTILS_NS::String());
        return *this;
    }

    bool Regex::Search(const SR_UTILS_NS::String& input) {
        m_input = input;

        const char* pBegin = m_input.c_str();
        const char* pEnd = pBegin + m_input.size();

        return std::regex_search(pBegin, pEnd, m_match, m_regex);
    }

    uint64_t Regex::Size() const noexcept {
        return m_match.size();
    }

    SR_UTILS_NS::String Regex::Suffix() const noexcept {
        auto&& subMatch = m_match.suffix();
        return SR_UTILS_NS::String(std::string_view(subMatch.first, static_cast<SR_UTILS_NS::SizeType>(subMatch.length())));
    }

    SR_UTILS_NS::String Regex::Prefix() const noexcept {
        auto&& subMatch = m_match.prefix();
        return SR_UTILS_NS::String(std::string_view(subMatch.first, static_cast<SR_UTILS_NS::SizeType>(subMatch.length())));
    }

    SR_UTILS_NS::String Regex::operator[](int64_t index) const noexcept {
        if (index < static_cast<int64_t>(Size())) {
            auto&& subMatch = m_match[index];
            return SR_UTILS_NS::String(std::string_view(subMatch.first, static_cast<SR_UTILS_NS::SizeType>(subMatch.length())));
        }

        SRHalt("Regex::operator[]() : out of range!");

        return SR_UTILS_NS::String();
    }
}
