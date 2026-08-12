//
// Created by Monika on 22.01.2023.
//

#ifndef SR_ENGINE_SRSL_LEXERUTILS_H
#define SR_ENGINE_SRSL_LEXERUTILS_H

#include <Utils/Common/Enumerations.h>

namespace SR_UTILS_NS::LexerDetails {
    struct LexerInclude {
        SR_UTILS_NS::StringAtom name;
        SR_UTILS_NS::String buffer;
    };

    static SR_INLINE constexpr char SPACE_CHARS[] = { ' ', '\n', '\r', '\t' };

    static SR_INLINE constexpr char IDENTIFIER_CHARS[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'A',
        'b', 'B',
        'c', 'C',
        'd', 'D',
        'e', 'E',
        'f', 'F',
        'g', 'G',
        'h', 'H',
        'i', 'I',
        'j', 'J',
        'k', 'K',
        'l', 'L',
        'm', 'M',
        'n', 'N',
        'o', 'O',
        'p', 'P',
        'q', 'Q',
        'r', 'R',
        's', 'S',
        't', 'T',
        'u', 'U',
        'v', 'V',
        'w', 'W',
        'x', 'X',
        'y', 'Y',
        'z', 'Z',
        '_',
    };

    bool IsIdentifier(SR_UTILS_NS::StringView token) noexcept;
    bool IsOperator(SR_UTILS_NS::StringView operation) noexcept;

    /// decodes escape sequences ('\n', '\t', '\"', '\\', '\xFF' and etc.) of the raw string lexem value
    SR_NODISCARD SR_UTILS_NS::String UnescapeString(SR_UTILS_NS::StringView rawValue);

    SR_ENUM_NS_CLASS_T(LexemKind, uint8_t,
        Unknown,

        OpeningSquareBracket, /// [
        ClosingSquareBracket, /// ]

        OpeningAngleBracket,  /// <
        ClosingAngleBracket,  /// >

        OpeningCurlyBracket,  /// {
        ClosingCurlyBracket,  /// }

        OpeningBracket,       /// (
        ClosingBracket,       /// )

        Plus,                 /// +
        Minus,                /// -
        Multiply,             /// *
        Divide,               /// /
        Percent,              /// %

        Dollar,                /// $
        At,                    /// @

        Assign,               /// =
        Semicolon,            /// ;
        Dot,                  /// .
        Comma,                /// ,
        Negation,             /// !
        And,                  /// &
        Or,                   /// |
        Question,             /// ?
        Colon,                /// :
        Tilda,                /// ~
        Exponentiation,       /// ^

        Integer,              /// 0-9

        Macro,                /// #
        MacroEnd,             /// \n

        String,               /// "some text", the value is stored without quotes and without decoding of escape sequences

        Identifier            /// _az_AZ_19_
    );

    SR_MAYBE_UNUSED static SR_UTILS_NS::StringView LexemKindToString(LexemKind lexem) {
        static SR_UTILS_NS::Map<LexemKind, SR_UTILS_NS::StringView> lexemToString = {
            { LexemKind::OpeningSquareBracket, "[" },
            { LexemKind::ClosingSquareBracket, "]" },
            { LexemKind::OpeningAngleBracket, "<" },
            { LexemKind::ClosingAngleBracket, ">" },
            { LexemKind::OpeningCurlyBracket, "{" },
            { LexemKind::ClosingCurlyBracket, "}" },
            { LexemKind::OpeningBracket, "(" },
            { LexemKind::ClosingBracket, ")" },
            { LexemKind::Plus, "+" },
            { LexemKind::Minus, "-" },
            { LexemKind::Multiply, "*" },
            { LexemKind::Divide, "/" },
            { LexemKind::Percent, "%" },
            { LexemKind::Assign, "=" },
            { LexemKind::Semicolon, ";" },
            { LexemKind::Dot, "." },
            { LexemKind::Comma, "," },
            { LexemKind::Negation, "!" },
            { LexemKind::And, "&" },
            { LexemKind::Or, "|" },
            { LexemKind::Question, "?" },
            { LexemKind::Colon, ":" },
            { LexemKind::Tilda, "~" },
            { LexemKind::Dollar, "$" },
            { LexemKind::At, "@" },
            { LexemKind::Exponentiation, "^" },
            { LexemKind::Macro, "#" },
        };
        if (lexemToString.find(lexem) != lexemToString.end()) {
            return lexemToString[lexem];
        }
        return SR_UTILS_NS::StringView();
    }

    SR_ENUM_NS_CLASS_T(LexerReturnCode, uint16_t,
        Unknown, Success, OutOfBounds, InvalidLexicalTree,
        UnknownLexem, UnexceptedLexem, UnexceptedDot, InvalidExpression, InvalidComplexExpression, InvalidDecorator,
        IncompleteExpression, EmptyExpression, InvalidScope, InvalidCall, InvalidIfStatement, UnknownShaderLanguage,
        InvalidAngleBracket, InvalidAssign, InvalidMathToken, InvalidNumericToken, EmptyToken, InvalidIncrementOrDecrement, InvalidListEnd,
        WrongMacroName, IncludeNotExists, UnexceptedError, IncludeError, InvalidFunction, InvalidString
    );

