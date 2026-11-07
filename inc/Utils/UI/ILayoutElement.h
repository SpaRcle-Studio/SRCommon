//
// Created by Monika on 26.01.2026.
//


#ifndef SR_ENGINE_UTILS_UI_I_LAYOUT_ELEMENT_H
#define SR_ENGINE_UTILS_UI_I_LAYOUT_ELEMENT_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class ILayoutElement {
    public:
        virtual ~ILayoutElement() = default;

        virtual bool IsLayoutIgnored() const { return false; }
        virtual float GetMinWidth() const = 0;
        virtual float GetPreferredWidth() const = 0;
        virtual float GetFlexibleWidth() const = 0;
        virtual float GetMinHeight() const = 0;
        virtual float GetPreferredHeight() const = 0;
        virtual float GetFlexibleHeight() const = 0;
        virtual int32_t GetLayoutPriority() const = 0;
    };
}

#endif //SR_ENGINE_UTILS_UI_I_LAYOUT_ELEMENT_H
