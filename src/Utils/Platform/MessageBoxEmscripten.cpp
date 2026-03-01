//
// Created by Monika on 01.03.2026.
//

#include <Utils/Platform/MessageBox.h>

namespace SR_PLATFORM_NS {
    MessageBoxResultType ShowMessageBox(
        const std::string_view& title,
        const std::string_view& message,
        MessageBoxType type,
        MessageBoxIconType iconType,
        MessageBoxDefaultButtonType defaultButtonType
    ) {
        // Emscripten does not support message boxes now
        return MessageBoxResultType::Cancel;
    }
}