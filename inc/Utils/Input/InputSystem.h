//
// Created by Nikita on 30.12.2020.
//

#ifndef SR_ENGINE_INPUT_SYSTEM_H
#define SR_ENGINE_INPUT_SYSTEM_H

#include <Utils/Common/PassKey.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Common/SubscriptionHolder.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Math/Rect.h>
#include <Utils/Math/Vector2.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(CursorLockMode, uint8_t, Everywhere, Editor, PlayMode);

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

    /// @noCopyable @noMovable
    class Input : public Singleton<Input>, public SubscriptionHolder, public SRClass {
        SR_CLASS()
        SR_REGISTER_SINGLETON(Input)
        enum class State { UnPressed, Down, Pressed, Up };
    public:
        void SetMouseScroll(double_t xOffset, double_t yOffset);
        void SetFocusedWindowRect(const std::optional<SR_MATH_NS::FRect>& rect);

        void Update(float_t dt);
        void Reload();
        void ResetMouse();
        void SetMouseDrag(const SR_MATH_NS::FVector2& drag);

        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector2 GetMouseDrag();
        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector2 GetMousePos() const;
        /// @method @evaluate
        SR_NODISCARD bool IsPlayMode() const;
        /// @method @evaluate
        SR_NODISCARD bool IsAppFocused() const;
        /// @method @evaluate
        SR_NODISCARD int32_t GetMouseWheel() const;
        /// @method @evaluate
        SR_NODISCARD SR_MATH_NS::FVector2 GetMouseScroll() const { return m_mouseScroll; }
        /// @method @evaluate
        SR_NODISCARD bool IsCursorLocked() const;

        /// @method @evaluate
        bool GetMouseDown(MouseCode code);
        /// @method @evaluate
        bool GetMouseUp(MouseCode code);
        /// @method @evaluate
        bool GetMouse(MouseCode code);

        /// @method @evaluate
        bool GetKeyDown(KeyCode key);
        /// @method @evaluate
        bool GetKeyUp(KeyCode key);
        /// @method @evaluate
        bool GetKey(KeyCode key);

        void LockCursor(CursorLockInfo& info);
        void UnlockCursor(const CursorLockInfo& info);

        void SetPlayMode(bool isPlayMode);
        void AddTextEvent(InputTextEvent&& event);

    private:
        void SetCursorVisible(bool isVisible);
        void Reset();
        void SetState(uint16_t keyIndex, State state);
        void UpdateMouse(float_t dt);
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

        State m_keys[256] = {};
    };

    class CursorLock : public Serializable {
        using Super = Serializable;
        SR_CLASS()
    public:
        CursorLock() = default;
        explicit CursorLock(CursorLockMode lockMode, std::optional<SR_MATH_NS::FRect> lockRect = std::nullopt);
        ~CursorLock() override;

        CursorLock(CursorLock&& other) noexcept;
        CursorLock(const CursorLock& other) noexcept;
        CursorLock& operator=(CursorLock&& other) noexcept;
        CursorLock& operator=(const CursorLock& other) noexcept;

        /// @method
        void SetLockMode(CursorLockMode lockMode);
        /// @method
        void Lock();
        /// @method
        void Unlock();

    private:
        CursorLockInfo m_info;
        bool m_isLock = false;
    };

    class InputAccumulator {
    public:
        InputAccumulator() = default;
        ~InputAccumulator() = default;

    public:
        void Accumulate();
        void Apply(uint32_t frames);
        void Reset();

    private:
        SR_MATH_NS::FVector2 m_mouseDragAccumulated;
        SR_MATH_NS::FVector2 m_mouseDragOriginal;

    };
} // namespace SR_UTILS_NS

#endif // SR_ENGINE_INPUT_SYSTEM_H
