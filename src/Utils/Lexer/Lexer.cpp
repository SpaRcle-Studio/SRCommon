//
// Created by Monika on 22.01.2023.
//

#include <Utils/Lexer/Lexer.h>
#include <Utils/FileSystem/FileSystem.h>

namespace SR_UTILS_NS {
    Lexer::~Lexer() {
        Clear();
    }

    Lexer::Lexems Lexer::Parse(SR_UTILS_NS::IAllocator* pAllocator, uint32_t lexemsReserve, const SR_UTILS_NS::Path& path, SR_UTILS_NS::String& buffer, uint16_t fileIndex) {
        SR_TRACY_ZONE;

        if (!SR_UTILS_NS::FileSystem::ReadFile(path, buffer) || buffer.empty()) {
            SR_ERROR("Lexer::Parse() : failed to read file!\n\tPath: {}", path);
            return { };
        }

        return ParseInternal(pAllocator, lexemsReserve, buffer, fileIndex);
    }

    Lexer::Lexems Lexer::ParseString(SR_UTILS_NS::IAllocator* pAllocator, uint32_t lexemsReserve, std::string_view code, uint16_t fileIndex) {
        SR_TRACY_ZONE;
        return ParseInternal(pAllocator, lexemsReserve, code, fileIndex);
    }

    bool Lexer::InBounds() const noexcept {
        return m_offset < m_source.size();
    }

    Lexer::Lexems Lexer::ParseInternal(SR_UTILS_NS::IAllocator* pAllocator, uint32_t lexemsReserve, std::string_view code, uint16_t fileIndex) {
        SR_TRACY_ZONE;

        Clear();

        m_pAllocator = pAllocator;
        m_lexems = SR_UTILS_NS::Vector<LexerDetails::Lexem>(m_pAllocator);
        m_lexems.reserve(lexemsReserve);
        m_source = code;
        m_fileIndex = fileIndex;

        while (InBounds()) {
            SkipSpaces();

            if (!InBounds()) {
                break;
            }

            if (auto&& lexem = ProcessLexem()) {
                m_lexems.emplace_back(lexem.value());
            }
        }

        return SR_UTILS_NS::Exchange(m_lexems, { });
    }

    Lexer::ProcessedLexem Lexer::ProcessLexem() {
        const char actualChar = m_source[m_offset];

        switch (actualChar) {
            case '[': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::OpeningSquareBracket, "[", m_fileIndex, m_line, m_position++);
            case ']': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::ClosingSquareBracket, "]", m_fileIndex, m_line, m_position++);

