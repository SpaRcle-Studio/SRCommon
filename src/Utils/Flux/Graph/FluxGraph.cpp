//
// Created by Monika on 18.08.2026.
//

#include <Utils/Flux/Graph/FluxGraph.h>
#include <Utils/Flux/Graph/FluxGraphCompileContext.h>
#include <Utils/Flux/Graph/FluxGraphCompiler.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Flux/Graph/FluxCompileUtils.h>
#include <Utils/Common/Singleton.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Memory/MemoryLiterals.h>
#include <Utils/Reflection/Method.h>
#include <Utils/Reflection/TypeInfoSerialization.h>
#include <Utils/Reflection/VTable.h>
#include <Utils/Serialization/JsonSerialization.h>

#include <Codegen/FluxGraph.generated.hpp>

namespace SR_FLUX_NS {
    /// ================================================== FluxGraph ===================================================

    FluxGraph::FluxGraph()
        : Serializable()
    {
        m_allocator = (IAllocator*)(new UnSynchronizedPoolAllocator());
        m_nodes = Vector<FluxGraphNode>(m_allocator.Get());
        m_links = Vector<FluxGraphLink>(m_allocator.Get());
        m_variables = Map<StringAtom, Reflection::Value>(m_allocator.Get());
    }

    FluxGraph::~FluxGraph() {
        std::exchange(m_nodes, {});
        std::exchange(m_links, {});
        std::exchange(m_variables, {});
    }

    FluxGraph::FluxGraph(const FluxGraph& other)
        : Serializable(other)
        , m_allocator(new UnSynchronizedPoolAllocator())
    {
        m_nodes = Vector<FluxGraphNode>(m_allocator.Get(), other.m_nodes.begin(), other.m_nodes.end());
        m_links = Vector<FluxGraphLink>(m_allocator.Get(), other.m_links.begin(), other.m_links.end());
        m_variables = Map<StringAtom, Reflection::Value>(m_allocator.Get(), other.m_variables);
    }

    FluxGraph& FluxGraph::operator=(const FluxGraph& other) {
        if (this != &other) {
            auto&& pNewAllocator = (IAllocator*)(new UnSynchronizedPoolAllocator());
            m_nodes = Vector<FluxGraphNode>(pNewAllocator, other.m_nodes.begin(), other.m_nodes.end());
            m_links = Vector<FluxGraphLink>(pNewAllocator, other.m_links.begin(), other.m_links.end());
            m_variables = Map<StringAtom, Reflection::Value>(pNewAllocator, other.m_variables);
            m_allocator = pNewAllocator;
        }
        return *this;
    }

    Optional<FluxProgram> FluxGraph::Compile() const {
        return FluxGraphCompiler(*this).Compile();
    }

    /// ============================================== Подготовительные проходы =========================================

    void FluxGraph::CollectStorage(FluxGraphCompileContext& context) const {
        for (auto&& [name, value] : m_variables) {
            const uint32_t index = AddVariable(context, value, true);
            if (index == SR_UINT32_MAX) {
                SR_ERROR("FluxGraph::CollectStorage() : failed to serialize variable \"{}\"!", name);
                context.hasErrors = true;
                return;
            }
            context.storageIndices.emplace(name, index);
        }
    }

    void FluxGraph::CollectConstants(FluxGraphCompileContext& context) const {
        for (uint32_t i = 0; i < m_nodes.size(); ++i) {
            auto&& node = m_nodes[i];

            if (node.GetType() == FluxGraphNodeType::Constant) {
                /// if not linked, the constant is not used and does not need to be serialized
                if (!FindOutputLink(i, 0)) {
                    context.constantIndices.emplace(MakeFluxValueKey(i, 0), SR_UINT32_MAX);
                    continue;
                }

                const uint32_t index = AddVariable(context, node.GetConstant(), false);
                if (index == SR_UINT32_MAX) {
                    SR_ERROR("FluxGraph::CollectConstants() : failed to serialize constant of node {}!", i);
                    context.hasErrors = true;
                    return;
                }
                context.constantIndices.emplace(MakeFluxValueKey(i, 0), index);
                continue;
            }

            /// шаг цикла по умолчанию, если пин шага не подключен
            if (node.GetType() == FluxGraphNodeType::For && !FindInputLink(i, 3)) {
                const uint32_t index = AddVariable(context, Reflection::Value::Create<int32_t>(1), false);
                if (index == SR_UINT32_MAX) {
                    SR_ERROR("FluxGraph::CollectConstants() : failed to create default step of node {}!", i);
                    context.hasErrors = true;
                    return;
                }
                context.constantIndices.emplace(MakeFluxValueKey(i, 3), index);
                continue;
            }

            /// входной пин выходного аргумента разрешается не подключать - тогда метод получает
            /// значение по умолчанию, которое узел тут же публикует изменённым на выходном пине
            if (node.GetType() == FluxGraphNodeType::Invoke || node.GetType() == FluxGraphNodeType::Evaluate) {
                auto&& pMethod = node.GetCallable().FindMethodMeta();
                if (!pMethod) {
                    continue;
                }

                const uint32_t firstArgumentPin = GetFirstArgumentPin(node.GetType(), node.GetCallable());

                for (uint32_t param = 0; param < pMethod->GetParamsCount(); ++param) {
                    const uint32_t pin = firstArgumentPin + param;
                    if (!pMethod->IsOutputParam(param) || FindInputLink(i, pin)) {
                        continue;
                    }

                    const uint32_t index = AddVariable(context, MakeDefaultValue(pMethod->GetParam(param).pTypeInfo), false);
                    if (index == SR_UINT32_MAX) {
                        SR_ERROR("FluxGraph::CollectConstants() : failed to create default value for argument \"{}\" of node {}! Connect the pin explicitly.",
                            pMethod->GetParam(param).name, i);
                        context.hasErrors = true;
                        return;
                    }
                    context.constantIndices.emplace(MakeFluxValueKey(i, pin), index);
                }
            }
        }
    }

