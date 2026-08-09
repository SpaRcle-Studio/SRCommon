//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/Parser/FluxParser.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Lexer/Lexer.h>
#include <Utils/Types/LockGuard.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Memory/MemoryLiterals.h>
#include <Utils/Common/LexicalCast.h>

namespace SR_FLUX_NS {
    bool FluxParser::Parse(StringView source, FluxProgram& program) {
        SR_TRACY_ZONE;
        SR_LOCK_GUARD;

        program = FluxProgram();

        if (source.empty()) {
            SR_ERROR("FluxParser::Parse() : source is empty!");
            return false;
        }

        if (!m_allocator) {
            m_allocator = (IAllocator*)(new MonotonicAllocator(1_MB));
        }
        m_lexems = Vector<LexerDetails::Lexem>();
        m_allocator->ResetMemory();

        m_lexems = Lexer::Instance().ParseString(m_allocator.Get(), 256, source, 0);
        if (m_lexems.empty()) {
            SR_ERROR("FluxParser::Parse() : failed to parse source!");
            return false;
        }

        program.allocator = (IAllocator*)(new MonotonicAllocator(64_KB));

        program.constants = Vector<FluxVariable>(program.allocator.Get());
        program.storage   = Vector<FluxVariable>(program.allocator.Get());
        program.functions = Vector<FluxFunction>(program.allocator.Get());

        program.constants.reserve(16);
        program.storage.reserve(16);
        program.functions.reserve(16);

        m_program = &program;
        m_currentLexem = 0;
        m_state = State::None;

        ParseConstantsOrStorage(false);
        ParseConstantsOrStorage(true);

        while (!IsEnd()) {
            if (!ParseFunction()) {
                SR_ERROR("FluxParser::Parse() : failed to parse function!");
                return false;
            }
        }

        return true;
    }

    bool FluxParser::ParseFunction() {
        if (!ExpectIdentifier("func")) {
            SR_ERROR("FluxParser::ParseFunction() : expected \"func\" keyword! But got \"{}\"!", Current().value);
            return false;
        }

        [[maybe_unused]] auto funcKeyword = Advance();

        FluxFunction& function = m_program->functions.emplace_back();
        function.name = Advance().value;
        function.instructions = Vector<FluxInstruction>(m_program->allocator.Get());
        function.instructions.reserve(256);

        while (!ExpectIdentifier("endfunc")) {
            auto&& instruction = function.instructions.emplace_back();
            instruction.opcode = ParseOpcode();
            if (instruction.opcode == FluxOpcode::Unknown) {
                SR_ERROR("FluxParser::ParseFunction() : unknown opcode \"{}\"!", Current().value);
                return false;
            }
            instruction.operands = Vector<FluxRegisterId>(m_program->allocator.Get());

            bool hasOperands =
                instruction.opcode == FluxOpcode::Branch ||
                instruction.opcode == FluxOpcode::Push ||
                instruction.opcode == FluxOpcode::Pop;

            if (instruction.opcode == FluxOpcode::Call) {
                instruction.operands.reserve(8);
                hasOperands = true;
                instruction.callable.object = Advance().value;
                [[maybe_unused]] auto dot = Advance();
                instruction.callable.function = Advance().value;
            }

            static constexpr FluxOpcode opcodesWithTwoOperands[4] = {
                FluxOpcode::Copy, FluxOpcode::Move, FluxOpcode::Swap, FluxOpcode::Ref
            };

            if (std::ranges::find(opcodesWithTwoOperands, instruction.opcode) != std::end(opcodesWithTwoOperands)) {
                instruction.operands.reserve(2);
                hasOperands = true;
            }

            while (hasOperands) {
                auto&& operandTypeLexem = Advance();
                const bool isConstant = operandTypeLexem.value == "$";
                const bool isStorage = operandTypeLexem.value == "@";
                const bool isRegister = operandTypeLexem.value == "%";
                if (!isConstant && !isStorage && !isRegister) {
                    Back();
                    break;
                }
                bool isLineEnd = false;
                const auto index = ParseInteger(isLineEnd);

                const uint32_t offset = isRegister ? (m_program->constants.size() + m_program->storage.size()) :
                    (isStorage ? m_program->constants.size() : 0);

                instruction.operands.emplace_back(index + offset);
                if (isLineEnd) {
                    break;
                }
            }
        }
        [[maybe_unused]] auto endFuncKeyword = Advance();
        return true;
    }

    bool FluxParser::ParseConstantsOrStorage(bool isStorage) {
        if (!ExpectIdentifier(isStorage ? "storage" : "consts")) {
            return true;
        }

        [[maybe_unused]] auto constsKeyword = Advance();

        while (!ExpectIdentifier(isStorage ? "endstorage" : "endconsts")) {
            FluxVariable& constant = isStorage ? m_program->storage.emplace_back() : m_program->constants.emplace_back();
            constant.type = String(m_program->allocator.Get());
            constant.value = String(m_program->allocator.Get());
            constant.type.reserve(8);
            constant.value.reserve(8);

            /// syntax: <type> = <value>
            while (Current().kind != LexerDetails::LexemKind::Assign) {
                constant.type += Advance().value;
            }
            auto lexem = Advance(); /// skip '='
            do {
                lexem = Advance();
                constant.value += lexem.value;
            }
            while (!lexem.isLineEnd && !IsEnd());
        }

        [[maybe_unused]] auto endConstsKeyword = Advance();

        return true;
    }

    const LexerDetails::Lexem& FluxParser::Current() const {
        return m_lexems[m_currentLexem];
    }

    const LexerDetails::Lexem& FluxParser::Advance() {
        return m_lexems[m_currentLexem++];
    }

    bool FluxParser::IsEnd() const {
        return m_currentLexem >= m_lexems.size();
    }

    bool FluxParser::ExpectIdentifier(StringView expected) const {
        if (IsEnd()) {
            return false;
        }
        return Current().value == expected;
    }

    FluxOpcode FluxParser::ParseOpcode() {
        auto&& lexem = Advance();
        struct OpcodeMapping {
            StringView name;
            FluxOpcode opcode;
        };
        static constexpr OpcodeMapping opcodeMappings[] = {
            { "cp", FluxOpcode::Copy },
            { "mv", FluxOpcode::Move },
            { "swap", FluxOpcode::Swap },
            { "ref", FluxOpcode::Ref },
            { "call", FluxOpcode::Call },
            { "ret", FluxOpcode::Return },
            { "jmp", FluxOpcode::Jump },
            { "br", FluxOpcode::Branch },
            { "push", FluxOpcode::Push },
            { "pop", FluxOpcode::Pop },
        };
        for (const auto& mapping : opcodeMappings) {
            if (lexem.value == mapping.name) {
                return mapping.opcode;
            }
        }
        return FluxOpcode::Unknown;
    }

    uint32_t FluxParser::ParseInteger(bool& isLineEnd) {
        static String buffer;
        buffer.clear();

        LexerDetails::Lexem lexem;
        do {
            lexem = Advance();
            if (lexem.kind == LexerDetails::LexemKind::Integer) {
                buffer += lexem.value;
            }
            else {
                Back();
                break;
            }
        }
        while (!IsEnd() && !lexem.isLineEnd);
        isLineEnd = lexem.isLineEnd;

        return LexicalCast<FluxRegisterId>(buffer);
    }

    void FluxParser::Back() {
        if (m_currentLexem > 0) {
            --m_currentLexem;
        }
        else {
            SRHalt("FluxParser::Back() : cannot go back, already at the beginning!");
        }
    }
}
