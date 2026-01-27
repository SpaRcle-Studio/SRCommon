//
// Created by Monika on 26.01.2026.
//


#ifndef SR_ENGINE_UTILS_UI_I_LAYOUT_ELEMENT_H
#define SR_ENGINE_UTILS_UI_I_LAYOUT_ELEMENT_H

#include <Utils/ECS/Component.h>

namespace SR_UTILS_NS {
    class ILayoutElement {
        virtual float GetMinWidth() const = 0;
        virtual float GetPreferredWidth() const = 0;
        virtual float GetMinHeight() const = 0;
        virtual float GetPreferredHeight() const = 0;
    };
}

#endif //SR_ENGINE_UTILS_UI_I_LAYOUT_ELEMENT_H
