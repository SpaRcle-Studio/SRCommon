//
// Created by Monika on 17.02.2022.
//

#ifndef SR_ENGINE_KEYCODES_H
#define SR_ENGINE_KEYCODES_H

#include <Utils/Debug.h>

#ifdef KeyCode
    #undef KeyCode
#endif

#ifdef KeyState
    #undef KeyState
#endif

#ifdef None
    #undef None
#endif

namespace SR_UTILS_NS {
    enum class KeyboardLayout : uint8_t {
        EN, RU, UNKNOWN
    };

    enum class KeyState : uint8_t {
        None, Down, Up, Press
    };

    enum class MouseCode : uint8_t {
        MouseLeft = 0,
        MouseRight = 1,
        MouseMiddle = 2,
        MouseButton4 = 3,
        MouseButton5 = 4,
        None = std::numeric_limits<uint8_t>::max() - 1
    };

    const MouseCode MouseCodes[] = {
        MouseCode::MouseLeft,
        MouseCode::MouseRight,
        MouseCode::MouseMiddle,
        MouseCode::MouseButton4,
        MouseCode::MouseButton5,
    };

    SR_ENUM_NS_CLASS_T(KeyCode, uint8_t,
        MouseLeft = 0,
        MouseRight = 1,
        MouseMiddle = 2,
        MouseButton4 = 3,
        MouseButton5 = 4,
        BackSpace = 8,
        Tab = 9,
        Enter = 13,
        LShift = 16,
        Ctrl = 17,
        Alt = 18,
        Escape = 27,
        Space = 32,
        LeftArrow = 37,
        UpArrow = 38,
        RightArrow = 39,
        DownArrow = 40,
        Del = 46,

        _0 = 48,
        _1 = 49,
        _2 = 50,
        _3 = 51,
        _4 = 52,
        _5 = 53,
        _6 = 54,
        _7 = 55,
        _8 = 56,
        _9 = 57,

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        S = 83,
        R = 82,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        Super = 91,
        F1 = 112,
        F2 = 113,
        F3 = 114,
        F4 = 115,
        F5 = 116,
        F6 = 117,
        F7 = 118,
        F8 = 119,
        F9 = 120,
        F10 = 121,
        F11 = 122,
        F12 = 123,
        Plus = 187,
        Minus = 189,
        Dot = 190,
        Slash = 191,
        Tilde = 192,
        BackSlash = 220,
        CapsLock = 240,

        PageUp = 33,
        PageDown = 34,
        End = 35,
        Home = 36,
        Insert = 45,

        None = std::numeric_limits<uint8_t>::max() - 1
    );

    const KeyCode KeyCodes[] = {
        KeyCode::MouseLeft,
        KeyCode::MouseRight,
        KeyCode::MouseMiddle,
        KeyCode::MouseButton4,
        KeyCode::MouseButton5,
        KeyCode::BackSpace,
        KeyCode::Tab,
        KeyCode::Enter,
        KeyCode::LShift,
        KeyCode::Ctrl,
        KeyCode::Alt,
        KeyCode::Escape,
        KeyCode::Space,
        KeyCode::LeftArrow,
        KeyCode::UpArrow,
        KeyCode::RightArrow,
        KeyCode::DownArrow,
        KeyCode::Del,

        KeyCode::_0,
        KeyCode::_1,
        KeyCode::_2,
        KeyCode::_3,
        KeyCode::_4,
        KeyCode::_5,
        KeyCode::_6,
        KeyCode::_7,
        KeyCode::_8,
        KeyCode::_9,

        KeyCode::A,
        KeyCode::B,
        KeyCode::C,
        KeyCode::D,
        KeyCode::E,
        KeyCode::F,
        KeyCode::G,
        KeyCode::H,
        KeyCode::I,
        KeyCode::J,
        KeyCode::K,
        KeyCode::L,
        KeyCode::M,
        KeyCode::N,
        KeyCode::O,
        KeyCode::P,
        KeyCode::Q,
        KeyCode::S,
        KeyCode::R,
        KeyCode::T,
        KeyCode::U,
        KeyCode::V,
        KeyCode::W,
        KeyCode::X,
        KeyCode::Y,
        KeyCode::Z,
        KeyCode::Super,
        KeyCode::F1,
        KeyCode::F2,
        KeyCode::F3,
        KeyCode::F4,
        KeyCode::F5,
        KeyCode::F6,
        KeyCode::F7,
        KeyCode::F8,
        KeyCode::F9,
        KeyCode::F10,
        KeyCode::F11,
        KeyCode::F12,
        KeyCode::Plus,
        KeyCode::Minus,
        KeyCode::Dot,
        KeyCode::Slash,
        KeyCode::Tilde,
        KeyCode::BackSlash,
        KeyCode::CapsLock,

        KeyCode::PageUp,
        KeyCode::PageDown,
        KeyCode::End,
        KeyCode::Home,
        KeyCode::Insert,
    };

