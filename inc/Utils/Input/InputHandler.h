//
// Created by Monika on 17.02.2022.
//

#ifndef SR_ENGINE_INPUTHANDLER_H
#define SR_ENGINE_INPUTHANDLER_H

#include <Utils/Input/InputEvents.h>
#include <Utils/Events/Event.h>
#include <Utils/Input/InputSystem.h>
#include <Utils/Input/InputDevice.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API InputHandler : public Event<InputDeviceData*> {
    protected:
        InputHandler();
        ~InputHandler() override;

    public:
        virtual void OnMouseMove(const MouseInputData* data);
        virtual void OnMousePress(const MouseInputData* data);
        virtual void OnMouseDown(const MouseInputData* data);
        virtual void OnMouseUp(const MouseInputData* data);

        virtual void OnKeyPress(const KeyboardInputData* data);
        virtual void OnKeyDown(const KeyboardInputData* data);
        virtual void OnKeyUp(const KeyboardInputData* data);

    protected:
        bool IsKeyPressed(KeyCode code);

    private:
        void Trigger(InputDeviceData* inputDeviceData) override;

        void TriggerKeyboard(KeyboardInputData* keyboardInputData);
        void TriggerMouse(MouseInputData* mouseInputData);

    };
}

#endif //SR_ENGINE_INPUTHANDLER_H
