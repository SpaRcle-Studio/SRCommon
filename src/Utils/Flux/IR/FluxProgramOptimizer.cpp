//
// Created by Monika on 03.09.2026.
//

#include <Utils/Flux/IR/FluxProgramOptimizer.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Reflection/Method.h>

namespace SR_FLUX_NS {
    namespace {
        /// сколько первых операндов инструкции являются ссылками на метки, а не на регистры
        SR_NODISCARD uint32_t GetLabelOperandCount(const FluxInstruction& instruction) noexcept {
            switch (instruction.opcode) {
                case FluxOpcode::Jump:
                case FluxOpcode::Branch:
                    return 1;
                case FluxOpcode::Fork:
                    return static_cast<uint32_t>(instruction.operands.size());
                default:
                    return 0;
            }
        }

        /// после такой инструкции исполнение не обязано перейти к следующей по счёту
        SR_NODISCARD bool IsControlFlowInstruction(const FluxInstruction& instruction) noexcept {
            switch (instruction.opcode) {
                case FluxOpcode::Jump:
                case FluxOpcode::Branch:
                case FluxOpcode::Fork:
                case FluxOpcode::Return:
                    return true;
                default:
                    return false;
            }
        }

        SR_NODISCARD bool HasValidArity(const FluxInstruction& instruction) noexcept {
            const auto operandCount = instruction.operands.size();
            switch (instruction.opcode) {
                case FluxOpcode::Copy:
                case FluxOpcode::Move:
                case FluxOpcode::Swap:
                case FluxOpcode::Ref:
                case FluxOpcode::Cast:
                    return operandCount == 2;
                case FluxOpcode::Push:
                case FluxOpcode::Pop:
                case FluxOpcode::Jump:
                case FluxOpcode::Branch:
                    return operandCount == 1;
                case FluxOpcode::Fork:
                    return operandCount >= 1;
                case FluxOpcode::Call:
                case FluxOpcode::Return:
                    return true;
                default:
                    return false;
            }
        }

        /// раскладка операндов вызова: без сигнатуры невозможно отличить обычный аргумент от
        /// выходного, который метод имеет право перезаписать, поэтому неизвестный вызов считается
        /// изменяющим все свои операнды
        struct FluxCallLayout {
            const Reflection::Method* pMethod = nullptr;
            /// нулевой операнд занят объектом вызова, если метод вызывается не у синглтона
            uint32_t argumentOffset = 0;
            bool isKnown = false;
            bool hasReturn = false;
        };

        SR_NODISCARD FluxCallLayout MakeCallLayout(const FluxInstruction& instruction) {
            FluxCallLayout layout;

            layout.pMethod = instruction.callable.FindMethodMeta();
            if (!layout.pMethod) {
                return layout;
            }

            const auto paramCount = static_cast<SizeType>(layout.pMethod->GetParamsCount());
            const auto operandCount = instruction.operands.size();

            if (operandCount == paramCount) {
                layout.argumentOffset = 0;
            }
            else if (operandCount == paramCount + 1) {
                layout.argumentOffset = 1;
            }
            else {
                /// сигнатура не совпала с инструкцией - доверять ей нельзя
                return layout;
            }

            layout.isKnown = true;
            layout.hasReturn = layout.pMethod->HasReturn();

            return layout;
        }

        /// анализ потока данных программы: живость регистров в каждой точке, а также сведения
        /// о том, что инструкция читает и что перезаписывает
        class FluxDataFlow {
        public:
            explicit FluxDataFlow(const FluxProgram& program)
                : m_program(program)
            { }

            /// false, если программа некорректна - такую оптимизировать нельзя
            SR_NODISCARD bool Build();

            SR_NODISCARD FluxRegisterId GetResultRegister() const noexcept { return m_registerBase; }

            SR_NODISCARD bool IsRegister(FluxRegisterId slot) const noexcept {
                return slot >= m_registerBase && slot < m_registerBase + m_registerCount;
            }

