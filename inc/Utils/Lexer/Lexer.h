//
// Created by Monika on 22.01.2023.
//

#ifndef SR_ENGINE_COMMON_LEXER_H
#define SR_ENGINE_COMMON_LEXER_H

#include <Utils/Lexer/LexerUtils.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Types/Optional.h>

namespace SR_UTILS_NS {
    class Lexer : public Singleton<Lexer> {
        SR_REGISTER_SINGLETON(Lexer)
        using Lexems = Vector<LexerDetails::Lexem>;
        using ProcessedLexem = Optional<LexerDetails::Lexem>;
    protected:
        ~Lexer() override;

    public:
        SR_NODISCARD Lexems Parse(SR_UTILS_NS::IAllocator* pAllocator, uint32_t lexemsReserve, const SR_UTILS_NS::Path& path, SR_UTILS_NS::String& buffer, uint16_t fileIndex);
        SR_NODISCARD Lexems ParseString(SR_UTILS_NS::IAllocator* pAllocator, uint32_t lexemsReserve, std::string_view code, uint16_t fileIndex);

    private:
        SR_NODISCARD bool InBounds() const noexcept;
        SR_NODISCARD ProcessedLexem ProcessLexem();
        SR_NODISCARD std::string_view ProcessIdentifier();

        SR_NODISCARD Lexems ParseInternal(SR_UTILS_NS::IAllocator* pAllocator, uint32_t lexemsReserve, std::string_view code, uint16_t fileIndex);

        void Clear();

        void SkipSpaces();
        void SkipComment();

    private:
        bool m_macroLine = false;
        std::string_view m_source;
        uint64_t m_offset = 0;
        uint16_t m_fileIndex = 0;
        uint64_t m_line = 0;
        uint64_t m_position = 0;

        Lexems m_lexems;
        SR_UTILS_NS::IAllocator* m_pAllocator = nullptr;

    };
}

#endif //SR_ENGINE_COMMON_LEXER_H
