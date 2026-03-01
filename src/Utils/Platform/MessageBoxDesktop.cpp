//
// Created by Monika on 02.05.2025.
//

#include <Utils/Platform/MessageBox.h>
#include <Utils/Platform/Platform.h>

#include <tinyfiledialogs/tinyfiledialogs.h>

namespace SR_PLATFORM_NS {
    MessageBoxResultType ShowMessageBox(
        const std::string_view& title,
        const std::string_view& message,
        MessageBoxType type,
        MessageBoxIconType iconType,
        MessageBoxDefaultButtonType defaultButtonType
    ) {
        std::string_view dialogTypeStr;
        std::string_view iconTypeStr;

        switch (type) {
            case MessageBoxType::Ok:
                dialogTypeStr = "ok";
                break;
            case MessageBoxType::OkCancel:
                dialogTypeStr = "okcancel";
                break;
            case MessageBoxType::YesNo:
                dialogTypeStr = "yesno";
                break;
            case MessageBoxType::YesNoCancel:
                dialogTypeStr = "yesnocancel";
                break;
            default:
                SR_PLATFORM_NS::WriteConsoleError("Unknown message box type!");
                SR_MAKE_BREAKPOINT;
                return MessageBoxResultType::Cancel;
        }

        switch (iconType) {
            case MessageBoxIconType::Info:
                iconTypeStr = "info";
                break;
            case MessageBoxIconType::Warning:
                iconTypeStr = "warning";
                break;
            case MessageBoxIconType::Error:
                iconTypeStr = "error";
                break;
            case MessageBoxIconType::Question:
                iconTypeStr = "question";
                break;
            default:
                SR_PLATFORM_NS::WriteConsoleError("Unknown message box icon type!");
                SR_MAKE_BREAKPOINT;
                return MessageBoxResultType::Cancel;
        }

        int defaultButton = 0;
        switch (defaultButtonType) {
            case MessageBoxDefaultButtonType::YesOk:
                defaultButton = 1;
                break;
            case MessageBoxDefaultButtonType::Cancel:
                defaultButton = 0;
                break;
            case MessageBoxDefaultButtonType::No:
                if (type == MessageBoxType::YesNoCancel) {
                    defaultButton = 2; /// 2 for no in yesnocancel
                }
                else {
                    defaultButton = 0; ///  0 for cancel/no
                }
                break;
            default:
                SR_PLATFORM_NS::WriteConsoleError("Unknown message box default button type!");
                SR_MAKE_BREAKPOINT;
                return MessageBoxResultType::Cancel;
        }

        /// 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel
        switch (tinyfd_messageBox(title.data(), message.data(), dialogTypeStr.data(), iconTypeStr.data(), defaultButton)) {
            case 0: {
                switch (type) {
                    case MessageBoxType::YesNo:
                        return MessageBoxResultType::No;
                    case MessageBoxType::YesNoCancel:
                    case MessageBoxType::OkCancel:
                        return MessageBoxResultType::Cancel;
                    default:
                        break;
                }
                break;
            }
            case 1: {
                return MessageBoxResultType::YesOk;
            }
            case 2: {
                switch (type) {
                    case MessageBoxType::YesNoCancel:
                        return MessageBoxResultType::No;
                    default:
                        break;
                }
                break;
            }
        }

        SR_PLATFORM_NS::WriteConsoleError("Unknown message box result type!");
        SR_MAKE_BREAKPOINT;
        return MessageBoxResultType::Cancel;
    }
}