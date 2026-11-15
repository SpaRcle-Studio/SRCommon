//
// Created by Codex on 15.11.2026.
//

#include <Utils/Localization/Grapheme.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS::Localization {
    bool IsCombiningCodepoint(uint32_t cp) {
        return (cp >= 0x0300 && cp <= 0x036F) || (cp >= 0x0483 && cp <= 0x0489)
            || (cp >= 0x1AB0 && cp <= 0x1AFF) || (cp >= 0x1DC0 && cp <= 0x1DFF)
            || (cp >= 0x20D0 && cp <= 0x20FF) || (cp >= 0xFE20 && cp <= 0xFE2F)
            || (cp >= 0xFE00 && cp <= 0xFE0F) || (cp >= 0xE0100 && cp <= 0xE01EF)
            || (cp >= 0x1F3FB && cp <= 0x1F3FF);
    }

    bool DecodeGraphemes(StringView text, Vector<GraphemeCodepoint>& result, uint32_t& count) {
        SR_TRACY_ZONE;
        result.clear();
        count = 0;
        uint32_t previous = 0;
        uint32_t regionalCount = 0;
        for (uint64_t offset = 0; offset < text.size();) {
            const auto begin = offset;
            const auto lead = static_cast<uint8_t>(text[offset++]);
            uint32_t cp = lead;
            uint32_t remaining = 0;
            uint32_t minimum = 0;
            if (lead >= 0xC2 && lead <= 0xDF) {
                cp = lead & 0x1F;
                remaining = 1;
                minimum = 0x80;
            }
            else if (lead >= 0xE0 && lead <= 0xEF) {
                cp = lead & 0x0F;
                remaining = 2;
                minimum = 0x800;
            }
            else if (lead >= 0xF0 && lead <= 0xF4) {
                cp = lead & 0x07;
                remaining = 3;
                minimum = 0x10000;
            }
            else if (lead >= 0x80) {
                SR_ERROR("DecodeGraphemes() : invalid UTF-8 at byte {}", begin);
                return false;
            }
            for (uint32_t i = 0; i < remaining; ++i) {
                if (offset >= text.size() || (static_cast<uint8_t>(text[offset]) & 0xC0) != 0x80) {
                    SR_ERROR("DecodeGraphemes() : invalid UTF-8 continuation at byte {}", offset);
                    return false;
                }
                cp = (cp << 6) | (static_cast<uint8_t>(text[offset++]) & 0x3F);
            }
            if (cp < minimum || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
                SR_ERROR("DecodeGraphemes() : invalid UTF-8 scalar at byte {}", begin);
                return false;
            }
            const bool regional = cp >= 0x1F1E6 && cp <= 0x1F1FF;
            const bool joined = !result.empty() && (IsCombiningCodepoint(cp) || cp == 0x200D || previous == 0x200D
                || (previous == '\r' && cp == '\n') || (regional && regionalCount % 2 == 1));
            if (!joined) {
                ++count;
            }
            result.emplace_back(GraphemeCodepoint{ cp, count - 1, begin });
            previous = cp;
            regionalCount = regional ? regionalCount + 1 : 0;
        }
        return true;
    }
}
