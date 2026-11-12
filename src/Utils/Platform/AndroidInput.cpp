//
// Created by OpenAI Codex on 12.11.2026.
//

#ifdef SR_ANDROID
#include <Utils/Platform/AndroidInput.h>
#include <Utils/Input/InputSystem.h>
#include <Utils/Input/KeyCodes.h>

namespace SR_UTILS_NS::Platform {
    int32_t AndroidKeyToEngine(int32_t key);

    namespace {
        std::mutex g_inputMutex;
        MouseState g_mouse;
        MouseState g_frameMouse;
        KeyboardState g_keyboard;
        KeyboardState g_frameKeyboard;
        bool g_mousePressed[5] = {};
        bool g_keyPressed[256] = {};
        bool g_nativeKeys[512] = {};
        uint16_t g_keyHolds[256] = {};
        SR_MATH_NS::FVector2 g_pressPosition;
        SR_MATH_NS::FVector2 g_scroll;
        Vector<AndroidTouchPoint> g_touches;
        int32_t g_primaryTouch = -1;

        void SetMouseButton(uint32_t index, bool pressed) {
            if (pressed && !g_mouse.buttonStates[index]) {
                g_mousePressed[index] = true;
                if (index == 0) {
                    g_pressPosition = g_mouse.position;
                }
            }
            g_mouse.buttonStates[index] = pressed;
        }
    }

    void ResetAndroidInput() {
        std::lock_guard lock(g_inputMutex);
        g_mouse = {};
        g_frameMouse = {};
        g_keyboard = {};
        g_frameKeyboard = {};
        for (auto& pressed : g_mousePressed) {
            pressed = false;
        }
        for (auto& pressed : g_keyPressed) {
            pressed = false;
        }
        for (auto& pressed : g_nativeKeys) {
            pressed = false;
        }
        for (auto& count : g_keyHolds) {
            count = 0;
        }
        g_touches.clear();
        g_primaryTouch = -1;
        g_scroll = {};
    }

    void ApplyAndroidInputEvent(const AndroidEvent& event) {
        if (event.type == AndroidEvent::InputReset) {
            ResetAndroidInput();
            return;
        }
        std::lock_guard lock(g_inputMutex);
        if (event.type == AndroidEvent::Key) {
            const int32_t native = event.key.keyCode;
            const int32_t key = AndroidKeyToEngine(native);
            if (key < 0 || key >= 256 || native < 0 || native >= 512) {
                return;
            }
            const bool pressed = event.key.action == AKEY_EVENT_ACTION_DOWN;
            if (g_nativeKeys[native] != pressed) {
                g_nativeKeys[native] = pressed;
                if (pressed) {
                    ++g_keyHolds[key];
                    g_keyPressed[key] = true;
                }
                else if (g_keyHolds[key] > 0) {
                    --g_keyHolds[key];
                }
                g_keyboard.keyStates[key] = g_keyHolds[key] != 0;
            }
            return;
        }
        if (event.type != AndroidEvent::Motion) {
            return;
        }
        const auto& motion = event.motion;
        const SR_MATH_NS::FVector2 position(motion.x, motion.y);
        if ((motion.source & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE) {
            g_mouse.position = position;
            const int32_t buttons[5] = {AMOTION_EVENT_BUTTON_PRIMARY, AMOTION_EVENT_BUTTON_SECONDARY,
                AMOTION_EVENT_BUTTON_TERTIARY, AMOTION_EVENT_BUTTON_BACK, AMOTION_EVENT_BUTTON_FORWARD};
            for (uint32_t i = 0; i < 5; ++i) {
                SetMouseButton(i, (motion.buttons & buttons[i]) != 0);
            }
            if (motion.action == AMOTION_EVENT_ACTION_SCROLL) {
                g_scroll += SR_MATH_NS::FVector2(motion.scrollX, motion.scrollY);
            }
            return;
        }
        const bool down = motion.action == AMOTION_EVENT_ACTION_DOWN || motion.action == AMOTION_EVENT_ACTION_POINTER_DOWN;
        const bool up = motion.action == AMOTION_EVENT_ACTION_UP || motion.action == AMOTION_EVENT_ACTION_POINTER_UP;
        if (!down && !up && motion.action != AMOTION_EVENT_ACTION_MOVE) {
            return;
        }
        auto point = g_touches.end();
        for (auto current = g_touches.begin(); current != g_touches.end(); ++current) {
            if (current->id == motion.pointerId) {
                point = current;
                break;
            }
        }
        if (down && point == g_touches.end()) {
            if (g_touches.size() >= 32) {
                return;
            }
            if (g_touches.empty()) {
                g_primaryTouch = motion.pointerId;
            }
            point = &g_touches.emplace_back();
            point->id = motion.pointerId;
        }
        if (point == g_touches.end()) {
            return;
        }
        point->position = position;
        point->pressure = motion.pressure;
        if (motion.pointerId == g_primaryTouch) {
            g_mouse.position = position;
            if (down || up) {
                SetMouseButton(0, down);
            }
            if (up) {
                g_primaryTouch = -1;
            }
        }
        if (up) {
            g_touches.erase(point);
        }
    }

    void BeginAndroidInputFrame() {
        SR_MATH_NS::FVector2 scroll;
        {
            std::lock_guard lock(g_inputMutex);
            g_frameMouse = g_mouse;
            g_frameKeyboard = g_keyboard;
            for (uint32_t i = 0; i < 5; ++i) {
                if (g_mousePressed[i]) {
                    g_frameMouse.buttonStates[i] = true;
                    if (i == 0 && !g_mouse.buttonStates[0]) {
                        g_frameMouse.position = g_pressPosition;
                    }
                }
                g_mousePressed[i] = false;
            }
            for (uint32_t i = 0; i < 256; ++i) {
                g_frameKeyboard.keyStates[i] |= g_keyPressed[i];
                g_keyPressed[i] = false;
            }
            scroll = g_scroll;
            g_scroll = {};
        }
        if (scroll.x != 0.f || scroll.y != 0.f) {
            Input::Instance().SetMouseScroll(scroll.x, scroll.y);
        }
    }

    MouseState GetAndroidMouseState() {
        std::lock_guard lock(g_inputMutex);
        return g_frameMouse;
    }

    KeyboardState GetAndroidKeyboardState() {
        std::lock_guard lock(g_inputMutex);
        return g_frameKeyboard;
    }

    Vector<AndroidTouchPoint> GetAndroidTouchPoints() {
        std::lock_guard lock(g_inputMutex);
        return g_touches;
    }
}
#endif
