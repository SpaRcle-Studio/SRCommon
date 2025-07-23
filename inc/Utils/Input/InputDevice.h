//
// Created by Monika on 16.06.2022.
//

#ifndef SR_ENGINE_INPUTDEVICE_H
#define SR_ENGINE_INPUTDEVICE_H

#include <Utils/Common/NonCopyable.h>
#include <Utils/Input/KeyCodes.h>
#include <Utils/Math/Vector2.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(InputDeviceType, uint8_t,
        Unknown,
        Mouse,
        Keyboard,
        Joystick
    );

    class SR_COMMON_DLL_API InputDeviceData : public NonCopyable {
    public:
        ~InputDeviceData() override = default;

    public:
        virtual InputDeviceType GetType() const = 0;

    };

    class SR_COMMON_DLL_API MouseInputData : public InputDeviceData {
    public:
        ~MouseInputData() override = default;

        MouseInputData()
            : m_code(MouseCode::None)
            , m_state(KeyState::None)
        { }

    public:
        InputDeviceType GetType() const override;

        SR_NODISCARD SR_MATH_NS::FVector2 GetDrag() const;
        SR_NODISCARD bool IsDrag() const;

    public:
        MouseCode m_code;
        KeyState m_state;
        SR_MATH_NS::FVector2 m_position;
        SR_MATH_NS::FVector2 m_drag;
        SR_MATH_NS::FVector2 m_wheel;

    };

    class SR_COMMON_DLL_API KeyboardInputData : public InputDeviceData {
    public:
        KeyboardInputData(KeyCode code, KeyState state)
            : m_code(code)
            , m_state(state)
        { }

        KeyboardInputData()
            : KeyboardInputData(KeyCode::None, KeyState::None)
        { }

    public:
        InputDeviceType GetType() const override;
        SR_NODISCARD KeyCode GetKeyCode() const;

    public:
        KeyCode m_code;
        KeyState m_state;

    };
}

#endif //SR_ENGINE_INPUTDEVICE_H