            /// регистр участвует в ref и может быть псевдонимом другого регистра
            SR_NODISCARD bool IsAliased(FluxRegisterId slot) const noexcept {
                return !IsRegister(slot) || m_aliased[slot - m_registerBase] != 0;
            }

            SR_NODISCARD bool IsLabelTarget(uint32_t index) const noexcept { return m_labelTargets[index] != 0; }

            SR_NODISCARD bool IsLiveIn(uint32_t index, FluxRegisterId slot) const { return TestSlot(m_liveIn, index, slot); }
            SR_NODISCARD bool IsLiveOut(uint32_t index, FluxRegisterId slot) const { return TestSlot(m_liveOut, index, slot); }

            /// операнд является выходным аргументом вызова - метод пишет в него результат
            SR_NODISCARD bool IsOutputOperand(uint32_t index, uint32_t operandIndex) const;
            SR_NODISCARD bool HasOutputOperand(uint32_t index, FluxRegisterId slot) const;

            /// можно ли подменить операнд другим регистром: подмена допустима только там,
            /// где операнд лишь читается
            SR_NODISCARD bool IsReadOnlyOperand(uint32_t index, uint32_t operandIndex) const;

            /// инструкция читает содержимое регистра
            SR_NODISCARD bool Uses(uint32_t index, FluxRegisterId slot) const;
            /// инструкция может изменить содержимое регистра
            SR_NODISCARD bool MayModify(uint32_t index, FluxRegisterId slot) const;
            /// инструкция гарантированно перезаписывает регистр целиком - прежнее значение теряется
            SR_NODISCARD bool MustDefine(uint32_t index, FluxRegisterId slot) const;

        private:
            template<typename Fn> void ForEachSuccessor(uint32_t index, Fn&& fn) const;
            template<typename Fn> void ForEachUse(uint32_t index, Fn&& fn) const;
            template<typename Fn> void ForEachMustDefine(uint32_t index, Fn&& fn) const;
            template<typename Fn> void ForEachMayModify(uint32_t index, Fn&& fn) const;

            void BuildLiveness();

            SR_NODISCARD bool TestSlot(const Vector<uint64_t>& bits, uint32_t index, FluxRegisterId slot) const {
                /// константы и хранилище живут всё исполнение и в анализе не участвуют
                if (!IsRegister(slot)) {
                    return false;
                }
                const uint32_t bit = slot - m_registerBase;
                return (bits[static_cast<SizeType>(index) * m_stride + (bit >> 6u)] & (1ull << (bit & 63u))) != 0;
            }

            void SetSlot(uint64_t* pWords, FluxRegisterId slot) const noexcept {
                if (!IsRegister(slot)) {
                    return;
                }
                const uint32_t bit = slot - m_registerBase;
                pWords[bit >> 6u] |= 1ull << (bit & 63u);
            }

            void ResetSlot(uint64_t* pWords, FluxRegisterId slot) const noexcept {
                if (!IsRegister(slot)) {
                    return;
                }
                const uint32_t bit = slot - m_registerBase;
                pWords[bit >> 6u] &= ~(1ull << (bit & 63u));
            }

        private:
            const FluxProgram& m_program;

            uint32_t m_instructionCount = 0;
            uint32_t m_registerBase = 0;
            uint32_t m_registerCount = 0;
            uint32_t m_stride = 0;

            Vector<FluxCallLayout> m_callLayouts;
            Vector<uint8_t> m_labelTargets;
            Vector<uint8_t> m_aliased;
            Vector<uint64_t> m_liveIn;
            Vector<uint64_t> m_liveOut;

        };

