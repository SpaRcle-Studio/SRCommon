//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_LAYOUT_UTILITY_H
#define SR_ENGINE_UTILS_UI_LAYOUT_UTILITY_H

#include <Utils/stdInclude.h>

namespace SR_UTILS_NS {
    class GameObject;

    namespace LayoutUtility {
        struct Metrics {
            float_t minimum = 0.f;
            float_t preferred = 0.f;
            float_t flexible = 0.f;
        };

        SR_COMMON_DLL_API Metrics GetMetrics(const GameObject& object, bool horizontal);
        SR_COMMON_DLL_API bool IsIgnored(const GameObject& object);
    }
}

#endif // SR_ENGINE_UTILS_UI_LAYOUT_UTILITY_H
