//
// Created by Codex on 15.11.2026.
//

#ifndef SR_ENGINE_UTILS_I_UI_INPUT_TARGET_H
#define SR_ENGINE_UTILS_I_UI_INPUT_TARGET_H

#include <Utils/Input/PointerInput.h>
#include <Utils/Math/Vector2.h>

namespace SR_UTILS_NS {
    class IUIInputTarget {
    public:
        virtual ~IUIInputTarget() = default;
        virtual bool HitTest(const PointerEvent& event) = 0;
        virtual bool IsInteractable() const = 0;
        virtual bool IsFocusable() const { return true; }
        virtual bool IsModal() const { return false; }
        virtual bool CanDrag() const { return false; }
        virtual bool AcceptsButton(PointerButton button) const { return button == PointerButton::Primary; }
        virtual void SetInputState(bool hovered, bool pressed, bool focused) = 0;
        virtual void PointerDown(const PointerEvent& event) { }
        virtual void PointerDrag(const PointerEvent& event) { }
        virtual void PointerUp(const PointerEvent& event, bool click) { }
        virtual void PointerCancel(const PointerEvent& event) {
            PointerUp(event, false);
        }
        virtual void Submit() { }
        virtual void Cancel() { }
        virtual bool Navigate(int32_t direction) { return false; }
        virtual bool Scroll(const SR_MATH_NS::FVector2& delta) { return false; }
        virtual int32_t GetInputPriority() const { return 0; }
        // Custom targets keep polling until all input-affecting changes notify the router.
        virtual bool RequiresInputPolling() const { return true; }
    };
}

#endif // SR_ENGINE_UTILS_I_UI_INPUT_TARGET_H
