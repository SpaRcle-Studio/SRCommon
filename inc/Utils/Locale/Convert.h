//
// Created by Monika on 23.10.2022.
//

#ifndef SR_ENGINE_CONVERT_H
#define SR_ENGINE_CONVERT_H

#include <Utils/Locale/Icu.h>

namespace SR_UTILS_NS::Locale {
    //std::string ConvertBetween(
    //     const char *begin,
    //     const char *end,
    //     const std::string& to_charset,
    //     const std::string& from_charset,
    //     EncMethodType how = EncMethodType::Default
    // ) {
    //     auto&& from = ICU::IcuStdConverter<char>(from_charset, how);
    //     auto&& to = ICU::IcuStdConverter<char>(to_charset, how);
    //     return to.Std(from.Icu(begin, end));
    // }

    template<typename CharType> std::basic_string<CharType> ConvertTo(const char *begin, const char *end, const char *charset, EncMethodType how = EncMethodType::Default) {
    #ifdef SR_ICU
        auto&& from = ICU::IcuStdConverter<char>(charset, how);
        auto&& to = ICU::IcuStdConverter<CharType>("UTF-8", how);
        return to.Std(from.IcuChecked(begin, end));
    #else
        return std::basic_string<CharType>();
    #endif
    }

    template<typename CharType> std::string ConvertFrom(const CharType *begin, const CharType *end, const char *charset, EncMethodType how = EncMethodType::Default) {
    #ifdef SR_ICU
        auto&& from = ICU::IcuStdConverter<CharType>("UTF-8", how);
        auto&& to = ICU::IcuStdConverter<char>(charset, how);
        return to.Std(from.IcuChecked(begin, end));
    #else
        return std::string();
    #endif
    }
}

#endif //SR_ENGINE_CONVERT_H