    struct LocationEntity {
        LocationEntity() = default;

        LocationEntity(uint64_t offset, uint64_t length, uint16_t fileIndex, uint64_t line, uint64_t position)
            : offset(offset)
            , length(length)
            , fileIndex(fileIndex)
            , line(line)
            , position(position)
        { }

        uint32_t offset = 0;
        uint32_t position = 0;
        uint32_t line = 0;
        uint32_t length = 0;
        uint16_t fileIndex = 0;
    };

    struct Lexem : public LocationEntity {
        Lexem() = default;

        Lexem(uint64_t offset, uint64_t length, LexemKind kind, std::string_view value, uint16_t fileIndex, uint64_t line, uint64_t position)
            : LocationEntity(offset, length, fileIndex, line, position)
            , kind(kind)
            , value(value)
        { }

        Lexem(uint64_t offset, uint64_t length, LexemKind kind, uint16_t fileIndex, uint64_t line, uint64_t position)
            : LocationEntity(offset, length, fileIndex, line, position)
            , kind(kind)
        { }

        void SetEndOfLine() { isLineEnd = true; }

        LexemKind kind = LexemKind::Unknown;
        bool isLineEnd = false;
        std::string_view value;
    };

    struct LexerMessage {
        LexerMessage(LexerReturnCode code) /** NOLINT */
            : code(code)
        { }

        LexerMessage(LexerReturnCode code, const Lexem& lexem)
            : code(code)
            , position(lexem.position)
            , characterIndex(lexem.offset)
            , line(lexem.line)
            , fileIndex(lexem.fileIndex)
            , lexemKind(lexem.kind)
        { }

        LexerMessage(LexerReturnCode code, const Lexem* pLexem)
            : LexerMessage(code, *pLexem)
        { }

        SR_NODISCARD String ToString(const Vector<LexerInclude>& files, uint8_t tab) const;

        LexerMessage& SetDescription(const std::string& text) { description = text; return *this; }

        LexerReturnCode code = LexerReturnCode::Unknown;
        uint64_t position = SR_UINT64_MAX;
        uint64_t characterIndex = SR_UINT64_MAX;
        uint64_t line = SR_UINT64_MAX;
        uint16_t fileIndex = SR_UINT16_MAX;
        LexemKind lexemKind = LexemKind::Unknown;
        std::string description;

    };

    struct LexerResult {
        LexerResult() = default;

        LexerResult(LexerReturnCode code) { /** NOLINT */
            AddError(code);
        }

        LexerResult(LexerReturnCode code, const Lexem* pLexem) {
            AddError(LexerMessage(code, pLexem));
        }

        LexerMessage& AddError(const LexerMessage& message) {
            return errors.emplace_back(message);
        }

        void Clear() {
            processedLexems = 0;
            warnings.clear();
            errors.clear();
        }

        SR_NODISCARD bool HasErrors() const { return !errors.empty(); }
        SR_NODISCARD bool HasWarnings() const { return !warnings.empty(); }
        SR_NODISCARD bool HasAny() const { return HasErrors() || HasWarnings(); }

        SR_NODISCARD SR_UTILS_NS::String ToString(const SR_UTILS_NS::Vector<LexerInclude>& files, uint8_t tab = 1) const {
            SR_UTILS_NS::String message;

            for (auto&& msg : errors) {
                message += "\n" + std::string(tab, '\t') + "Error code: " + msg.ToString(files, tab + 1);
            }

            for (auto&& msg : warnings) {
                message += "\n" + std::string(tab, '\t') + "Warning: " + msg.ToString(files, tab + 1);
            }

            return message;
        }

        uint64_t processedLexems = 0;
        SR_UTILS_NS::Vector<LexerMessage> warnings;
        SR_UTILS_NS::Vector<LexerMessage> errors;
    };

    SR_MAYBE_UNUSED SR_INLINE_STATIC std::string LexemsToString(const std::vector<Lexem>& lexems) {
        std::string code;

        LexemKind previously = LexemKind::Unknown;
        for (auto&& lexem : lexems) {
            if (lexem.kind == previously && previously == LexemKind::Identifier) {
                code += " ";
            }
            if (lexem.kind == LexemKind::String) {
                /// the value is stored without quotes, but escape sequences are not decoded, so the string is restored as is
                code += "\"";
                code += lexem.value;
                code += "\"";
            }
            else {
                code += lexem.value;
            }
            previously = lexem.kind;
        }

        return code;
    }
}

#endif //SR_ENGINE_SRSL_LEXERUTILS_H