        bool FluxDataFlow::Build() {
            SR_TRACY_ZONE;

            m_instructionCount = static_cast<uint32_t>(m_program.instructions.size());
            m_registerBase = static_cast<uint32_t>(m_program.constants.size() + m_program.storage.size());
            m_registerCount = m_program.requiredRegisters;

            if (m_instructionCount == 0 || m_registerCount == 0) {
                return false;
            }

            m_stride = (m_registerCount + 63u) / 64u;

            m_labelTargets.clear();
            m_labelTargets.resize(m_instructionCount, 0);

            for (auto&& label : m_program.labels) {
                if (label.instructionPointer > m_instructionCount) {
                    return false;
                }
                if (label.instructionPointer < m_instructionCount) {
                    m_labelTargets[label.instructionPointer] = 1;
                }
            }

            m_aliased.clear();
            m_aliased.resize(m_registerCount, 0);

            m_callLayouts.clear();
            m_callLayouts.resize(m_instructionCount);

            for (uint32_t index = 0; index < m_instructionCount; ++index) {
                const FluxInstruction& instruction = m_program.instructions[index];

                if (!HasValidArity(instruction)) {
                    return false;
                }

                const uint32_t labelOperandCount = GetLabelOperandCount(instruction);
                for (uint32_t operandIndex = 0; operandIndex < instruction.operands.size(); ++operandIndex) {
                    const FluxRegisterId operand = instruction.operands[operandIndex];
                    if (operandIndex < labelOperandCount) {
                        if (operand >= m_program.labels.size()) {
                            return false;
                        }
                    }
                    else if (operand >= m_registerBase + m_registerCount) {
                        return false;
                    }
                }

                if (instruction.opcode == FluxOpcode::Call) {
                    m_callLayouts[index] = MakeCallLayout(instruction);
                }
                /// ref делает приёмник псевдонимом источника: запись в один регистр меняет значение
                /// другого, поэтому оба исключаются из оптимизации
                else if (instruction.opcode == FluxOpcode::Ref) {
                    for (auto&& operand : instruction.operands) {
                        if (IsRegister(operand)) {
                            m_aliased[operand - m_registerBase] = 1;
                        }
                    }
                }
            }

            BuildLiveness();

            return true;
        }

        void FluxDataFlow::BuildLiveness() {
            SR_TRACY_ZONE;

            const auto wordCount = static_cast<SizeType>(m_instructionCount) * m_stride;

            m_liveIn.clear();
            m_liveIn.resize(wordCount, 0);
            m_liveOut.clear();
            m_liveOut.resize(wordCount, 0);

            Vector<uint64_t> buffer;
            buffer.resize(m_stride, 0);

            /// liveOut(i) = объединение liveIn всех преемников, liveIn(i) = чтения инструкции
            /// плюс то, что остаётся живым после неё и ей не перезаписывается
            bool isChanged = true;
            while (isChanged) {
                isChanged = false;

                for (uint32_t offset = 0; offset < m_instructionCount; ++offset) {
                    const uint32_t index = m_instructionCount - 1 - offset;

                    for (uint32_t word = 0; word < m_stride; ++word) {
                        buffer[word] = 0;
                    }

                    ForEachSuccessor(index, [&](const uint32_t successor) {
                        const uint64_t* pSuccessorLiveIn = &m_liveIn[static_cast<SizeType>(successor) * m_stride];
                        for (uint32_t word = 0; word < m_stride; ++word) {
                            buffer[word] |= pSuccessorLiveIn[word];
                        }
                    });

                    uint64_t* pLiveOut = &m_liveOut[static_cast<SizeType>(index) * m_stride];
                    for (uint32_t word = 0; word < m_stride; ++word) {
                        if (pLiveOut[word] != buffer[word]) {
                            pLiveOut[word] = buffer[word];
                            isChanged = true;
                        }
                    }

                    ForEachMustDefine(index, [&](const FluxRegisterId slot) { ResetSlot(buffer.data(), slot); });
                    ForEachUse(index, [&](const FluxRegisterId slot) { SetSlot(buffer.data(), slot); });

                    uint64_t* pLiveIn = &m_liveIn[static_cast<SizeType>(index) * m_stride];
                    for (uint32_t word = 0; word < m_stride; ++word) {
                        if (pLiveIn[word] != buffer[word]) {
                            pLiveIn[word] = buffer[word];
                            isChanged = true;
                        }
                    }
                }
            }
        }