    void FluxGraph::CollectUsages(FluxGraphCompileContext& context) const {
        for (auto&& link : m_links) {
            const uint32_t sourceNode = link.GetSourceNode();
            const uint32_t targetNode = link.GetTargetNode();

            if (sourceNode >= m_nodes.size() || targetNode >= m_nodes.size()) {
                SR_ERROR("FluxGraph::CollectUsages() : link references an unknown node!");
                context.hasErrors = true;
                return;
            }

            if (IsDataOutputPin(m_nodes[sourceNode].GetType(), link.GetSourcePin())) {
                ++context.dataUseCount[MakeFluxValueKey(sourceNode, link.GetSourcePin())];
            }
            else {
                ++context.flowInputCount[targetNode];
            }
        }
    }

    /// ==================================================== Связи =====================================================

    const FluxGraphLink* FluxGraph::FindInputLink(const uint32_t nodeIndex, const uint32_t pinIndex) const {
        for (auto&& link : m_links) {
            if (link.GetTargetNode() == nodeIndex && link.GetTargetPin() == pinIndex) {
                return &link;
            }
        }
        return nullptr;
    }

    const FluxGraphLink* FluxGraph::FindOutputLink(const uint32_t nodeIndex, const uint32_t pinIndex) const {
        for (auto&& link : m_links) {
            if (link.GetSourceNode() == nodeIndex && link.GetSourcePin() == pinIndex) {
                return &link;
            }
        }
        return nullptr;
    }

    uint32_t FluxGraph::GetMaxInputPin(const uint32_t nodeIndex) const {
        uint32_t maxPin = 0;
        for (auto&& link : m_links) {
            if (link.GetTargetNode() == nodeIndex) {
                maxPin = SR_MAX(maxPin, link.GetTargetPin());
            }
        }
        return maxPin;
    }

    uint32_t FluxGraph::GetMaxOutputPin(const uint32_t nodeIndex) const {
        uint32_t maxPin = 0;
        for (auto&& link : m_links) {
            if (link.GetSourceNode() == nodeIndex) {
                maxPin = SR_MAX(maxPin, link.GetSourcePin());
            }
        }
        return maxPin;
    }

    uint32_t FluxGraph::AddVariable(FluxGraphCompileContext& context, const Reflection::Value& value, const bool isStorage) const {
        if (!value.IsValid()) {
            return SR_UINT32_MAX;
        }

        String type;
        Reflection::SaveTypeInfo(type, &value.GetTypeInfo());
        if (type.empty()) {
            return SR_UINT32_MAX;
        }

        JsonSerializer serializer;
        serializer.SetPrettyPrint(false);
        Reflection::SerializeValue(value, serializer);
        const String serialized = serializer.ToStringBase();

        auto&& program = *context.program;
        auto&& variables = isStorage ? program.storage : program.constants;

        /// одинаковые константы схлопываются - это уменьшает адресное пространство программы.
        /// Ячейки хранилища схлопывать нельзя, они адресуются по имени переменной
        if (!isStorage) {
            for (uint32_t i = 0; i < variables.size(); ++i) {
                if (variables[i].type == type && variables[i].value == serialized) {
                    return i;
                }
            }
        }

        FluxVariable& variable = variables.emplace_back();
        variable.type = String(program.allocator.Get());
        variable.value = String(program.allocator.Get());
        variable.type += type;
        variable.value += serialized;

        return static_cast<uint32_t>(variables.size() - 1);
    }

