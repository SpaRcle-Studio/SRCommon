//
// Created by Monika on 16.06.2022.
//

#include <Utils/Input/InputHandler.h>

namespace SR_UTILS_NS {
    void InputHandler::Trigger(InputDeviceData *inputDeviceData) {
        switch (inputDeviceData->GetType()) {
            case InputDeviceType::Keyboard:
                TriggerKeyboard(dynamic_cast<KeyboardInputData*>(inputDeviceData));
                break;
            case InputDeviceType::Mouse:
                TriggerMouse(dynamic_cast<MouseInputData*>(inputDeviceData));
                break;
            case InputDeviceType::Joystick:
            case InputDeviceType::Unknown:
            default:
                break;
        }
    }

    void InputHandler::TriggerKeyboard(KeyboardInputData *keyboardInputData) {
        if (!keyboardInputData) {
            return;
        }

        switch (keyboardInputData->m_state) {
            case KeyState::Down:
                OnKeyDown(keyboardInputData);
                break;
            case KeyState::Up:
                OnKeyUp(keyboardInputData);
                break;
            case KeyState::Press:
                OnKeyPress(keyboardInputData);
                break;
            case KeyState::None:
            default:
                break;
        }
    }

    void InputHandler::TriggerMouse(MouseInputData *mouseInputData) {
        if (!mouseInputData) {
            return;
        }

        if (mouseInputData->IsDrag()) {
            OnMouseMove(mouseInputData);
        }

        switch (mouseInputData->m_state) {
            case KeyState::Down:
                OnMouseDown(mouseInputData);
                break;
            case KeyState::Up:
                OnMouseUp(mouseInputData);
                break;
            case KeyState::Press:
                OnMousePress(mouseInputData);
                break;
            case KeyState::None:
            default:
                break;
        }
    }

    bool InputHandler::IsKeyPressed(KeyCode code) {
        return Input::Instance().GetKey(code);
    }

    void InputHandler::OnMouseMove(const MouseInputData* /*data*/) {

    }

    void InputHandler::OnMousePress(const MouseInputData* /*data*/) {

    }

    void InputHandler::OnMouseUp(const MouseInputData* /*data*/) {

    }

    void InputHandler::OnMouseDown(const MouseInputData* /*data*/) {

    }

    void InputHandler::OnKeyPress(const KeyboardInputData* /*data*/) {

    }

    void InputHandler::OnKeyDown(const KeyboardInputData* /*data*/) {

    }

    void InputHandler::OnKeyUp(const KeyboardInputData* /*data*/) {

    }

    InputHandler::InputHandler()
        : Event(typeid(InputHandler).name())
    { }

    InputHandler::~InputHandler() = default;
}