        template<typename Fn> void FluxDataFlow::ForEachSuccessor(const uint32_t index, Fn&& fn) const {
            const FluxInstruction& instruction = m_program.instructions[index];

            const auto visitLabel = [&](const FluxRegisterId labelIndex) {
                const uint32_t pointer = m_program.labels[labelIndex].instructionPointer;
                if (pointer < m_instructionCount) {
                    fn(pointer);
                }
            };

            const auto visitNext = [&] {
                if (index + 1 < m_instructionCount) {
                    fn(index + 1);
                }
            };

            switch (instruction.opcode) {
                case FluxOpcode::Jump:
                    visitLabel(instruction.operands[0]);
                    break;
                case FluxOpcode::Branch:
                    visitLabel(instruction.operands[0]);
                    visitNext();
                    break;
                case FluxOpcode::Fork:
                    /// потомки продолжают исполнение с копией текущих регистров
                    for (auto&& labelIndex : instruction.operands) {
                        visitLabel(labelIndex);
                    }
                    visitNext();
                    break;
                case FluxOpcode::Return:
                    /// стек вызовов заполнять нечем - ret завершает исполнение
                    break;
                default:
                    visitNext();
                    break;
            }
        }

        template<typename Fn> void FluxDataFlow::ForEachUse(const uint32_t index, Fn&& fn) const {
            const FluxInstruction& instruction = m_program.instructions[index];

            switch (instruction.opcode) {
                case FluxOpcode::Copy:
                case FluxOpcode::Move:
                case FluxOpcode::Ref:
                case FluxOpcode::Cast:
                case FluxOpcode::Push:
                    fn(instruction.operands[0]);
                    break;
                case FluxOpcode::Swap:
                    fn(instruction.operands[0]);
                    fn(instruction.operands[1]);
                    break;
                case FluxOpcode::Branch:
                    /// условие читается из нулевого регистра
                    fn(GetResultRegister());
                    break;
                case FluxOpcode::Call:
                    /// выходной аргумент тоже считается читаемым: метод получает ссылку и может
                    /// прочитать её до записи
                    for (auto&& operand : instruction.operands) {
                        fn(operand);
                    }
                    break;
                default:
                    break;
            }
        }

