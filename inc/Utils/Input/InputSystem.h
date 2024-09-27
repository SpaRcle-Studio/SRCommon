//
// Created by Nikita on 30.12.2020.
//

#ifndef SR_ENGINE_INPUTSYSTEM_H
#define SR_ENGINE_INPUTSYSTEM_H

#include <Utils/Math/Vector2.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Common/Singleton.h>

namespace SR_UTILS_NS {
    class SR_DLL_EXPORT Input : public Singleton<Input> {
        SR_REGISTER_SINGLETON(Input)

        enum class State {
            UnPressed, Down, Pressed, Up
        };
        using CursorLockCallback = SR_HTYPES_NS::Function<void()>;

    protected:
        ~Input() override = default;

    public:
        void SetMouseScroll(double_t xOffset, double_t yOffset){
            m_mouseScrollCurrent = { (float_t)xOffset, (float_t)yOffset };
        }

        void Check();
        void Reload();
        void ResetMouse();

        SR_NODISCARD SR_MATH_NS::FVector2 GetMouseDrag();
        SR_NODISCARD SR_MATH_NS::FVector2 GetMousePos() const { return m_mouse; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetPrevMousePos() const { return m_mousePrev; }
        SR_NODISCARD bool IsMouseMoved() const;

        int32_t GetMouseWheel();

        bool GetMouseDown(MouseCode code) { return GetKeyDown(static_cast<KeyCode>(code)); }
        bool GetMouseUp(MouseCode code) { return GetKeyUp(static_cast<KeyCode>(code)); }
        bool GetMouse(MouseCode code) { return GetKey(static_cast<KeyCode>(code)); }

        bool GetKeyDown(KeyCode key);
        bool GetKeyUp(KeyCode key);
        bool GetKey(KeyCode key);

        void SetCursorVisible(bool isVisible);
        void SetCursorLockCallback(CursorLockCallback&& callback);

        void LockCursor();
        void UnlockCursor();

    private:
        void Reset();

    private:
        uint32_t m_counterLock = 0;
        CursorLockCallback m_lockCursorCallback;

        SR_MATH_NS::FVector2 m_mouseDrag;
        SR_MATH_NS::FVector2 m_mousePrev;
        SR_MATH_NS::FVector2 m_mouse;
        SR_MATH_NS::FVector2 m_mouseScroll;
        SR_MATH_NS::FVector2 m_mouseScrollCurrent;

        std::atomic<bool> m_init = false;
        std::atomic<bool> m_isVisible = true;

        State m_keys[256] = { };
        uint8_t* m_arr = nullptr;
    };

    class CursorLock : public NonCopyable {
    public:
        CursorLock() {
            m_isLock  = true;
            Input::Instance().LockCursor();
        };

        ~CursorLock() {
            if (m_isLock) {
                m_isLock = false;
                Input::Instance().UnlockCursor();
            } };

        CursorLock(CursorLock&& ref) noexcept {
            m_isLock = SR_UTILS_NS::Exchange(ref.m_isLock, {});
        }

        CursorLock& operator=(CursorLock&& other) noexcept {
            if (this != &other){
                m_isLock = SR_UTILS_NS::Exchange(other.m_isLock, { });
            }
            return *this;
        }

    private:
        bool m_isLock = false;
    };

}

#endif //SR_ENGINE_INPUTSYSTEM_H
