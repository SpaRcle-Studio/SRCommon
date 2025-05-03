//
// Created by Monika on 02.05.2025.
//

#ifndef SR_ENGINE_UTILS_PLATFORM_MESSAGE_BOX_H
#define SR_ENGINE_UTILS_PLATFORM_MESSAGE_BOX_H

#include <Utils/stdInclude.h>

namespace SR_PLATFORM_NS {
    enum class MessageBoxType : uint8_t {
        Ok, OkCancel, YesNo, YesNoCancel
    };

    enum class MessageBoxDefaultButtonType : uint8_t {
        YesOk, No, Cancel
    };

    enum class MessageBoxIconType : uint8_t {
        Info,
        Warning,
        Error,
        Question
    };

    enum class MessageBoxResultType : uint8_t {
        YesOk = 0,
        No = 1,
        Cancel = 2,
    };

    SR_COMMON_DLL_API extern MessageBoxResultType ShowMessageBox(
        const std::string_view& title,
        const std::string_view& message,
        MessageBoxType type,
        MessageBoxIconType iconType,
        MessageBoxDefaultButtonType defaultButtonType
    );
}

#endif //SR_ENGINE_UTILS_PLATFORM_MESSAGE_BOX_H
