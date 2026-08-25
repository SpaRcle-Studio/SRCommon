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
        program.instructions = Vector<FluxInstruction>(program.allocator.Get());
        program.labels = Vector<FluxLabel>(program.allocator.Get());

        program.constants.reserve(16);
        program.storage.reserve(16);
        program.instructions.reserve(256);
        program.labels.reserve(16);

        m_program = &program;
        m_currentLexem = 0;
        m_state = State::None;

        /// parse registers count
        if (ExpectIdentifier("registers")) {
            Advance(); /// skip 'registers'
            bool isLineEnd = false;
            const auto count = ParseInteger(isLineEnd);
            if (isLineEnd) {
                program.requiredRegisters = count;
            }
            else {
                SR_ERROR("FluxParser::Parse() : expected line end after registers count!");
                return false;
            }
        }

        ParseConstantsOrStorage(false);
        ParseConstantsOrStorage(true);

        while (!IsEnd()) {
            if (!ParseInstruction()) {
                SR_ERROR("FluxParser::Parse() : failed to parse function!");
                return false;
            }
        }

        if (!ResolveLabels()) {
            SR_ERROR("FluxParser::Parse() : failed to resolve labels!");
            return false;
        }

        return true;
    }

    bool FluxParser::ParseInstruction() {
        auto&& labelPossible = Current();
        if (labelPossible.kind == LexerDetails::LexemKind::Identifier && m_currentLexem + 1 < m_lexems.size()) {
            auto&& nextLexem = m_lexems[m_currentLexem + 1];
            if (nextLexem.kind == LexerDetails::LexemKind::Colon) {
                auto&& label = m_program->labels.emplace_back();
                label.name = labelPossible.value;
                label.instructionPointer = m_program->instructions.size();
                Advance(); /// label name
                Advance(); /// ':'
            }
        }

        auto&& instruction = m_program->instructions.emplace_back();
        instruction.opcode = ParseOpcode();
        if (instruction.opcode == FluxOpcode::Unknown) {
            Back();
            SR_ERROR("FluxParser::ParseInstruction() : unknown opcode \"{}\"!", Current().value);
            return false;
        }
        instruction.operands = Vector<FluxRegisterId>(m_program->allocator.Get());

        if (instruction.opcode == FluxOpcode::Jump || instruction.opcode == FluxOpcode::Branch) {
            instruction.operands.reserve(1);
            auto&& labelNameLexem = Advance();
            if (labelNameLexem.kind != LexerDetails::LexemKind::Identifier) {
                SR_ERROR("FluxParser::ParseInstruction() : expected label name after jump opcode!");
                return false;
            }
            auto&& labelName = StringAtom(labelNameLexem.value);
            auto&& pLabelIt = m_labels.find(labelName);
            if (pLabelIt == m_labels.end()) {
                m_labels.emplace_back(labelName);
                pLabelIt = std::prev(m_labels.end());
            }
            instruction.operands.emplace_back(static_cast<FluxRegisterId>(m_labels.distance(pLabelIt)));
            return true;
        }

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

        if (instruction.opcode == FluxOpcode::Cast) {
            /// синтаксис: cast <TargetType> <src> <dst>
            auto&& targetLexem = Advance();
            if (targetLexem.kind != LexerDetails::LexemKind::Identifier) {
                SR_ERROR("FluxParser::ParseInstruction() : expected target type name after cast opcode!");
                return false;
            }
            instruction.callable.object = targetLexem.value;
        }

        static constexpr FluxOpcode opcodesWithTwoOperands[5] = {
            FluxOpcode::Copy, FluxOpcode::Move, FluxOpcode::Swap, FluxOpcode::Ref, FluxOpcode::Cast
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
        return true;
    }

    bool FluxParser::ParseConstantsOrStorage(bool isStorage) {
        if (!ExpectIdentifier(isStorage ? "storage" : "consts")) {
            return true;
        }

        Advance(); /// skip 'consts' or 'storage'

        while (!ExpectIdentifier(isStorage ? "endstorage" : "endconsts")) {
            FluxVariable& constant = isStorage ? m_program->storage.emplace_back() : m_program->constants.emplace_back();
            constant.type = String(m_program->allocator.Get());
            constant.value = String(m_program->allocator.Get());
            constant.type.reserve(8);

            /// syntax: <type> = <value>
            while (Current().kind != LexerDetails::LexemKind::Assign) {
                constant.type += Advance().value;
            }
            auto lexem = Advance(); /// skip '='
            if (lexem.isLineEnd) {
                continue; /// no value assigned, default value
            }
            constant.value.reserve(8);
            do {
                lexem = Advance();
                if (lexem.kind == LexerDetails::LexemKind::String) {
                    /// the lexer cuts quotes off, but the value is used as a json literal, so they have to be restored
                    constant.value += '"';
                    constant.value += lexem.value;
                    constant.value += '"';
                }
                else {
                    constant.value += lexem.value;
                }
            }
            while (!lexem.isLineEnd && !IsEnd());
        }

        Advance(); /// skip 'endconsts' or 'endstorage'

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
        for (const auto& mapping : OPCODE_MAPPINGS) {
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

    bool FluxParser::ResolveLabels() {
        for (auto&& instruction : m_program->instructions) {
            if (instruction.opcode == FluxOpcode::Jump || instruction.opcode == FluxOpcode::Branch) {
                auto&& labelId = instruction.operands[0];
                auto&& labelName = m_labels[labelId];
                auto&& pLabelIt = m_program->labels.find_if([&labelName](const FluxLabel& label) { return label.name == labelName; });
                if (pLabelIt == m_program->labels.end()) {
                    SR_ERROR("FluxParser::ResolveLabels() : label \"{}\" not found!", labelName);
                    return false;
                }
                instruction.operands[0] = static_cast<FluxRegisterId>(m_program->labels.distance(pLabelIt));
            }
        }

        return true;
    }
}
