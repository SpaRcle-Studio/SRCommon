//
// Created by Monika on 22.01.2023.
//

#ifndef SR_ENGINE_REGEX_H
#define SR_ENGINE_REGEX_H

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    class Regex {
    public:
        Regex() = default;
        Regex(const SR_UTILS_NS::String& regex); /** NOLINT */
        Regex(Regex&& other) noexcept;

        Regex& operator=(Regex&& other) noexcept;

    public:
        bool Search(const SR_UTILS_NS::String& input);

        SR_NODISCARD uint64_t Size() const noexcept;
        SR_NODISCARD SR_UTILS_NS::String Prefix() const noexcept;
        SR_NODISCARD SR_UTILS_NS::String Suffix() const noexcept;

        SR_NODISCARD SR_UTILS_NS::String operator[](int64_t index) const noexcept;

    private:
        std::regex m_regex;
        std::cmatch m_match;
        SR_UTILS_NS::String m_input;

    };
}

#endif //SR_ENGINE_REGEX_H
