//
// Created by Monika on 18.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_H

#include <Utils/Flux/IR/FluxInstruction.h>
#include <Utils/Reflection/Value.h>

namespace SR_FLUX_NS {
    struct FluxProgram;
    class FluxGraphNode;

    static constexpr uint32_t FluxInvalidNode = SR_UINT32_MAX;
    static constexpr uint32_t FluxInvalidLabel = SR_UINT32_MAX;

    /// Ключ значения, которое производит выходной пин узла графа
    SR_MAYBE_UNUSED static constexpr uint64_t MakeFluxValueKey(uint32_t nodeIndex, uint32_t pinIndex) noexcept {
        return (static_cast<uint64_t>(nodeIndex) << 32u) | static_cast<uint64_t>(pinIndex);
    }

    class FluxGraphLink {
    public:
        SR_NODISCARD uint32_t GetSourceNode() const { return m_sourceNode; }
        SR_NODISCARD uint32_t GetTargetNode() const { return m_targetNode; }
        SR_NODISCARD uint32_t GetSourcePin() const { return m_sourcePin; }
        SR_NODISCARD uint32_t GetTargetPin() const { return m_targetPin; }
        SR_NODISCARD void* GetUserData() const { return m_userData; }

        void SetSourceNode(uint32_t sourceNode) { m_sourceNode = sourceNode; }
        void SetTargetNode(uint32_t targetNode) { m_targetNode = targetNode; }
        void SetSourcePin(uint32_t sourcePin) { m_sourcePin = sourcePin; }
        void SetTargetPin(uint32_t targetPin) { m_targetPin = targetPin; }
        void SetUserData(void* userData) { m_userData = userData; }

    private:
        uint32_t m_sourceNode = 0;
        uint32_t m_targetNode = 0;
        uint32_t m_sourcePin = 0;
        uint32_t m_targetPin = 0;
        void* m_userData = nullptr;
    };

    enum class FluxGraphNodeType : uint8_t {
        Unknown,

        Event,

        Evaluate,
        Invoke,

        Constant,
        ReadVariable,
        WriteVariable,

        Branch,
        For,
        While,

        /// TODO:
        /// Sequence,
        /// Synchronize,
    };

    /// Раскладка пинов узлов. Flow-пин всегда имеет индекс 0 (и на входе, и на выходе).
    /// Узлы Evaluate / Constant / ReadVariable являются чистыми - они не участвуют в потоке
    /// исполнения и вычисляются в точке использования.
    ///
    ///   Event         | out: 0 - flow, 1..N - аргументы события
    ///   Invoke        | in:  0 - flow, 1 - объект (не подключен -> синглтон), 2..N - аргументы
    ///                 | out: 0 - flow, 1 - возвращаемое значение
    ///   Evaluate      | in:  0 - объект (не подключен -> синглтон), 1..N - аргументы
    ///                 | out: 0 - результат вызова
    ///   Constant      | out: 0 - значение
    ///   ReadVariable  | out: 0 - значение
    ///   WriteVariable | in:  0 - flow, 1 - значение          | out: 0 - flow
    ///   Branch        | in:  0 - flow, 1 - условие           | out: 0 - flow (true), 1 - flow (false)
    ///   While         | in:  0 - flow, 1 - условие           | out: 0 - flow (тело), 1 - flow (после цикла)
    ///   For           | in:  0 - flow, 1 - начало, 2 - конец, 3 - шаг (опционально)
    ///                 | out: 0 - flow (тело), 1 - flow (после цикла), 2 - текущий индекс
    class FluxGraphNode {
    public:
        SR_NODISCARD FluxGraphNodeType GetType() const { return m_type; }
        SR_NODISCARD const FluxCallable& GetCallable() const { return m_callable; }
        SR_NODISCARD StringAtom GetName() const { return m_name; }
        SR_NODISCARD const Reflection::Value& GetConstant() const { return m_constant; }
        SR_NODISCARD const String& GetUid() const { return m_uid; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetPosition() const { return m_position; }
        SR_NODISCARD void* GetUserData() const { return m_userData; }

        void SetType(FluxGraphNodeType type) { m_type = type; }
        void SetCallable(const FluxCallable& callable) { m_callable = callable; }
        void SetName(const StringAtom& name) { m_name = name; }
        void SetConstant(const Reflection::Value& constant) { m_constant = constant; }
        void SetUid(const String& uid) { m_uid = uid; }
        void SetPosition(const SR_MATH_NS::FVector2& position) { m_position = position; }
        void SetUserData(void* userData) { m_userData = userData; }

    private:
        FluxGraphNodeType m_type = FluxGraphNodeType::Unknown;
        FluxCallable m_callable;
        StringAtom m_name; /// ReadVariable / WriteVariable / Event
        Reflection::Value m_constant; /// Constant
        String m_uid;
        SR_MATH_NS::FVector2 m_position;
        void* m_userData = nullptr;

    };

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