     SR_INLINE_STATIC SR_UTILS_NS::KeyCode KeyCodeFromXkbKeysym(uint32_t keysym) {
        switch (keysym) {
            case 48: return SR_UTILS_NS::KeyCode::_1;
            case 49: return SR_UTILS_NS::KeyCode::_1;
            case 50: return SR_UTILS_NS::KeyCode::_2;
            case 51: return SR_UTILS_NS::KeyCode::_3;
            case 52: return SR_UTILS_NS::KeyCode::_4;
            case 53: return SR_UTILS_NS::KeyCode::_5;
            case 54: return SR_UTILS_NS::KeyCode::_6;
            case 55: return SR_UTILS_NS::KeyCode::_7;
            case 56: return SR_UTILS_NS::KeyCode::_8;
            case 57: return SR_UTILS_NS::KeyCode::_9;
            case 113: return SR_UTILS_NS::KeyCode::Q;
            case 119: return SR_UTILS_NS::KeyCode::W;
            case 101: return SR_UTILS_NS::KeyCode::E;
            case 114: return SR_UTILS_NS::KeyCode::R;
            case 116: return SR_UTILS_NS::KeyCode::T;
            case 121: return SR_UTILS_NS::KeyCode::Y;
            case 117: return SR_UTILS_NS::KeyCode::U;
            case 105: return SR_UTILS_NS::KeyCode::I;
            case 111: return SR_UTILS_NS::KeyCode::O;
            case 112: return SR_UTILS_NS::KeyCode::P;
            case 97: return SR_UTILS_NS::KeyCode::A;
            case 115: return SR_UTILS_NS::KeyCode::S;
            case 100: return SR_UTILS_NS::KeyCode::D;
            case 102: return SR_UTILS_NS::KeyCode::F;
            case 103: return SR_UTILS_NS::KeyCode::G;
            case 104: return SR_UTILS_NS::KeyCode::H;
            case 106: return SR_UTILS_NS::KeyCode::J;
            case 107: return SR_UTILS_NS::KeyCode::K;
            case 108: return SR_UTILS_NS::KeyCode::L;
            case 122: return SR_UTILS_NS::KeyCode::Z;
            case 120: return SR_UTILS_NS::KeyCode::X;
            case 99: return SR_UTILS_NS::KeyCode::C;
            case 118: return SR_UTILS_NS::KeyCode::V;
            case 98: return SR_UTILS_NS::KeyCode::B;
            case 110: return SR_UTILS_NS::KeyCode::N;
            case 109: return SR_UTILS_NS::KeyCode::M;
            case 65470: return SR_UTILS_NS::KeyCode::F1;
            case 65471: return SR_UTILS_NS::KeyCode::F2;
            case 65472: return SR_UTILS_NS::KeyCode::F3;
            case 65473: return SR_UTILS_NS::KeyCode::F4;
            case 65474: return SR_UTILS_NS::KeyCode::F5;
            case 65475: return SR_UTILS_NS::KeyCode::F6;
            case 65476: return SR_UTILS_NS::KeyCode::F7;
            case 65477: return SR_UTILS_NS::KeyCode::F8;
            case 65478: return SR_UTILS_NS::KeyCode::F9;
            case 65479: return SR_UTILS_NS::KeyCode::F10;
            case 65480: return SR_UTILS_NS::KeyCode::F11;
            case 65481: return SR_UTILS_NS::KeyCode::F12;
            case 61: return SR_UTILS_NS::KeyCode::Plus;
            case 45: return SR_UTILS_NS::KeyCode::Minus;
            case 46: return SR_UTILS_NS::KeyCode::Dot;
            case 96: return SR_UTILS_NS::KeyCode::Tilde;
            case 65361: return SR_UTILS_NS::KeyCode::LeftArrow;
            case 65362: return SR_UTILS_NS::KeyCode::UpArrow;
            case 65363: return SR_UTILS_NS::KeyCode::RightArrow;
            case 65364: return SR_UTILS_NS::KeyCode::DownArrow;
            case 65535: return SR_UTILS_NS::KeyCode::Del;
            case 65288: return SR_UTILS_NS::KeyCode::BackSpace;
            case 47: return SR_UTILS_NS::KeyCode::Slash;
            case 92: return SR_UTILS_NS::KeyCode::BackSlash;
            case 65307: return SR_UTILS_NS::KeyCode::Escape;
            case 65507: return SR_UTILS_NS::KeyCode::Ctrl;
            case 65505: return SR_UTILS_NS::KeyCode::LShift;
            case 65513: return SR_UTILS_NS::KeyCode::Alt;
            case 65515: return SR_UTILS_NS::KeyCode::Super;
            case 32: return SR_UTILS_NS::KeyCode::Space;
            case 65289: return SR_UTILS_NS::KeyCode::Tab;
            case 65509: return SR_UTILS_NS::KeyCode::CapsLock;
            case 65379: return SR_UTILS_NS::KeyCode::Insert;
            case 65360: return SR_UTILS_NS::KeyCode::Home;
            case 65367: return SR_UTILS_NS::KeyCode::End;
            case 65365: return SR_UTILS_NS::KeyCode::PageUp;
            case 65366: return SR_UTILS_NS::KeyCode::PageDown;
            case 65293: return SR_UTILS_NS::KeyCode::Enter;
            default:
                break;
        }
        SR_WARN("KeyCodeFromXkbKeysym() : unmapped keysym: {}", keysym);
        return SR_UTILS_NS::KeyCode::None;
    }
}

#endif //SR_ENGINE_KEYCODES_H
