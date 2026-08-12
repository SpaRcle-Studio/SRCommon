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

    SR_UTILS_NS::String UnescapeString(SR_UTILS_NS::StringView rawValue) {
        SR_UTILS_NS::String result;
        result.reserve(rawValue.size());

        for (uint64_t i = 0; i < rawValue.size(); ++i) {
            if (rawValue[i] != '\\' || i + 1 >= rawValue.size()) {
                result += rawValue[i];
                continue;
            }

            const char escapedChar = rawValue[++i];

            switch (escapedChar) {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'v': result += '\v'; break;
                case 'f': result += '\f'; break;
                case 'b': result += '\b'; break;
                case 'a': result += '\a'; break;
                case '0': result += '\0'; break;
                case '"': result += '"'; break;
                case '\'': result += '\''; break;
                case '\\': result += '\\'; break;
                case '\n': break; /// line continuation
                case 'x': {
                    uint32_t code = 0;
                    uint8_t digits = 0;

                    while (digits < 2 && i + 1 < rawValue.size()) {
                        const char hexChar = rawValue[i + 1];

                        uint32_t digit = 0;
                        if (hexChar >= '0' && hexChar <= '9') {
                            digit = static_cast<uint32_t>(hexChar - '0');
                        }
                        else if (hexChar >= 'a' && hexChar <= 'f') {
                            digit = static_cast<uint32_t>(hexChar - 'a') + 10;
                        }
                        else if (hexChar >= 'A' && hexChar <= 'F') {
                            digit = static_cast<uint32_t>(hexChar - 'A') + 10;
                        }
                        else {
                            break;
                        }

                        code = (code << 4) | digit;
                        ++digits;
                        ++i;
                    }

                    if (digits == 0) {
                        result += escapedChar; /// there is nothing to decode, keep the character as is
                    }
                    else {
                        result += static_cast<char>(code);
                    }
                    break;
                }
                default:
                    /// unknown escape sequence, keep the character as is
                    result += escapedChar;
                    break;
            }
        }

        return result;
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
