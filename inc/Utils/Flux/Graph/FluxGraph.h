//
// Created by Monika on 18.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_H

#include <Utils/Reflection/Value.h>
#include <Utils/Flux/IR/FluxInstruction.h>

namespace SR_FLUX_NS {
    struct FluxGraphCompileContext;
    struct FluxValueRef;
    struct FluxProgram;
    class FluxGraphNode;

    /// Ключ значения, которое производит выходной пин узла графа
    SR_MAYBE_UNUSED static constexpr uint64_t MakeFluxValueKey(uint32_t nodeIndex, uint32_t pinIndex) noexcept {
        return (static_cast<uint64_t>(nodeIndex) << 32u) | static_cast<uint64_t>(pinIndex);
    }

    class FluxGraphLink : public Serializable {
        SR_CLASS()
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

        SR_NODISCARD bool operator==(const FluxGraphLink& other) const;
        SR_NODISCARD auto operator!=(const FluxGraphLink& other) const { return !(*this == other); }

    private:
        /// @property
        uint32_t m_sourceNode = 0;
        /// @property
        uint32_t m_targetNode = 0;
        /// @property
        uint32_t m_sourcePin = 0;
        /// @property
        uint32_t m_targetPin = 0;

        void* m_userData = nullptr;
    };

    SR_ENUM_NS_CLASS_T(FluxGraphNodeType, uint8_t,
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

        Cast

        /// TODO:
        /// Sequence,
        /// Synchronize,
    )

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
    ///   Cast          | in:  0 - flow, 1 - объект
    ///                 | out: 0 - flow (успех), 1 - flow (cast failed), 2 - приведённый объект
    class FluxGraphNode : public Serializable {
        SR_CLASS()
    public:
        SR_NODISCARD FluxGraphNodeType GetType() const { return m_type; }
        SR_NODISCARD FluxCallable GetCallable() const;
        SR_NODISCARD StringAtom GetName() const { return m_name; }
        SR_NODISCARD const Reflection::Value& GetConstant() const { return m_constant; }
        SR_NODISCARD Reflection::Value& GetConstantMutable() { return m_constant; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetPosition() const { return m_position; }
        SR_NODISCARD void* GetUserData() const { return m_userData; }

        void SetType(FluxGraphNodeType type) { m_type = type; }
        void SetCallable(const FluxCallable& callable);
        void SetName(const StringAtom& name) { m_name = name; }
        void SetConstant(const Reflection::Value& constant) { m_constant = constant; }
        void SetPosition(const SR_MATH_NS::FVector2& position) { m_position = position; }
        void SetUserData(void* userData) { m_userData = userData; }

        SR_NODISCARD bool operator==(const FluxGraphNode& other) const;

    private:
        /// @property
        FluxGraphNodeType m_type = FluxGraphNodeType::Unknown;
        /// @property @tooltip(for ReadVariable / WriteVariable / Event, target type for Cast)
        StringAtom m_name;
        /// @property
        SR_MATH_NS::FVector2 m_position;
        /// @property
        Reflection::Value m_constant;
        /// @property
        StringAtom m_callableObject;
        /// @property
        StringAtom m_callableFunction;

        void* m_userData = nullptr;

    };

    class FluxGraph : public Serializable {
        SR_CLASS()
    public:
        FluxGraph();
        ~FluxGraph() override;

        FluxGraph(const FluxGraph& other);
        FluxGraph& operator=(const FluxGraph& other);

    public:
        /// @return индекс добавленного узла
        uint32_t AddNode(const FluxGraphNode& node);
        void AddLink(const FluxGraphLink& link);

        /// удаляет узел вместе со связями, индексы оставшихся узлов сдвигаются
        void RemoveNode(uint32_t nodeIndex);
        void RemoveLink(uint32_t sourceNode, uint32_t sourcePin, uint32_t targetNode, uint32_t targetPin);
        void RemoveInputLink(uint32_t nodeIndex, uint32_t pinIndex);
        void RemoveOutputLink(uint32_t nodeIndex, uint32_t pinIndex);

        SR_NODISCARD FluxProgram Compile() const;

        SR_NODISCARD uint32_t GetNodeCount() const { return static_cast<uint32_t>(m_nodes.size()); }
        SR_NODISCARD FluxGraphNode* GetNode(uint32_t nodeIndex);
        SR_NODISCARD const FluxGraphNode* GetNode(uint32_t nodeIndex) const;

        SR_NODISCARD const Vector<FluxGraphNode>& GetNodes() const { return m_nodes; }
        SR_NODISCARD Vector<FluxGraphNode>& GetNodes() { return m_nodes; }
        SR_NODISCARD const Vector<FluxGraphLink>& GetLinks() const { return m_links; }
        SR_NODISCARD Vector<FluxGraphLink>& GetLinks() { return m_links; }
        SR_NODISCARD const Map<StringAtom, Reflection::Value>& GetVariables() const { return m_variables; }
        SR_NODISCARD Map<StringAtom, Reflection::Value>& GetVariables() { return m_variables; }

        SR_NODISCARD const FluxGraphLink* FindInputLink(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD const FluxGraphLink* FindOutputLink(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD uint32_t GetMaxInputPin(uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t GetMaxOutputPin(uint32_t nodeIndex) const;

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
        SR_NODISCARD uint32_t CompileCastNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;

        SR_NODISCARD bool CompileCall(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t objectPin) const;

        SR_NODISCARD FluxValueRef EvaluateOutput(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD FluxValueRef EvaluateCondition(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t pinIndex) const;
        void ReleaseValue(FluxGraphCompileContext& context, const FluxValueRef& value) const;

        void PushLoopScope(FluxGraphCompileContext& context) const;
        void PopLoopScope(FluxGraphCompileContext& context) const;

        SR_NODISCARD uint32_t GetFlowTarget(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD static uint32_t GetUseCount(const FluxGraphCompileContext& context, uint64_t key);
        SR_NODISCARD static uint32_t GetFlowInputCount(const FluxGraphCompileContext& context, uint32_t nodeIndex);

        FluxInstruction& EmitInstruction(FluxGraphCompileContext& context, FluxOpcode opcode, uint32_t debugId) const;
        void EmitBinary(FluxGraphCompileContext& context, FluxOpcode opcode, FluxRegisterId source, FluxRegisterId destination, uint32_t debugId) const;
        void EmitJump(FluxGraphCompileContext& context, FluxOpcode opcode, uint32_t labelIndex, uint32_t debugId) const;
        SR_NODISCARD uint32_t CreateLabel(FluxGraphCompileContext& context, StringView prefix) const;
        void BindLabel(FluxGraphCompileContext& context, uint32_t labelIndex) const;

        SR_NODISCARD uint32_t AddVariable(FluxGraphCompileContext& context, const Reflection::Value& value, bool isStorage) const;

    private:
        /// @property
        Vector<FluxGraphNode> m_nodes;
        /// @property
        Vector<FluxGraphLink> m_links;
        /// @property
        Map<StringAtom, Reflection::Value> m_variables;

        /// объект, предоставляющий арифметику для узла For. Разрешается средой исполнения по имени,
        /// поэтому может быть переопределён без перекомпиляции
        StringAtom m_arithmeticObject;
        StringAtom m_addFunction;
        StringAtom m_lessFunction;

        RawPointerHolder<IAllocator> m_allocator;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_H
