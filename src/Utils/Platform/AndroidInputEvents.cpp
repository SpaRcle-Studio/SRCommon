//
// Created by OpenAI Codex on 12.11.2026.
//

#ifdef SR_ANDROID
#include <Utils/Platform/AndroidInput.h>
#include <Utils/Platform/AndroidLifecycle.h>
#include <Utils/Input/KeyCodes.h>

namespace SR_UTILS_NS::Platform {
    int32_t AndroidKeyToEngine(int32_t key) {
        if (key >= AKEYCODE_A && key <= AKEYCODE_Z) {
            return static_cast<int32_t>(KeyCode::A) + key - AKEYCODE_A;
        }
        if (key >= AKEYCODE_0 && key <= AKEYCODE_9) {
            return static_cast<int32_t>(KeyCode::_0) + key - AKEYCODE_0;
        }
        if (key >= AKEYCODE_F1 && key <= AKEYCODE_F12) {
            return static_cast<int32_t>(KeyCode::F1) + key - AKEYCODE_F1;
        }
        if (key >= AKEYCODE_NUMPAD_0 && key <= AKEYCODE_NUMPAD_9) {
            return static_cast<int32_t>(KeyCode::_0) + key - AKEYCODE_NUMPAD_0;
        }
        switch (key) {
            case AKEYCODE_DEL: return static_cast<int32_t>(KeyCode::BackSpace);
            case AKEYCODE_FORWARD_DEL: return static_cast<int32_t>(KeyCode::Delete);
            case AKEYCODE_TAB: return static_cast<int32_t>(KeyCode::Tab);
            case AKEYCODE_ENTER:
            case AKEYCODE_NUMPAD_ENTER: return static_cast<int32_t>(KeyCode::Enter);
            case AKEYCODE_SPACE: return static_cast<int32_t>(KeyCode::Space);
            case AKEYCODE_ESCAPE: return static_cast<int32_t>(KeyCode::Escape);
            case AKEYCODE_DPAD_LEFT: return static_cast<int32_t>(KeyCode::LeftArrow);
            case AKEYCODE_DPAD_RIGHT: return static_cast<int32_t>(KeyCode::RightArrow);
            case AKEYCODE_DPAD_UP: return static_cast<int32_t>(KeyCode::UpArrow);
            case AKEYCODE_DPAD_DOWN: return static_cast<int32_t>(KeyCode::DownArrow);
            case AKEYCODE_SHIFT_LEFT:
            case AKEYCODE_SHIFT_RIGHT: return static_cast<int32_t>(KeyCode::LShift);
            case AKEYCODE_CTRL_LEFT:
            case AKEYCODE_CTRL_RIGHT: return static_cast<int32_t>(KeyCode::LCtrl);
            case AKEYCODE_ALT_LEFT:
            case AKEYCODE_ALT_RIGHT: return static_cast<int32_t>(KeyCode::LAlt);
            case AKEYCODE_MOVE_HOME: return static_cast<int32_t>(KeyCode::Home);
            case AKEYCODE_MOVE_END: return static_cast<int32_t>(KeyCode::End);
            case AKEYCODE_PAGE_UP: return static_cast<int32_t>(KeyCode::PageUp);
            case AKEYCODE_PAGE_DOWN: return static_cast<int32_t>(KeyCode::PageDown);
            case AKEYCODE_INSERT: return static_cast<int32_t>(KeyCode::Insert);
            case AKEYCODE_CAPS_LOCK: return static_cast<int32_t>(KeyCode::CapsLock);
            case AKEYCODE_META_LEFT:
            case AKEYCODE_META_RIGHT: return static_cast<int32_t>(KeyCode::Super);
            case AKEYCODE_MINUS: return static_cast<int32_t>(KeyCode::Minus);
            case AKEYCODE_EQUALS: return static_cast<int32_t>(KeyCode::Plus);
            case AKEYCODE_COMMA: return static_cast<int32_t>(KeyCode::Comma);
            case AKEYCODE_PERIOD: return static_cast<int32_t>(KeyCode::Dot);
            case AKEYCODE_SLASH: return static_cast<int32_t>(KeyCode::Slash);
            case AKEYCODE_BACKSLASH: return static_cast<int32_t>(KeyCode::BackSlash);
            case AKEYCODE_GRAVE: return static_cast<int32_t>(KeyCode::Tilde);
            case AKEYCODE_SEMICOLON: return static_cast<int32_t>(KeyCode::Semicolon);
            case AKEYCODE_APOSTROPHE: return static_cast<int32_t>(KeyCode::Apostrophe);
            case AKEYCODE_LEFT_BRACKET: return static_cast<int32_t>(KeyCode::LeftBracket);
            case AKEYCODE_RIGHT_BRACKET: return static_cast<int32_t>(KeyCode::RightBracket);
            default: return -1;
        }
    }

    int32_t QueueAndroidInputEvent(const AInputEvent* input) {
        if (!input || !IsAndroidInputAccepted()) {
            return 0;
        }
        AndroidEvent event{};
        if (AInputEvent_getType(input) == AINPUT_EVENT_TYPE_KEY) {
            const auto key = AndroidKeyToEngine(AKeyEvent_getKeyCode(input));
            const auto action = AKeyEvent_getAction(input);
            if (key < 0 || (action != AKEY_EVENT_ACTION_DOWN && action != AKEY_EVENT_ACTION_UP)) {
                return 0;
            }
            event.type = AndroidEvent::Key;
            event.key.keyCode = AKeyEvent_getKeyCode(input);
            event.key.action = action;
            return PushAndroidInputEvent(event) ? 1 : 0;
        }
        if (AInputEvent_getType(input) != AINPUT_EVENT_TYPE_MOTION) {
            return 0;
        }
        const auto source = AInputEvent_getSource(input);
        const bool pointer = (source & AINPUT_SOURCE_CLASS_POINTER) != 0;
        if (!pointer) {
            return 0;
        }
        const int32_t rawAction = AMotionEvent_getAction(input);
        const int32_t action = rawAction & AMOTION_EVENT_ACTION_MASK;
        const SizeType count = AMotionEvent_getPointerCount(input);
        const SizeType changed = (rawAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        if (action == AMOTION_EVENT_ACTION_CANCEL) {
            event.type = AndroidEvent::InputReset;
            return PushAndroidInputEvent(event) ? 1 : 0;
        }
        const bool single = action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_UP ||
            action == AMOTION_EVENT_ACTION_POINTER_DOWN || action == AMOTION_EVENT_ACTION_POINTER_UP;
        if (count == 0 || (single && changed >= count)) {
            return 0;
        }
        event.type = AndroidEvent::Motion;
        event.motion.action = action;
        event.motion.source = source;
        event.motion.buttons = AMotionEvent_getButtonState(input);
        event.motion.scrollX = AMotionEvent_getAxisValue(input, AMOTION_EVENT_AXIS_HSCROLL, 0);
        event.motion.scrollY = AMotionEvent_getAxisValue(input, AMOTION_EVENT_AXIS_VSCROLL, 0);
        const SizeType begin = single ? changed : 0;
        const SizeType end = single ? changed + 1 : count;
        for (SizeType i = begin; i < end; ++i) {
            event.motion.pointerId = AMotionEvent_getPointerId(input, i);
            event.motion.x = AMotionEvent_getX(input, i);
            event.motion.y = AMotionEvent_getY(input, i);
            event.motion.pressure = AMotionEvent_getPressure(input, i);
            if (!PushAndroidInputEvent(event)) {
                return 0;
            }
        }
        return 1;
    }
}
#endif