            case '<': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::OpeningAngleBracket, "<", m_fileIndex, m_line, m_position++);
            case '>': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::ClosingAngleBracket, ">", m_fileIndex, m_line, m_position++);

            case '{': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::OpeningCurlyBracket, "{", m_fileIndex, m_line, m_position++);
            case '}': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::ClosingCurlyBracket, "}", m_fileIndex, m_line, m_position++);

            case '(': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::OpeningBracket, "(", m_fileIndex, m_line, m_position++);
            case ')': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::ClosingBracket, ")", m_fileIndex, m_line, m_position++);

            case '+': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Plus, "+", m_fileIndex, m_line, m_position++);
            case '-': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Minus, "-", m_fileIndex, m_line, m_position++);
            case '*': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Multiply, "*", m_fileIndex, m_line, m_position++);
            case '%': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Percent, "%", m_fileIndex, m_line, m_position++);
            case '^': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Exponentiation, "^", m_fileIndex, m_line, m_position++);
            case '?': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Question, "?", m_fileIndex, m_line, m_position++);
            case ':': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Colon, ":", m_fileIndex, m_line, m_position++);
            case '~': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Tilda, "~", m_fileIndex, m_line, m_position++);

            case '/': {
                if (m_offset + 1 < m_source.size()) {
                    switch (m_source[m_offset + 1]) {
                        case '/':
                        case '*':
                            SkipComment();
                            return {};
                        default:
                            break;
                    }
                }

                return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Divide, "/", m_fileIndex, m_line, m_position++);
            }
            case '=': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Assign, "=", m_fileIndex, m_line, m_position++);
            case ';': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Semicolon, ";", m_fileIndex, m_line, m_position++);
            case '.': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Dot, ".", m_fileIndex, m_line, m_position++);
            case ',': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Comma, ",", m_fileIndex, m_line, m_position++);
            case '!': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Negation, "!", m_fileIndex, m_line, m_position++);
            case '&': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::And, "&", m_fileIndex, m_line, m_position++);
            case '|': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Or, "|", m_fileIndex, m_line, m_position++);
            case '$': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Dollar, "$", m_fileIndex, m_line, m_position++);
            case '@': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::At, "@", m_fileIndex, m_line, m_position++);

            case '0': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "0", m_fileIndex, m_line, m_position++);
            case '1': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "1", m_fileIndex, m_line, m_position++);
            case '2': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "2", m_fileIndex, m_line, m_position++);
            case '3': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "3", m_fileIndex, m_line, m_position++);
            case '4': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "4", m_fileIndex, m_line, m_position++);
            case '5': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "5", m_fileIndex, m_line, m_position++);
            case '6': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "6", m_fileIndex, m_line, m_position++);
            case '7': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "7", m_fileIndex, m_line, m_position++);
            case '8': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "8", m_fileIndex, m_line, m_position++);
            case '9': return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Integer, "9", m_fileIndex, m_line, m_position++);

            case '#':
                m_macroLine = true;
                return LexerDetails::Lexem(m_offset++, 1, LexerDetails::LexemKind::Macro, "#", m_fileIndex, m_line, m_position++);

            case '"': return ProcessString();

            default:
                break;
        }

        const uint64_t offset = m_offset;
        const uint64_t position = m_position;

        auto&& identifier = ProcessIdentifier();

        const uint64_t length = identifier.size();

        return LexerDetails::Lexem(offset, length, LexerDetails::LexemKind::Identifier, identifier, m_fileIndex, m_line, position);
    }

    Lexer::ProcessedLexem Lexer::ProcessString() {
        const uint64_t offset = m_offset;
        const uint64_t line = m_line;
        const uint64_t position = m_position;

        ++m_offset; /// skip opening '"'
        ++m_position;

        const uint64_t contentOffset = m_offset;
        uint64_t contentLength = 0;
        bool isTerminated = false;

        while (InBounds()) {
            const char stringChar = m_source[m_offset];

            /// any character after the '\' is a part of the string, even '"' or '\' itself
            if (stringChar == '\\') {
                ++m_offset;
                ++m_position;
                ++contentLength;

                if (InBounds()) {
                    if (m_source[m_offset] == '\n') {
                        ++m_line;
                        m_position = 0;
                    }
                    else {
                        ++m_position;
                    }
                    ++m_offset;
                    ++contentLength;
                }

                continue;
            }

            if (stringChar == '"') {
                ++m_offset; /// skip closing '"'
                ++m_position;
                isTerminated = true;
                break;
            }

            /// all other characters are taken as is, spaces and comments are not processed inside of the string
            if (stringChar == '\n') {
                ++m_line;
                m_position = 0;
            }
            else {
                ++m_position;
            }

            ++m_offset;
            ++contentLength;
        }

        if (!isTerminated) {
            SR_ERROR("Lexer::ProcessString() : unterminated string!"
                 " \n\tLine: {}"
                 " \n\tPosition: {}", line, position
            );
        }

        /// the value doesn't contain quotes, but escape sequences are kept as is, use LexerDetails::UnescapeString() to decode them
        const std::string_view value = contentLength > 0 ? std::string_view(m_source.data() + contentOffset, contentLength) : std::string_view();

        /// the length contains quotes to make the location point to the whole string
        return LexerDetails::Lexem(offset, m_offset - offset, LexerDetails::LexemKind::String, value, m_fileIndex, line, position);
    }

    std::string_view Lexer::ProcessIdentifier() {
        uint32_t identifierStart = SR_UINT32_MAX;
        uint32_t identifierLength = 0;

    retry:
        if (InBounds()) {
            const char firstChar = m_source[m_offset];
            if ((firstChar >= 'a' && firstChar <= 'z') || (firstChar >= 'A' && firstChar <= 'Z') || firstChar == '_' || (firstChar >= '0' && firstChar <= '9')) {
                if (identifierStart == SR_UINT32_MAX) {
                    identifierStart = m_offset;
                }
                ++identifierLength;
                ++m_offset;
                ++m_position;
                goto retry;
            }
        }

        if (identifierLength == 0) {
            SR_ERROR("Lexer::ProcessIdentifier() : invalid identifier!"
                 " \n\tPosition: {}"
                 " \n\tIdentifier: \"{}\"", m_offset, m_source[m_offset]
            );
            ++m_offset;
            ++m_position;
        }

        if (identifierStart == SR_UINT32_MAX) {
            return {};
        }
        return std::string_view(m_source.data() + identifierStart, identifierLength);
    }

    void Lexer::SkipSpaces() {
    retry:
        while (InBounds()) {
            for (auto&& spaceChar : LexerDetails::SPACE_CHARS) {
                if (spaceChar == m_source[m_offset]) {
                    ++m_offset;
                    ++m_position;

                    if (spaceChar == '\n') {
                        if (m_macroLine) {
                            m_macroLine = false;
                            m_lexems.emplace_back(LexerDetails::Lexem(m_offset, 1, LexerDetails::LexemKind::MacroEnd, "\n", m_fileIndex, m_line, m_position));
                        }
                        if (!m_lexems.empty()) {
                            m_lexems.back().SetEndOfLine();
                        }
                        ++m_line;
                        m_position = 0;
                    }

                    goto retry;
                }
            }

            break;
        }
    }

    void Lexer::SkipComment() {
        m_offset += 1; /// skip '/'
        m_position += 1;

        switch (m_source[m_offset]) {
            case '/': {
                m_offset += 1; /// skip '/'
                m_position += 1;
                while (m_offset < m_source.size() && m_source[m_offset] != '\n' && m_source[m_offset] != '\r') {
                    m_offset += 1;
                    m_position += 1;
                }
                break;
            }
            case '*': {
                m_offset += 1; /// skip '*'
                m_position += 1;
                while (m_offset + 1 < m_source.size() && !(m_source[m_offset] == '*' && m_source[m_offset + 1] == '/')) {
                    m_offset += 1;
                    m_position += 1;
                }
                /// skip "*/"
                if (m_offset + 1 < m_source.size() && (m_source[m_offset] == '*' && m_source[m_offset + 1] == '/')) {
                    m_offset += 2;
                    m_position += 2;
                }
                break;
            }
            default:
                SRHalt("Lexer::SkipComment() : something went wrong!");
                return;
        }
    }

    void Lexer::Clear() {
        m_macroLine = false;
        m_source = { };
        m_fileIndex = 0;
        m_offset = 0;
        m_lexems.clear();
        m_line = 0;
        m_position = 0;
    }
}