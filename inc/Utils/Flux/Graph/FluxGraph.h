//
// Created by Monika on 18.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_H

#include <Utils/Reflection/Value.h>
#include <Utils/Reflection/Method.h>
#include <Utils/Flux/IR/FluxInstruction.h>
#include <Utils/Types/Optional.h>

namespace SR_FLUX_NS {
    struct FluxGraphCompileContext;
    struct FluxValueRef;
    struct FluxProgram;
    class FluxGraphNode;

    SR_MAYBE_UNUSED static constexpr uint32_t FluxInvalidPin = SR_UINT32_MAX;

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

        Cast,

        Sequence,
        ParallelSequence

        /// TODO:
        /// Synchronize,
    )

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

        SR_NODISCARD Optional<FluxProgram> Compile() const;

        SR_NODISCARD Reflection::Value* FindVariable(StringAtom name);

        SR_NODISCARD uint32_t GetNodeCount() const { return static_cast<uint32_t>(m_nodes.size()); }
        SR_NODISCARD uint32_t GetNodeIndex(const FluxGraphNode* pNode) const;
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

        /// подготовительные проходы, выполняются до генерации инструкций, так как адресное
        /// пространство операндов зависит от количества констант и ячеек хранилища
        void CollectStorage(FluxGraphCompileContext& context) const;
        void CollectConstants(FluxGraphCompileContext& context) const;
        void CollectUsages(FluxGraphCompileContext& context) const;

        SR_NODISCARD uint32_t AddVariable(FluxGraphCompileContext& context, const Reflection::Value& value, bool isStorage) const;

    private:
        /// @property
        Vector<FluxGraphNode> m_nodes;
        /// @property
        Vector<FluxGraphLink> m_links;
        /// @property
        Map<StringAtom, Reflection::Value> m_variables;

        RawPointerHolder<IAllocator> m_allocator;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_H