    class FluxGraph : public NonCopyable {
        using Super = NonCopyable;
    public:
        FluxGraph();
        ~FluxGraph() override;

    public:
        void AddNode(const FluxGraphNode& node);
        void AddLink(const FluxGraphLink& link);

        SR_NODISCARD FluxProgram Compile() const;

    private:
        /// подготовительные проходы, выполняются до генерации инструкций, так как адресное
        /// пространство операндов зависит от количества констант и ячеек хранилища
        void CollectStorage(FluxGraphCompileContext& context) const;
        void CollectConstants(FluxGraphCompileContext& context) const;
        void CollectUsages(FluxGraphCompileContext& context) const;

        void CompileEventNode(FluxGraphCompileContext& context) const;
        void CompileFlow(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t terminatorLabel) const;
        SR_NODISCARD uint32_t CompileNode(FluxGraphCompileContext& context) const;

        SR_NODISCARD uint32_t CompileInvokeNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileWriteVariableNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileBranchNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileWhileNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileForNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;

        SR_NODISCARD bool CompileCall(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t objectPin) const;

        SR_NODISCARD FluxValueRef EvaluateOutput(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD FluxValueRef EvaluateCondition(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t pinIndex) const;
        void ReleaseValue(FluxGraphCompileContext& context, const FluxValueRef& value) const;

        void PushLoopScope(FluxGraphCompileContext& context) const;
        void PopLoopScope(FluxGraphCompileContext& context) const;

        SR_NODISCARD const FluxGraphLink* FindInputLink(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD const FluxGraphLink* FindOutputLink(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD uint32_t GetFlowTarget(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD uint32_t GetMaxInputPin(uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t GetMaxOutputPin(uint32_t nodeIndex) const;
        SR_NODISCARD static uint32_t GetUseCount(const FluxGraphCompileContext& context, uint64_t key);
        SR_NODISCARD static uint32_t GetFlowInputCount(const FluxGraphCompileContext& context, uint32_t nodeIndex);

        FluxInstruction& EmitInstruction(FluxGraphCompileContext& context, FluxOpcode opcode, uint32_t debugId) const;
        void EmitBinary(FluxGraphCompileContext& context, FluxOpcode opcode, FluxRegisterId source, FluxRegisterId destination, uint32_t debugId) const;
        void EmitJump(FluxGraphCompileContext& context, FluxOpcode opcode, uint32_t labelIndex, uint32_t debugId) const;
        SR_NODISCARD uint32_t CreateLabel(FluxGraphCompileContext& context, StringView prefix) const;
        void BindLabel(FluxGraphCompileContext& context, uint32_t labelIndex) const;

        SR_NODISCARD uint32_t AddVariable(FluxGraphCompileContext& context, const Reflection::Value& value, bool isStorage) const;

    private:
        /// объект, предоставляющий арифметику для узла For. Разрешается средой исполнения по имени,
        /// поэтому может быть переопределён без перекомпиляции
        StringAtom m_arithmeticObject;
        StringAtom m_addFunction;
        StringAtom m_lessFunction;

        Map<StringAtom, Reflection::Value> m_variables;
        Vector<FluxGraphNode> m_nodes;
        Vector<FluxGraphLink> m_links;
        RawPointerHolder<IAllocator> m_allocator;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_H
