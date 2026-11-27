//
// Created by Monika on 23.10.2022.
//

#ifndef SR_ENGINE_ENCODING_H
#define SR_ENGINE_ENCODING_H

#include <Utils/Localization/Convert.h>
#include <Utils/Localization/UTF.h>
#include <Utils/Localization/EncMethodType.h>
#include <Utils/Common/AssertFwd.h>

namespace SR_UTILS_NS::Localization {
    SR_MAYBE_UNUSED static void SetLocale() {
        setlocale(LC_ALL, "rus");
        setlocale(LC_NUMERIC, "C");
    }

    template<typename CharOut, typename CharIn, typename ResultType> void UtfToUtf(ResultType& result, CharIn const* begin, CharIn const* end, EncMethodType how = EncMethodType::Default) {
        static_assert(std::is_same_v<typename ResultType::value_type, CharOut>, "UtfToUtf() : result char type mismatch!");

        result.clear();
        result.reserve(end - begin);

        typedef std::back_insert_iterator<ResultType> inserter_type;
        inserter_type inserter(result);

        Utf::CodePoint c;

        while (begin != end) {
            c = Utf::UtfTraits<CharIn>::template Decode<CharIn const *>(begin, end);
            if (c == Utf::Illegal || c == Utf::Incomplete) {
                if (how == EncMethodType::Stop) {
                    SRHalt("Conversion error!");
                    result.clear();
                    return;
                }
            }
            else {
                Utf::UtfTraits<CharOut>::template Encode<inserter_type>(c, inserter);
            }
        }
    }

    ///
    /// Convert a Unicode NUL terminated string \a str other Unicode encoding
    ///
    template<typename CharOut, typename CharIn, typename ResultType> void UtfToUtf(ResultType& result, CharIn const *str, EncMethodType how = EncMethodType::Default) {
        CharIn const *end = str;

        while (*end) {
            end++;
        }

        UtfToUtf<CharOut, CharIn>(result, str, end, how);
    }

    ///
    /// Convert a Unicode string \a str other Unicode encoding
    ///
    template<typename CharOut, typename CharIn, typename ResultType> void UtfToUtf(ResultType& result, std::basic_string_view<CharIn> const& str, EncMethodType how = EncMethodType::Default) {
        UtfToUtf<CharOut, CharIn>(result, str.data(), str.data() + str.size(), how);
    }
}

#endif //SR_ENGINE_ENCODING_H
