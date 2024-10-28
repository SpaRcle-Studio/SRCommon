//
// Created by Nikita on 30.12.2020.
//

#include <Utils/Input/InputSystem.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Common/StringAtomLiterals.h>

namespace SR_UTILS_NS {
    void Input::Check() {
        SR_TRACY_ZONE;

        if (!m_init) {
            Reset();
            m_init = true;
        }

        m_mouseScroll = m_mouseScrollCurrent;
        m_mouseScrollCurrent = SR_MATH_NS::FVector2(0, 0);

        m_mousePrev = m_mouse;

        if (!m_arr) {
            m_arr = new uint8_t[256];
            memset(m_arr, 0, 256);
        }

        if (!SR_PLATFORM_NS::GetSystemKeyboardState(m_arr)) {
            return;
        }

        auto&& mouseState = SR_PLATFORM_NS::GetMouseState();

        m_mouse = mouseState.position;
        m_mouseDrag = m_mouse - m_mousePrev;

        if (m_counterLock >= 1 && m_lockCursorCallback) {
            if(m_isVisible) {
                SetCursorVisible(false);
            }
            m_lockCursorCallback();
        }
        else if(!m_isVisible) {
            SetCursorVisible(true);
        }

    #if defined(SR_WIN32)
        for (uint16_t i = 5; i < 256; ++i) {
            if (m_arr[i] >> 7 != 0) {
                switch (m_keys[i]) {
                case State::UnPressed:
                case State::Up:
                    SetState(i, State::Down);
                    break;
                case State::Down:
                    SetState(i, State::Pressed);
                    break;
                case State::Pressed:
                    /// skip
                    break;
                }
            }
            else {
                switch (m_keys[i]) {
                case State::UnPressed:
                    /// skip
                    break;
                case State::Down:
                case State::Pressed:
                    SetState(i, State::Up);
                    break;
                case State::Up:
                    SetState(i, State::UnPressed);
                    break;
                }
            }
        }
    #elif defined(SR_LINUX)
        for (uint16_t i = 5; i < 256; ++i) {
            if (m_arr[i] == 0 && (m_keys[i] == State::Down || m_keys[i] == State::Pressed)) {
                /// If a key was already Pressed or Down and now is not pressed, then it's Up
                SetState(i, State::Up);
            }
            else {
                /// Otherwise, set the key state to the current state
                SetState(i, static_cast<State>(m_arr[i]));
            }
        }

        memset(m_arr, 0, 256);
    #endif

        for (uint8_t i = 0; i < 5; ++i) {
            if (mouseState.buttonStates[i]) {
                switch (m_keys[i]) {
                    case State::UnPressed: SetState(i, State::Down); break;
                    case State::Down: SetState(i, State::Pressed); break;
                    case State::Pressed: break; /// skip
                    case State::Up: SetState(i, State::Down); break;
                }
            }
            else {
                switch (m_keys[i]) {
                    case State::UnPressed: break; /// skip
                    case State::Down:
                    case State::Pressed:
                        SetState(i, State::Up);
                        break;
                    case State::Up:
                        SetState(i, State::UnPressed);
                        break;
                }
            }
        }
    }

    bool Input::GetKeyDown(KeyCode key) {
        return m_keys[(int)key] == State::Down;
    }

    bool Input::GetKeyUp(KeyCode key) {
        return m_keys[(int)key] == State::Up;
    }

    bool Input::GetKey(KeyCode key) {
        return (m_keys[(int)key] == State::Pressed || m_keys[(int)key] == State::Down);
    }

    SR_MATH_NS::FVector2 Input::GetMouseDrag() {
        return m_mouseDrag;
    }

    int32_t Input::GetMouseWheel() {
        return m_mouseScroll.y;
    }

    void Input::Reset() {
        for (auto& key : m_keys) {
            key = State::UnPressed;
        }

        ResetMouse();
    }

    void Input::SetState(uint16_t keyIndex, State state) {
        if (m_keys[keyIndex] == state) {
            return;
        }

        m_keys[keyIndex] = state;

        if (state == State::Down && HasSubscriptions()) {
            SubscriptionMessage msg;
            msg.SetInt("KeyCode"_atom, keyIndex);
            Broadcast("Down"_atom, msg);
        }
        else if (state == State::Up && HasSubscriptions()) {
            SubscriptionMessage msg;
            msg.SetInt("KeyCode"_atom, keyIndex);
            Broadcast("Up"_atom, msg);
        }
    }

    void Input::ResetMouse() {
        m_mousePrev = m_mouse = SR_UTILS_NS::Platform::GetMousePos();
        m_mouseScroll = m_mouseScrollCurrent = SR_MATH_NS::FVector2(0.f);
    }

    void Input::Reload() {
        m_init = false;
        Reset();
    }

    bool Input::IsMouseMoved() const {
        return GetMousePos() != GetPrevMousePos();
    }

    void Input::SetCursorVisible(bool isVisible) {
        if (isVisible != m_isVisible) {
            m_isVisible = isVisible;
            SR_PLATFORM_NS::SetCursorVisible(isVisible);
        }
    }

    void Input::SetCursorLockCallback(CursorLockCallback&& callback) {
        m_lockCursorCallback = std::move(callback);
    }

    void Input::LockCursor(){
        ++m_counterLock;
    }

    void Input::UnlockCursor(){
        --m_counterLock;
    }
}
