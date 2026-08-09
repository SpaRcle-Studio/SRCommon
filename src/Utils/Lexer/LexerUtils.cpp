//
// Created by Monika on 17.07.2026.
//

#include <Utils/Lexer/LexerUtils.h>
#include <Utils/Resources/ResourceManager.h>

namespace SR_UTILS_NS::LexerDetails {
    bool IsIdentifier(SR_UTILS_NS::StringView token) noexcept {
        bool isFirst = true;
        for (auto&& tokenChar : token) {
            if (isFirst && SR_MATH_NS::IsNumber(std::string_view(&tokenChar, 1))) {
                return false;
            }
            isFirst = false;

            if ((tokenChar >= 'a' && tokenChar <= 'z') ||
                (tokenChar >= 'A' && tokenChar <= 'Z') ||
                (tokenChar >= '0' && tokenChar <= '9') ||
                (tokenChar == '_'))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    bool IsOperator(SR_UTILS_NS::StringView operation) noexcept {
        constexpr char operators[15] = {
            '+', '-', '!', '.', '~', '>', '^', '<', ':', '?', '|', '&', '%',
        };
        if (operation.size() != 1) {
            return false;
        }
        const char firstChar = operation[0];
        for (const char op : operators) {
            if (firstChar == op) {
                return true;
            }
        }
        return false;
    }

    String LexerMessage::ToString(const Vector<LexerInclude> &files, uint8_t tab) const {
        SR_UTILS_NS::String message = SR_UTILS_NS::EnumReflector::ToStringAtom(code);
        if (fileIndex != SR_UINT16_MAX) {
            if (fileIndex >= files.size()) {
                SRHalt("Invalid index!");
            }
            else {
                auto&& resourcesPath = SR_UTILS_NS::ResourceManager::Instance().GetResPath();
            #ifdef SR_WIN32
                message += "\n{}File: {}/{}:{}:{}"_format(std::string(tab, '\t'), resourcesPath, files[fileIndex].name.ToStringView(), line, position);
            #else
                message += "\n{}File: file:///{}/{}:{}:{}"_format(std::string(tab, '\t'), resourcesPath, files[fileIndex].name.ToStringView(), line, position);
            #endif
            }
        }

        if (!description.empty()) {
            message += "\n" + std::string(tab, '\t') + "Description: " + description;
        }

        if (lexemKind != LexemKind::Unknown) {
            message += "\n" + std::string(tab, '\t') + "Lexem: " + SR_UTILS_NS::EnumReflector::ToStringAtom(lexemKind).ToStringRef();
        }

        return message;
    }
}
