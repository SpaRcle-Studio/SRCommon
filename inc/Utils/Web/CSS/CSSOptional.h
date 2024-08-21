//
// Created by Monika on 20.08.2024.
//

#ifndef SR_COMMON_WEB_CSS_OPTIONAL_H
#define SR_COMMON_WEB_CSS_OPTIONAL_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS::Web {
    template<class T> class CSSOptional : public T {
    public:
        CSSOptional() = default;
        CSSOptional(const T& value) : T(value), m_isDefault(false) { } /** NOLINT(google-explicit-constructor) */
        CSSOptional(const T&& value) : T(value), m_isDefault(false) { } /** NOLINT(google-explicit-constructor) */

    public:
        SR_NODISCARD bool IsDefault() const { return m_isDefault; }

    protected:
        void OnChanged() final { m_isDefault = false; }

    private:
        bool m_isDefault = true;

    };
}

#endif //SR_COMMON_WEB_CSS_OPTIONAL_H