        template<typename Fn> void FluxDataFlow::ForEachMustDefine(const uint32_t index, Fn&& fn) const {
            const FluxInstruction& instruction = m_program.instructions[index];

            switch (instruction.opcode) {
                case FluxOpcode::Copy:
                case FluxOpcode::Ref:
                    fn(instruction.operands[1]);
                    break;
                case FluxOpcode::Move:
                    /// перемещение оставляет источник пустым
                    fn(instruction.operands[0]);
                    fn(instruction.operands[1]);
                    break;
                case FluxOpcode::Swap:
                    fn(instruction.operands[0]);
                    fn(instruction.operands[1]);
                    break;
                case FluxOpcode::Cast:
                    fn(instruction.operands[1]);
                    /// признак успешного приведения кладётся в нулевой регистр
                    fn(GetResultRegister());
                    break;
                case FluxOpcode::Pop:
                    fn(instruction.operands[0]);
                    break;
                case FluxOpcode::Call: {
                    const FluxCallLayout& layout = m_callLayouts[index];
                    /// про вызов с неизвестной сигнатурой нельзя утверждать, что он что-то
                    /// перезаписывает целиком
                    if (!layout.isKnown) {
                        break;
                    }
                    if (layout.hasReturn) {
                        fn(GetResultRegister());
                    }
                    for (uint32_t operandIndex = layout.argumentOffset; operandIndex < instruction.operands.size(); ++operandIndex) {
                        if (layout.pMethod->IsOutputParam(operandIndex - layout.argumentOffset)) {
                            fn(instruction.operands[operandIndex]);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        template<typename Fn> void FluxDataFlow::ForEachMayModify(const uint32_t index, Fn&& fn) const {
            const FluxInstruction& instruction = m_program.instructions[index];

            ForEachMustDefine(index, fn);

            if (instruction.opcode == FluxOpcode::Call && !m_callLayouts[index].isKnown) {
                fn(GetResultRegister());
                for (auto&& operand : instruction.operands) {
                    fn(operand);
                }
            }
        }

        bool FluxDataFlow::IsOutputOperand(const uint32_t index, const uint32_t operandIndex) const {
            const FluxInstruction& instruction = m_program.instructions[index];
            if (instruction.opcode != FluxOpcode::Call) {
                return false;
            }

            const FluxCallLayout& layout = m_callLayouts[index];
            if (!layout.isKnown) {
                return true;
            }
            if (operandIndex < layout.argumentOffset) {
                return false;
            }

            return layout.pMethod->IsOutputParam(operandIndex - layout.argumentOffset);
        }

        bool FluxDataFlow::HasOutputOperand(const uint32_t index, const FluxRegisterId slot) const {
            const FluxInstruction& instruction = m_program.instructions[index];
            for (uint32_t operandIndex = 0; operandIndex < instruction.operands.size(); ++operandIndex) {
                if (instruction.operands[operandIndex] == slot && IsOutputOperand(index, operandIndex)) {
                    return true;
                }
            }
            return false;
        }

        bool FluxDataFlow::IsReadOnlyOperand(const uint32_t index, const uint32_t operandIndex) const {
            const FluxInstruction& instruction = m_program.instructions[index];

            switch (instruction.opcode) {
                case FluxOpcode::Copy:
                case FluxOpcode::Cast:
                case FluxOpcode::Push:
                /// источник перемещения читается и обнуляется, но подмену это не ломает:
                /// подменяемый регистр и так переставал быть нужен
                case FluxOpcode::Move:
                    return operandIndex == 0;
                case FluxOpcode::Call:
                    return !IsOutputOperand(index, operandIndex);
                default:
                    /// swap меняет операнды местами, pop и ref пишут в приёмник, а операнды
                    /// переходов являются метками
                    return false;
            }
        }

        bool FluxDataFlow::Uses(const uint32_t index, const FluxRegisterId slot) const {
            bool result = false;
            ForEachUse(index, [&](const FluxRegisterId used) { result |= used == slot; });
            return result;
        }

        bool FluxDataFlow::MayModify(const uint32_t index, const FluxRegisterId slot) const {
            bool result = false;
            ForEachMayModify(index, [&](const FluxRegisterId modified) { result |= modified == slot; });
            return result;
        }

        bool FluxDataFlow::MustDefine(const uint32_t index, const FluxRegisterId slot) const {
            bool result = false;
            ForEachMustDefine(index, [&](const FluxRegisterId defined) { result |= defined == slot; });
            return result;
        }

        void RemoveInstruction(FluxProgram& program, const uint32_t index) {
            program.instructions.erase(program.instructions.begin() + index);
            /// метка, указывавшая на удалённую инструкцию, начинает указывать на следующую
            for (auto&& label : program.labels) {
                if (label.instructionPointer > index) {
                    --label.instructionPointer;
                }
            }
        }

        /// mv %1 %1 и cp %1 %1 не делают ничего
        SR_NODISCARD bool IsSelfAssignment(const FluxInstruction& instruction) noexcept {
            switch (instruction.opcode) {
                case FluxOpcode::Copy:
                case FluxOpcode::Move:
                case FluxOpcode::Swap:
                    return instruction.operands[0] == instruction.operands[1];
                default:
                    return false;
            }
        }

        /// пересылка в регистр, который больше никто не прочитает
        SR_NODISCARD bool IsDeadAssignment(const FluxDataFlow& dataFlow, const FluxInstruction& instruction, const uint32_t index) {
            if (instruction.opcode != FluxOpcode::Copy && instruction.opcode != FluxOpcode::Move) {
                return false;
            }
            const FluxRegisterId destination = instruction.operands[1];
            /// запись в хранилище видна снаружи программы - её убирать нельзя
            if (!dataFlow.IsRegister(destination) || dataFlow.IsAliased(destination)) {
                return false;
            }
            return !dataFlow.IsLiveOut(index, destination);
        }

        /// Убирает mv src dst, заменяя последующие чтения dst на src. Подмена возможна, пока
        /// исполнение идёт линейно, src не изменён, а dst после этого больше никому не нужен.
        /// Инструкция при этом не удаляется - её удаляет вызывающий код
        SR_NODISCARD bool TryPropagateMove(FluxProgram& program, const FluxDataFlow& dataFlow, const uint32_t index) {
            const FluxRegisterId source = program.instructions[index].operands[0];
            const FluxRegisterId destination = program.instructions[index].operands[1];

            if (source == destination) {
                return false;
            }
            /// подменять можно только регистры: константу нельзя переместить, а хранилище видно
            /// снаружи программы
            if (!dataFlow.IsRegister(source) || !dataFlow.IsRegister(destination)) {
                return false;
            }
            if (dataFlow.IsAliased(source) || dataFlow.IsAliased(destination)) {
                return false;
            }

            const auto instructionCount = static_cast<uint32_t>(program.instructions.size());

            Vector<uint32_t> rewritten;
            bool isRemovable = true;

            uint32_t scanned = index + 1;
            for (; scanned < instructionCount; ++scanned) {
                const FluxInstruction& instruction = program.instructions[scanned];

                /// в точку входа метки можно попасть по другому пути, где подмены не было,
                /// а после инструкции перехода исполнение уходит из линейного участка
                if (dataFlow.IsLabelTarget(scanned) || IsControlFlowInstruction(instruction)) {
                    isRemovable = !dataFlow.IsLiveIn(scanned, destination);
                    break;
                }

                bool usesDestination = false;
                bool isBlocked = false;

                for (uint32_t operandIndex = 0; operandIndex < instruction.operands.size(); ++operandIndex) {
                    if (instruction.operands[operandIndex] != destination) {
                        continue;
                    }
                    usesDestination = true;
                    if (!dataFlow.IsReadOnlyOperand(scanned, operandIndex)) {
                        isBlocked = true;
                        break;
                    }
                }

                /// порядок чтения и записи аргументов внутри метода неизвестен: если источник
                /// является выходным аргументом, метод может перезаписать его до чтения подменённого
                if (usesDestination && !isBlocked && dataFlow.HasOutputOperand(scanned, source)) {
                    isBlocked = true;
                }

                if (isBlocked) {
                    isRemovable = !dataFlow.IsLiveIn(scanned, destination);
                    break;
                }

                if (usesDestination) {
                    rewritten.emplace_back(scanned);
                }

                const bool isSourceModified = dataFlow.MayModify(scanned, source);
                const bool isDestinationRedefined = dataFlow.MustDefine(scanned, destination);

                if (isSourceModified || isDestinationRedefined) {
                    /// операнды читаются до записи результата, поэтому подмена в самой инструкции
                    /// ещё корректна, а дальше источник уже не хранит нужное значение
                    isRemovable = isDestinationRedefined || !dataFlow.IsLiveOut(scanned, destination);
                    break;
                }
            }

            if (!isRemovable) {
                return false;
            }

            for (auto&& rewrittenIndex : rewritten) {
                for (auto&& operand : program.instructions[rewrittenIndex].operands) {
                    if (operand == destination) {
                        operand = source;
                    }
                }
            }

            return true;
        }

        /// Схлопывает пару "mv src dst" и "cp dst src" в одну копию: значение нужно сразу в двух
        /// регистрах, но перемещение опустошает источник, и его приходится наполнять обратно.
        /// Такое встречается там, где результат вызова читается и веткой br из нулевого регистра,
        /// и кем-то ещё позже. Инструкцию по индексу превращает в копию, а копию обратно удаляет
        SR_NODISCARD bool TryFoldMoveWithCopyBack(FluxProgram& program, const FluxDataFlow& dataFlow, const uint32_t index) {
            const FluxRegisterId source = program.instructions[index].operands[0];
            const FluxRegisterId destination = program.instructions[index].operands[1];

            if (source == destination) {
                return false;
            }
            if (!dataFlow.IsRegister(source) || !dataFlow.IsRegister(destination)) {
                return false;
            }
            if (dataFlow.IsAliased(source) || dataFlow.IsAliased(destination)) {
                return false;
            }

            const auto instructionCount = static_cast<uint32_t>(program.instructions.size());

            for (uint32_t scanned = index + 1; scanned < instructionCount; ++scanned) {
                const FluxInstruction& instruction = program.instructions[scanned];

                /// в точку входа метки можно попасть по другому пути, где источник не наполнялся,
                /// а после инструкции перехода исполнение уходит из линейного участка
                if (dataFlow.IsLabelTarget(scanned) || IsControlFlowInstruction(instruction)) {
                    return false;
                }

                if (instruction.opcode == FluxOpcode::Copy &&
                    instruction.operands[0] == destination && instruction.operands[1] == source
                ) {
                    program.instructions[index].opcode = FluxOpcode::Copy;
                    RemoveInstruction(program, scanned);
                    return true;
                }

                /// до возврата значения оба регистра должны остаться нетронутыми, а опустевший
                /// после перемещения источник - непрочитанным
                if (dataFlow.Uses(scanned, source) || dataFlow.MayModify(scanned, source) ||
                    dataFlow.MayModify(scanned, destination)
                ) {
                    return false;
                }
            }

            return false;
        }
    }

    /**
    example of useless operations:
        call FluxUtils.GetActiveFluxComponent
        mv %0 %1 <-- this
        call FluxComponent.GetGameObject %1
        mv %0 %1 <-- this
        call GameObject.GetTransform %1
        mv %0 %1 <-- NOT this
        call Input.GetMouseDrag
        mv %0 %3 <-- this
        call MathUtils.GetFVector2X %3
        mv %0 %4 <-- this
        call MathUtils.MultiplyFloat %4 @12
        mv %0 %4 <-- this
        call Transform.RotateXYZ %1 @11 %4 @11

        mv %0 %2 <-- this
        cp %2 %0 <-- this
        br branch_true_15

        call MathUtils.SubtractFloat $0 %5
        mv %0 %5 <-- this
        cp %5 $0 <-- NOT this
    */
    bool RemoveUselessMoveAndCopyOperations(FluxProgram& program) {
        SR_TRACY_ZONE;

        bool isOptimized = false;
        bool isChanged = true;

        while (isChanged) {
            isChanged = false;

            FluxDataFlow dataFlow(program);
            if (!dataFlow.Build()) {
                break;
            }

            for (uint32_t index = 0; index < program.instructions.size(); ++index) {
                const FluxInstruction& instruction = program.instructions[index];

                const bool isTransfer = instruction.opcode == FluxOpcode::Copy ||
                    instruction.opcode == FluxOpcode::Move ||
                    instruction.opcode == FluxOpcode::Swap;

                if (!isTransfer) {
                    continue;
                }

                const bool isUseless = IsSelfAssignment(instruction) ||
                    IsDeadAssignment(dataFlow, instruction, index) ||
                    (instruction.opcode == FluxOpcode::Move && TryPropagateMove(program, dataFlow, index));

                if (isUseless) {
                    RemoveInstruction(program, index);
                }
                /// значение нужно в обоих регистрах - от пары остаётся одна копия
                else if (instruction.opcode != FluxOpcode::Move || !TryFoldMoveWithCopyBack(program, dataFlow, index)) {
                    continue;
                }

                /// подмена операндов и удаление инструкции сделали анализ неактуальным
                isOptimized = true;
                isChanged = true;
                break;
            }
        }

        return isOptimized;
    }

    bool OptimizeProgram(FluxProgram& program) {
        SR_TRACY_ZONE;
        return RemoveUselessMoveAndCopyOperations(program);
    }
}
