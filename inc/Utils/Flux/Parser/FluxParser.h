//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_PARSER_H
#define SR_ENGINE_COMMON_FLUX_PARSER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Lexer/LexerUtils.h>
#include <Utils/Flux/IR/FluxOpcode.h>

namespace SR_FLUX_NS {
    struct FluxProgram;

    class FluxParser : public SR_UTILS_NS::Singleton<FluxParser> {
        SR_REGISTER_SINGLETON(FluxParser)
        enum class State : uint8_t {
            None,
            Function,
            Constants
        };
    public:
        SR_NODISCARD bool Parse(StringView source, FluxProgram& program);

    private:
        bool ParseInstruction();
        bool ParseConstantsOrStorage(bool isStorage);
        bool ResolveLabels();

        SR_NODISCARD FluxOpcode ParseOpcode();

        const SR_UTILS_NS::LexerDetails::Lexem& Advance();
        SR_NODISCARD const SR_UTILS_NS::LexerDetails::Lexem& Current() const;
        SR_NODISCARD bool IsEnd() const;
        SR_NODISCARD bool ExpectIdentifier(StringView expected) const;
        SR_NODISCARD uint32_t ParseInteger(bool& isLineEnd);

        void Back();

    private:
        SR_HTYPES_NS::RawPointerHolder<IAllocator> m_allocator;
        FluxProgram* m_program = nullptr;
        uint32_t m_currentLexem = 0;
        Vector<LexerDetails::Lexem> m_lexems;
        State m_state = State::None;
        Vector<StringAtom> m_labels;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_PARSER_H
