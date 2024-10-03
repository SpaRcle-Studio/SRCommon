//
// Created by Monika on 15.08.2024.
//

#ifndef SR_COMMON_WEB_CSS_H
#define SR_COMMON_WEB_CSS_H

#include <Utils/Web/CSS/CSSSizeValue.h>
#include <Utils/Web/CSS/CSSColor.h>
#include <Utils/Web/CSS/CSSEnums.h>
#include <Utils/Types/SharedPtr.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Debug.h>

namespace SR_UTILS_NS::Web {
    struct CSSStyle {
        CSSStyle() = default;

        void ParseProperty(std::string_view name, std::string_view data);

        SR_NODISCARD std::string ToString(uint16_t depth = 0) const;

        static CSSStyle Merge(const CSSStyle& main, const CSSStyle& other);

        CSSOptionalSizeValue /// content size
            width, // = CSSSizeValue(0.f, CSSSizeValue::Unit::Percent),
            height; // = CSSSizeValue(0.f, CSSSizeValue::Unit::Percent);

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

        CSSOptionalEnum<CSSDisplay> display = CSSOptionalEnum<CSSDisplay>::CreateDefault(DEFAULT_CSS_DISPLAY);

        CSSPosition position = CSSPosition::Static;

        CSSColor color = DEFAULT_CSS_COLOR;
        CSSColor backgroundColor = DEFAULT_CSS_COLOR;

        CSSBoxSizing boxSizing = CSSBoxSizing::ContentBox;
    };

    class CSS : public SR_HTYPES_NS::SharedPtr<CSS> {
        using Super = SR_HTYPES_NS::SharedPtr<CSS>;
    public:
        using Ptr = SR_HTYPES_NS::SharedPtr<CSS>;

    public:
        CSS();

    public:
        SR_NODISCARD std::string ToString(uint16_t depth = 0) const;

        SR_NODISCARD const CSSStyle* GetClassStyle(const std::string& token) const {
            SR_TRACY_ZONE;
            const auto it = m_styles.find(std::make_pair(token, true));
            return it != m_styles.end() ? &it->second : nullptr;
        }

        SR_NODISCARD const CSSStyle* GetTagStyle(const std::string& token) const {
            SR_TRACY_ZONE;
            const auto it = m_styles.find(std::make_pair(token, false));
            return it != m_styles.end() ? &it->second : nullptr;
        }

        void AddClassStyle(std::string&& token, const CSSStyle& style) {
            SRAssert2(!token.empty(), "Token is empty!");
            m_styles.emplace(std::make_pair(std::move(token), true), style);
        }

        void AddTagStyle(std::string&& token, const CSSStyle& style) {
            SRAssert2(!token.empty(), "Token is empty!");
            m_styles.emplace(std::make_pair(std::move(token), false), style);
        }

        SR_NODISCARD const CSSStyle* GetGlobalStyle() const {
            return &m_globalStyle;
        }

        SR_NODISCARD CSSStyle* GetOrCreateStyle(std::string_view token, bool isClass) {
            SRAssert2(!token.empty(), "Token is empty!");
            if (token == "*") { return &m_globalStyle; }
            const auto pIt = std::ranges::find_if(m_styles, [token, isClass](const auto& pair) {
                return pair.first.first == token && pair.first.second == isClass;
            });
            if (pIt != m_styles.end()) {
                return &pIt->second;
            }
            return &m_styles.emplace(std::make_pair(token, isClass), m_globalStyle).first->second;
        }

    private:
        CSSStyle m_globalStyle;
        std::map<std::pair<std::string, bool /** is class */>, CSSStyle> m_styles;

    };
}

#endif //SR_COMMON_WEB_CSS_H
