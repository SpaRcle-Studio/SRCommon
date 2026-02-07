//
// Created by Nikita on 30.12.2020.
//

#ifndef SR_ENGINE_INPUT_SYSTEM_H
#define SR_ENGINE_INPUT_SYSTEM_H

#include <Utils/Math/Vector2.h>
#include <Utils/Math/Rect.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Common/PassKey.h>
#include <Utils/Common/SubscriptionHolder.h>

namespace SR_UTILS_NS {
    enum class CursorLockMode : uint8_t {
        Everywhere, Editor, PlayMode
    };

    struct CursorLockInfo {
        std::optional<SR_MATH_NS::FRect> lockRect;
        CursorLockMode lockMode = CursorLockMode::Everywhere;
        uint64_t id = 0;
    };

    struct InputTextEvent {
        char text[256];
        uint8_t length = 0;
        void* pSource = nullptr;

        SR_NODISCARD std::string_view GetText() const;
        void SetText(std::string_view text);
    };

    static const StringAtom INPUT_TEXT_EVENT_DATA_ID = "InputTextEventData";
    static const StringAtom INPUT_TEXT_EVENT_ID = "InputTextEvent";

    class Input : public Singleton<Input>, public SubscriptionHolder {
        SR_REGISTER_SINGLETON(Input)
        enum class State {
            UnPressed, Down, Pressed, Up
        };
    protected:
        ~Input() override;

    public:
        void SetMouseScroll(double_t xOffset, double_t yOffset);
        void SetFocusedWindowRect(const std::optional<SR_MATH_NS::FRect>& rect);

        void Update();
        void Reload();
        void ResetMouse();

        SR_NODISCARD SR_MATH_NS::FVector2 GetMouseDrag();
        SR_NODISCARD SR_MATH_NS::FVector2 GetMousePos() const;
        SR_NODISCARD bool IsPlayMode() const;
        SR_NODISCARD bool IsAppFocused() const;
        SR_NODISCARD int32_t GetMouseWheel() const;

        bool GetMouseDown(MouseCode code);
        bool GetMouseUp(MouseCode code);
        bool GetMouse(MouseCode code);

        bool GetKeyDown(KeyCode key);
        bool GetKeyUp(KeyCode key);
        bool GetKey(KeyCode key);

        void LockCursor(CursorLockInfo& info);
        void UnlockCursor(const CursorLockInfo& info);

        void SetPlayMode(bool isPlayMode);
        void AddTextEvent(InputTextEvent&& event);

    private:
        void SetCursorVisible(bool isVisible);
        void Reset();
        void SetState(uint16_t keyIndex, State state);
        void UpdateMouse();
        void UpdateKeyboard();

        SR_NODISCARD const CursorLockInfo* GetActiveLock() const;

    private:
        std::array<std::vector<CursorLockInfo>, 3> m_cursorLocks;
        std::vector<InputTextEvent> m_textEvents;

        SR_MATH_NS::FVector2 m_mouseDrag;
        SR_MATH_NS::FVector2 m_mouse;
        SR_MATH_NS::FVector2 m_mousePrev;
        SR_MATH_NS::FVector2 m_mouseScroll;
        SR_MATH_NS::FVector2 m_mouseScrollCurrent;

        std::optional<SR_MATH_NS::FRect> m_focusedWindowRect;

        std::atomic<bool> m_init = false;
        std::atomic<bool> m_isLocked = false;
        std::atomic<bool> m_isVisible = true;
        std::atomic<bool> m_isPlayMode = false;

        State m_keys[256] = { };

    };

    class CursorLock : public NonCopyable {
        using Super = NonCopyable;
    public:
        explicit CursorLock(CursorLockMode lockMode, std::optional<SR_MATH_NS::FRect> lockRect = std::nullopt);
        ~CursorLock() override;

        CursorLock(CursorLock&& other) noexcept;
        CursorLock& operator=(CursorLock&& other) noexcept;

    private:
        CursorLockInfo m_info;
        bool m_isLock = false;

    };
}

#endif //SR_ENGINE_INPUT_SYSTEM_H
