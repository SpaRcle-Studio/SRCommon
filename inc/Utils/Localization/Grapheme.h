//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_GRAPHEME_H
#define SR_ENGINE_UTILS_GRAPHEME_H

#include <Utils/Types/Vector.h>
#include <Utils/Types/String.h>

namespace SR_UTILS_NS::Localization {
    struct GraphemeCodepoint {
        uint32_t codepoint = 0;
        uint32_t grapheme = 0;
        uint64_t byteOffset = 0;
    };

    SR_COMMON_DLL_API bool DecodeGraphemes(StringView text, Vector<GraphemeCodepoint>& result, uint32_t& count);
    SR_COMMON_DLL_API bool IsCombiningCodepoint(uint32_t codepoint);
}

#endif // SR_ENGINE_UTILS_GRAPHEME_H
