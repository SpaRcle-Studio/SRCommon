//
// Created by Codex on 15.11.2026.
//

#include <Utils/UI/UIInputAdapter.h>
#include <Utils/Input/InputSystem.h>

namespace SR_UTILS_NS {
    UINavigationState UIInputAdapter::ReadKeyboard() {
        auto&& input = Input::Instance();
        UINavigationState result;
        result.focused = input.IsAppFocused();
        const bool tab = input.GetKeyDown(KeyCode::Tab);
        const bool shift = input.GetKey(KeyCode::LShift) || input.GetKey(KeyCode::RShift);
        result.nextFocus = tab && !shift;
        result.previousFocus = tab && shift;
        result.submit = input.GetKeyDown(KeyCode::Enter) || input.GetKeyDown(KeyCode::Space);
        result.cancel = input.GetKeyDown(KeyCode::Escape);
        if (input.GetKeyDown(KeyCode::LeftArrow) || input.GetKeyDown(KeyCode::UpArrow)) {
            result.direction = -1;
        }
        else if (input.GetKeyDown(KeyCode::RightArrow) || input.GetKeyDown(KeyCode::DownArrow)) {
            result.direction = 1;
        }
        return result;
    }
}
