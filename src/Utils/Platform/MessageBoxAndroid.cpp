//
// Created by Monika on 02.05.2025.
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
        // Android does not support message boxes now
        return MessageBoxResultType::Cancel;
    }
}