    uint32_t FluxGraph::AddNode(const FluxGraphNode& node) {
        m_nodes.emplace_back(node);
        return static_cast<uint32_t>(m_nodes.size() - 1);
    }

    void FluxGraph::AddLink(const FluxGraphLink& link) {
        /// дубликат связи удвоил бы число входящих потоков узла и создал бы метку слияния на ровном месте
        if (m_links.find_if([&link](const FluxGraphLink& other) { return other == link; }) != m_links.end()) {
            return;
        }
        m_links.emplace_back(link);
    }

    void FluxGraph::RemoveNode(const uint32_t nodeIndex) {
        if (nodeIndex >= m_nodes.size()) {
            SRHalt("FluxGraph::RemoveNode() : node index {} is out of range!", nodeIndex);
            return;
        }

        /// связи хранят индексы узлов, поэтому после удаления узла их нужно пересчитать
        for (auto pIt = m_links.begin(); pIt != m_links.end();) {
            if (pIt->GetSourceNode() == nodeIndex || pIt->GetTargetNode() == nodeIndex) {
                pIt = m_links.erase(pIt);
                continue;
            }

            if (pIt->GetSourceNode() > nodeIndex) {
                pIt->SetSourceNode(pIt->GetSourceNode() - 1);
            }
            if (pIt->GetTargetNode() > nodeIndex) {
                pIt->SetTargetNode(pIt->GetTargetNode() - 1);
            }

            ++pIt;
        }

        m_nodes.erase(m_nodes.begin() + nodeIndex);
    }

    void FluxGraph::RemoveLink(const uint32_t sourceNode, const uint32_t sourcePin, const uint32_t targetNode, const uint32_t targetPin) {
        std::erase_if(m_links, [&](const FluxGraphLink& link) {
            return link.GetSourceNode() == sourceNode && link.GetSourcePin() == sourcePin &&
                link.GetTargetNode() == targetNode && link.GetTargetPin() == targetPin;
        });
    }

    void FluxGraph::RemoveInputLink(const uint32_t nodeIndex, const uint32_t pinIndex) {
        std::erase_if(m_links, [&](const FluxGraphLink& link) {
            return link.GetTargetNode() == nodeIndex && link.GetTargetPin() == pinIndex;
        });
    }

    void FluxGraph::RemoveOutputLink(const uint32_t nodeIndex, const uint32_t pinIndex) {
        std::erase_if(m_links, [&](const FluxGraphLink& link) {
            return link.GetSourceNode() == nodeIndex && link.GetSourcePin() == pinIndex;
        });
    }

    FluxGraphNode* FluxGraph::GetNode(const uint32_t nodeIndex) {
        return nodeIndex < m_nodes.size() ? &m_nodes[nodeIndex] : nullptr;
    }

    const FluxGraphNode* FluxGraph::GetNode(const uint32_t nodeIndex) const {
        return nodeIndex < m_nodes.size() ? &m_nodes[nodeIndex] : nullptr;
    }

    uint32_t FluxGraph::GetNodeIndex(const FluxGraphNode* pNode) const {
        if (!pNode) {
            return SR_UINT32_MAX;
        }

        const auto nodeIndex = static_cast<uint32_t>(pNode - m_nodes.data());
        return nodeIndex < m_nodes.size() ? nodeIndex : SR_UINT32_MAX;
    }

    Reflection::Value* FluxGraph::FindVariable(StringAtom name) {
        auto&& pIt = m_variables.find(name);
        return pIt != m_variables.end() ? &pIt->second : nullptr;
    }

    void FluxGraphNode::SetCallable(const FluxCallable& callable) {
        m_callableObject = callable.object;
        m_callableFunction = callable.function;
    }

    FluxCallable FluxGraphNode::GetCallable() const {
        return { m_callableObject, m_callableFunction };
    }

    bool FluxGraphNode::operator==(const FluxGraphNode &other) const {
        return std::tie(m_type, m_name, m_position, m_constant, m_callableObject, m_callableFunction) ==
               std::tie(other.m_type, other.m_name, other.m_position, other.m_constant, other.m_callableObject, other.m_callableFunction);
    }

    bool FluxGraphLink::operator==(const FluxGraphLink &other) const {
        return std::tie(m_sourceNode, m_targetNode, m_sourcePin, m_targetPin) ==
               std::tie(other.m_sourceNode, other.m_targetNode, other.m_sourcePin, other.m_targetPin);
    }
}
