//
// Created by Monika on 21.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_COMPILE_CONTEXT_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_COMPILE_CONTEXT_H

#include <Utils/Flux/IR/FluxInstruction.h>

namespace SR_FLUX_NS {
    struct FluxProgram;
    static constexpr uint32_t FluxInvalidNode = SR_UINT32_MAX;
    static constexpr uint32_t FluxInvalidLabel = SR_UINT32_MAX;

    /// Ссылка на значение, произведённое выходным пином узла графа
    struct FluxValueRef {
        FluxRegisterId operand = 0; /// идентификатор операнда в адресном пространстве программы
        uint32_t sourceNode = FluxInvalidNode;
        uint32_t sourcePin = 0;
        uint32_t registerIndex = 0; /// индекс регистра (если значение лежит в регистре)
        uint32_t loopDepth = 0; /// глубина цикла, на которой значение было материализовано
        bool isRegister = false; /// значение занимает регистр и требует освобождения

        SR_NODISCARD bool IsValid() const noexcept { return sourceNode != FluxInvalidNode; }
    };

    /// Состояние распределителя регистров, снимаемое на время компиляции ветви
    struct FluxRegisterSnapshot {
        Vector<bool> availableRegisters;
        Map<uint64_t, FluxValueRef> materialized;
        Map<uint64_t, uint32_t> pendingUses;
        uint32_t deferredReleaseCount = 0;
    };

    struct FluxGraphCompileContext {
        FluxProgram* program = nullptr;
        uint32_t nodeIndex = 0;

        /// true - регистр свободен. Нулевой регистр зарезервирован средой исполнения под результат
        /// вызова и условие ветвления, поэтому он никогда не выделяется под значения графа
        Vector<bool> availableRegisters;

        uint32_t registerBase = 0; /// constants.size() + storage.size()
        uint32_t requiredRegisters = 1; /// максимум регистров, понадобившийся за всю компиляцию
        uint32_t loopDepth = 0;
        uint32_t labelCounter = 0;
        uint32_t terminatorLabel = FluxInvalidLabel; /// чем завершать текущую цепочку потока
        bool flowTerminated = false;
        bool hasErrors = false;

        Map<StringAtom, uint32_t> storageIndices; /// имя переменной -> индекс в storage
        Map<uint64_t, uint32_t> constantIndices; /// ключ значения -> индекс в constants
        Map<uint64_t, uint32_t> dataUseCount; /// ключ значения -> количество потребителей
        Map<uint32_t, uint32_t> flowInputCount; /// узел -> количество входящих flow-связей
        Map<uint32_t, uint32_t> emittedLabels; /// узел слияния потоков -> индекс метки
        Map<uint64_t, FluxValueRef> materialized; /// ключ значения -> уже вычисленное значение
        Map<uint64_t, uint32_t> pendingUses; /// ключ значения -> сколько использований осталось

        Vector<FluxValueRef> deferredReleases; /// освобождения, отложенные до выхода из цикла
        Vector<uint32_t> loopScopeStarts;
        Vector<uint64_t> evaluationStack; /// защита от циклов среди чистых узлов

        SR_NODISCARD uint32_t AllocateRegister();
        void FreeRegister(uint32_t index);

        SR_NODISCARD FluxRegisterId ToOperand(uint32_t registerIndex) const noexcept {
            return static_cast<FluxRegisterId>(registerBase + registerIndex);
        }

        SR_NODISCARD FluxRegisterSnapshot SaveState() const;
        void RestoreState(const FluxRegisterSnapshot& snapshot);

        void ResetExecutionState();
    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_COMPILE_CONTEXT_H
