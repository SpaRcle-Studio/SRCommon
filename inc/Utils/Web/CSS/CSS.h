//
// Created by Monika on 15.08.2024.
//

#ifndef SR_COMMON_WEB_CSS_H
#define SR_COMMON_WEB_CSS_H

#include <Utils/Web/CSS/CSSSizeValue.h>
#include <Utils/Web/CSS/CSSColor.h>
#include <Utils/Web/CSS/CSSEnums.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS::Web {
    struct CSSStyle {
        CSSStyle() = default;

        void ParseProperty(std::string_view name, std::string_view data);

        SR_NODISCARD std::string ToString(uint16_t depth = 0) const;

        CSSOptionalSizeValue width, height; /// content size
        CSSOptionalSizeValue minWidth, minHeight;
        CSSOptionalSizeValue maxWidth, maxHeight;

        CSSOptionalSizeValue top, right, bottom, left;
        CSSOptionalSizeValue marginTop, marginRight, marginBottom, marginLeft;

        CSSOptionalSizeValue paddingTop, paddingRight, paddingBottom, paddingLeft;
        CSSOptionalSizeValue borderTop, borderRight, borderBottom, borderLeft;

        /**                     margin-top
        *                       border-top
        *                      padding-top
        *  m    b     p                        p      b      m
        *  -    -     -         content        -      -      -
        *  left left  left                     right  right  right
        *                     padding-bottom
        *                     border-bottom
        *                     margin-bottom
        **/

        CSSOptionalSizeValue opacity;
        CSSOptionalSizeValue zIndex;

        CSSDisplay display = DEFAULT_CSS_DISPLAY;

        CSSColor color = DEFAULT_CSS_COLOR;
        CSSColor backgroundColor = DEFAULT_CSS_COLOR;
    };

    enum class CSSPosition : uint8_t {
        Absolute,
        Relative,
        Fixed,
        Static,
        Sticky,
    };

    class CSS : public SR_HTYPES_NS::SharedPtr<CSS> {
        using Super = SR_HTYPES_NS::SharedPtr<CSS>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<CSS>;

    public:
        CSS();

    public:
        SR_NODISCARD std::string ToString(uint16_t depth = 0) const;

        void AddStyle(std::string&& token, const CSSStyle& style) {
            m_tokens.emplace(std::move(token), style);
        }

    private:
        std::map<std::string, CSSStyle> m_tokens;

    };
}

#endif //SR_COMMON_WEB_CSS_H
