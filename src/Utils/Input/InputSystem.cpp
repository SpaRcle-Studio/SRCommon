//
// Created by Nikita on 30.12.2020.
//

#include <Utils/Input/InputSystem.h>
#include <Utils/Common/Numeric.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Common/StringAtomLiterals.h>
#include <Utils/Common/SubscriptionMessage.h>
#include <Utils/Types/Thread.h>

namespace SR_UTILS_NS {
    std::string_view InputTextEvent::GetText() const {
        return std::string_view(text, length);
    }

    void InputTextEvent::SetText(std::string_view text) {
        if (text.size() > sizeof(this->text) - 1) {
            SRHalt("InputTextEvent::SetText() : text size exceeds maximum length!");
            return;
        }
        std::memcpy(this->text, text.data(), text.size());
        this->text[text.size()] = '\0'; // null-terminate the string
        length = static_cast<uint8_t>(text.size());
    }

    Input::~Input() = default;

    void Input::UpdateMouse() {
        auto&& mouseState = SR_PLATFORM_NS::GetMouseState();

        m_mouseScroll = m_mouseScrollCurrent;
        m_mouseScrollCurrent = SR_MATH_NS::FVector2(0, 0);

        auto&& pActiveLock = GetActiveLock();
        const bool isLocked = IsAppFocused() && pActiveLock;

        if (isLocked) {
            m_mousePrev = pActiveLock->lockRect ? pActiveLock->lockRect->Center() : m_focusedWindowRect->Center();
            m_mousePrev = m_mousePrev.Round();

            if (!m_isLocked) {
                m_mouse = m_mousePrev;
                m_isLocked = true;
            }
            else {
                m_mouse = mouseState.position;
            }

            SetCursorVisible(false);
            SR_PLATFORM_NS::SetMousePos(m_mousePrev.CastToInt());
        }
        else {
            m_mousePrev = m_mouse;
            m_mouse = mouseState.position;
            m_isLocked = false;
            SetCursorVisible(true);
        }

        m_mouseDrag = m_mouse - m_mousePrev;

        for (uint8_t i = 0; i < 5; ++i) {
            if (IsAppFocused() && mouseState.buttonStates[i]) {
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

    void Input::UpdateKeyboard() {
        if (!IsAppFocused()) {
            return;
        }

        const SR_PLATFORM_NS::KeyboardState keyboardState = SR_PLATFORM_NS::GetSystemKeyboardState();

        for (uint16_t i = 5; i < 256; ++i) {
            if (keyboardState.keyStates[i]) {
                switch (m_keys[i]) {
                    case State::UnPressed:
                    case State::Up:
                        SetState(i, State::Down);
                        break;
                    case State::Down:
                        SetState(i, State::Pressed);
                        break;
                    case State::Pressed: break;
                }
            }
            else {
                switch (m_keys[i]) {
                    case State::UnPressed:
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
    }

    void Input::Update() {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        if (!m_init) {
            Reset();
            m_init = true;
        }

        for (auto& event : m_textEvents) {
            SubscriptionMessage msg;
            msg.SetAny(INPUT_TEXT_EVENT_DATA_ID, event);
            Broadcast(INPUT_TEXT_EVENT_ID, msg);
        }
        m_textEvents.clear();

        UpdateMouse();
        UpdateKeyboard();
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

    int32_t Input::GetMouseWheel() const {
        return m_mouseScroll.y;
    }

    bool Input::GetMouseDown(MouseCode code) { return GetKeyDown(static_cast<KeyCode>(code)); }
    bool Input::GetMouseUp(MouseCode code) { return GetKeyUp(static_cast<KeyCode>(code)); }
    bool Input::GetMouse(MouseCode code) { return GetKey(static_cast<KeyCode>(code)); }

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
        m_mouse = m_mousePrev = SR_PLATFORM_NS::GetMousePos();
        m_mouseScroll = m_mouseScrollCurrent = SR_MATH_NS::FVector2(0.f);
    }

    void Input::Reload() {
        m_init = false;
        Reset();
    }

    void Input::SetCursorVisible(bool isVisible) {
        if (isVisible != m_isVisible) {
            m_isVisible = isVisible;
            SR_PLATFORM_NS::SetCursorVisible(isVisible);
        }
    }

    void Input::LockCursor(CursorLockInfo& info) {
        SR_LOCK_GUARD;

    retry:
        info.id = SR_UTILS_NS::Random::Instance().UInt64();
        for (const auto& lock : m_cursorLocks[static_cast<uint32_t>(info.lockMode)]) {
            if (lock.id == info.id) {
                goto retry;
            }
        }

        m_cursorLocks[static_cast<uint32_t>(info.lockMode)].emplace_back(info);
    }

    void Input::UnlockCursor(const CursorLockInfo& info) {
        SR_LOCK_GUARD;
        auto&& locks = m_cursorLocks[static_cast<uint32_t>(info.lockMode)];
        auto&& pIt = std::find_if(locks.begin(), locks.end(), [&info](const CursorLockInfo& lock) { return lock.id == info.id; });

        if (pIt != locks.end()) {
            locks.erase(pIt);
        }
        else {
            SRHalt("Failed to unlock cursor! Lock not found!");
        }
    }

    SR_MATH_NS::FVector2 Input::GetMousePos() const {
        return m_mouse;
    }

    void Input::SetMouseScroll(double_t xOffset, double_t yOffset) {
        m_mouseScrollCurrent = { (float_t)xOffset, (float_t)yOffset };
    }

    void Input::SetFocusedWindowRect(const std::optional<SR_MATH_NS::FRect>& rect) {
        m_focusedWindowRect = rect;
    }

    void Input::SetPlayMode(bool isPlayMode) {
        m_isPlayMode = isPlayMode;
    }

    void Input::AddTextEvent(InputTextEvent&& event) {
        SR_TRACY_ZONE;
        constexpr size_t maxTextEvents = 16;
        if (m_textEvents.size() >= maxTextEvents) {
            m_textEvents.erase(m_textEvents.begin());
        }
        m_textEvents.emplace_back(std::move(event));
    }

    bool Input::IsPlayMode() const {
        return m_isPlayMode;
    }

    bool Input::IsAppFocused() const {
        return m_focusedWindowRect.has_value();
    }

    const CursorLockInfo* Input::GetActiveLock() const {
    #if defined(SR_LINUX)
        return nullptr;
    #endif

        if (!m_cursorLocks[static_cast<uint32_t>(CursorLockMode::Everywhere)].empty()) {
            return &m_cursorLocks[static_cast<uint32_t>(CursorLockMode::Everywhere)].back();
        }

        if (!m_cursorLocks[static_cast<uint32_t>(CursorLockMode::Editor)].empty() && !m_isPlayMode) {
            return &m_cursorLocks[static_cast<uint32_t>(CursorLockMode::Editor)].back();
        }

        if (!m_cursorLocks[static_cast<uint32_t>(CursorLockMode::PlayMode)].empty() && m_isPlayMode) {
            return &m_cursorLocks[static_cast<uint32_t>(CursorLockMode::PlayMode)].back();
        }

        return nullptr;
    }

    CursorLock::CursorLock(CursorLockMode lockMode, std::optional<SR_MATH_NS::FRect> lockRect)
        : Super()
        , m_isLock(true)
    {
        m_info.lockMode = lockMode;
        m_info.lockRect = lockRect;
        Input::Instance().LockCursor(m_info);
    }

    CursorLock::~CursorLock() {
        if (m_isLock) {
            m_isLock = false;
            Input::Instance().UnlockCursor(m_info);
        }
    };

    CursorLock::CursorLock(CursorLock&& other) noexcept {
        std::swap(m_isLock, other.m_isLock);
        std::swap(m_info, other.m_info);
    }

    CursorLock& CursorLock::operator=(CursorLock&& other) noexcept {
        if (this != &other) {
            std::swap(m_isLock, other.m_isLock);
            std::swap(m_info, other.m_info);
        }
        return *this;
    }
}
