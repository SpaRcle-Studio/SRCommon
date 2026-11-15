//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_UI_INPUT_ADAPTER_H
#define SR_ENGINE_UTILS_UI_INPUT_ADAPTER_H

#include <Utils/Input/PointerInput.h>

namespace SR_UTILS_NS {
    struct UINavigationState {
        bool focused = true;
        bool nextFocus = false;
        bool previousFocus = false;
        bool submit = false;
        bool cancel = false;
        int32_t direction = 0;
    };

    class SR_COMMON_DLL_API UIInputAdapter {
    public:
        static UINavigationState ReadKeyboard();
    };
}

#endif // SR_ENGINE_UTILS_UI_INPUT_ADAPTER_H
