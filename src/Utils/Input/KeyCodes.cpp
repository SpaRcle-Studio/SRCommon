//
// Created by monika on 2/7/26.
//

#include <Utils/Input/KeyCodes.h>

namespace SR_UTILS_NS {
    bool IsPrintableKeyCode(KeyCode code) {
        if (code == KeyCode::Minus || code == KeyCode::Plus
            || code == KeyCode::Dot || code == KeyCode::Tilde
            || code == KeyCode::Slash || code == KeyCode::BackSlash
            || code == KeyCode::Semicolon || code == KeyCode::Apostrophe
            || code == KeyCode::LeftBracket || code == KeyCode::RightBracket
        ) {
            return true;
        }

        const int32_t codeInt = static_cast<int32_t>(code);

        return (codeInt >= static_cast<int32_t>(KeyCode::_0) && codeInt <= static_cast<int32_t>(KeyCode::_9)) ||
               (codeInt >= static_cast<int32_t>(KeyCode::A) && codeInt <= static_cast<int32_t>(KeyCode::Z));
    }

    SR_UTILS_NS::KeyCode KeyCodeFromXkbKeysym(uint32_t keysym) {
        switch (keysym) {
            case 48: return SR_UTILS_NS::KeyCode::_0;
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
            case 65535: return SR_UTILS_NS::KeyCode::Delete;
            case 65288: return SR_UTILS_NS::KeyCode::BackSpace;
            case 47: return SR_UTILS_NS::KeyCode::Slash;
            case 92: return SR_UTILS_NS::KeyCode::BackSlash;
            case 65307: return SR_UTILS_NS::KeyCode::Escape;
            case 65507: return SR_UTILS_NS::KeyCode::LCtrl;
            case 65505: return SR_UTILS_NS::KeyCode::LShift;
            case 65513: return SR_UTILS_NS::KeyCode::LAlt;
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

    SR_UTILS_NS::KeyCode KeyCodeFromEvdev(uint32_t keysym) {
        switch (keysym) {
            case 1: return SR_UTILS_NS::KeyCode::Escape;

            case 2: return SR_UTILS_NS::KeyCode::_1;
            case 3: return SR_UTILS_NS::KeyCode::_2;
            case 4: return SR_UTILS_NS::KeyCode::_3;
            case 5: return SR_UTILS_NS::KeyCode::_4;
            case 6: return SR_UTILS_NS::KeyCode::_5;
            case 7: return SR_UTILS_NS::KeyCode::_6;
            case 8: return SR_UTILS_NS::KeyCode::_7;
            case 9: return SR_UTILS_NS::KeyCode::_8;
            case 10: return SR_UTILS_NS::KeyCode::_9;
            case 11: return SR_UTILS_NS::KeyCode::_0;
            case 12: return SR_UTILS_NS::KeyCode::Minus;
            case 13: return SR_UTILS_NS::KeyCode::Plus;
            case 14: return SR_UTILS_NS::KeyCode::BackSpace;

            case 15: return SR_UTILS_NS::KeyCode::Tab;
            case 16: return SR_UTILS_NS::KeyCode::Q;
            case 17: return SR_UTILS_NS::KeyCode::W;
            case 18: return SR_UTILS_NS::KeyCode::E;
            case 19: return SR_UTILS_NS::KeyCode::R;
            case 20: return SR_UTILS_NS::KeyCode::T;
            case 21: return SR_UTILS_NS::KeyCode::Y;
            case 22: return SR_UTILS_NS::KeyCode::U;
            case 23: return SR_UTILS_NS::KeyCode::I;
            case 24: return SR_UTILS_NS::KeyCode::O;
            case 25: return SR_UTILS_NS::KeyCode::P;

            case 26: return SR_UTILS_NS::KeyCode::LeftBracket;
            case 27: return SR_UTILS_NS::KeyCode::RightBracket;

            case 28: return SR_UTILS_NS::KeyCode::Enter;
            case 29: return SR_UTILS_NS::KeyCode::LCtrl;

            case 30: return SR_UTILS_NS::KeyCode::A;
            case 31: return SR_UTILS_NS::KeyCode::S;
            case 32: return SR_UTILS_NS::KeyCode::D;
            case 33: return SR_UTILS_NS::KeyCode::F;
            case 34: return SR_UTILS_NS::KeyCode::G;
            case 35: return SR_UTILS_NS::KeyCode::H;
            case 36: return SR_UTILS_NS::KeyCode::J;
            case 37: return SR_UTILS_NS::KeyCode::K;
            case 38: return SR_UTILS_NS::KeyCode::L;
            case 39: return SR_UTILS_NS::KeyCode::Semicolon;
            case 40: return SR_UTILS_NS::KeyCode::Apostrophe;

            case 41: return SR_UTILS_NS::KeyCode::Tilde;

            case 42: return SR_UTILS_NS::KeyCode::LShift;
            case 43: return SR_UTILS_NS::KeyCode::BackSlash;

            case 44: return SR_UTILS_NS::KeyCode::Z;
            case 45: return SR_UTILS_NS::KeyCode::X;
            case 46: return SR_UTILS_NS::KeyCode::C;
            case 47: return SR_UTILS_NS::KeyCode::V;
            case 48: return SR_UTILS_NS::KeyCode::B;
            case 49: return SR_UTILS_NS::KeyCode::N;
            case 50: return SR_UTILS_NS::KeyCode::M;

            case 51: return SR_UTILS_NS::KeyCode::Comma;
            case 52: return SR_UTILS_NS::KeyCode::Dot;
            case 53: return SR_UTILS_NS::KeyCode::Slash;
            case 54: return SR_UTILS_NS::KeyCode::RShift;
            case 56: return SR_UTILS_NS::KeyCode::LAlt;
            case 57: return SR_UTILS_NS::KeyCode::Space;

            case 58: return SR_UTILS_NS::KeyCode::CapsLock;

            case 59: return SR_UTILS_NS::KeyCode::F1;
            case 60: return SR_UTILS_NS::KeyCode::F2;
            case 61: return SR_UTILS_NS::KeyCode::F3;
            case 62: return SR_UTILS_NS::KeyCode::F4;
            case 63: return SR_UTILS_NS::KeyCode::F5;
            case 64: return SR_UTILS_NS::KeyCode::F6;
            case 65: return SR_UTILS_NS::KeyCode::F7;
            case 66: return SR_UTILS_NS::KeyCode::F8;
            case 67: return SR_UTILS_NS::KeyCode::F9;
            case 68: return SR_UTILS_NS::KeyCode::F10;

            case 87: return SR_UTILS_NS::KeyCode::F11;
            case 88: return SR_UTILS_NS::KeyCode::F12;

            case 97: return SR_UTILS_NS::KeyCode::RCtrl;

            case 100: return SR_UTILS_NS::KeyCode::RAlt;

            case 102: return SR_UTILS_NS::KeyCode::Home;

            case 103: return SR_UTILS_NS::KeyCode::UpArrow;

            case 104: return SR_UTILS_NS::KeyCode::PageUp;

            case 105: return SR_UTILS_NS::KeyCode::LeftArrow;
            case 106: return SR_UTILS_NS::KeyCode::RightArrow;

            case 107: return SR_UTILS_NS::KeyCode::End;

            case 108: return SR_UTILS_NS::KeyCode::DownArrow;

            case 109: return SR_UTILS_NS::KeyCode::PageDown;
            case 110: return SR_UTILS_NS::KeyCode::Insert;
            case 111: return SR_UTILS_NS::KeyCode::Delete;

            case 125: return SR_UTILS_NS::KeyCode::Super;

            case 127: return SR_UTILS_NS::KeyCode::Menu;

            default:
                break;
        }
        SR_WARN("KeyCodeFromEvdev() : unmapped keysym: {}", keysym);
        return SR_UTILS_NS::KeyCode::None;
    }
}