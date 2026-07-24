//
// Created by Monika on 30.08.2023.
//

#include <Utils/SRLM/LogicalNode.h>
#include <Utils/SRLM/DataType.h>
#include <Utils/SRLM/LogicalNodeManager.h>

#include <Enum/LogicalNodeStatus.hpp>

namespace SR_SRLM_NS {
    LogicalNode::~LogicalNode() {
        ClearLogicalNode();
    }

    LogicalNode* LogicalNode::LoadXml(const SR_XML_NS::Node& xmlNode) {
        return nullptr;
    }

    void LogicalNode::SaveXml(SR_XML_NS::Node& xmlNode) {

    }

    void LogicalNode::SetInput(const DataType* pInput, uint32_t index) {

    }

    void LogicalNode::Reset() {

    }

    const DataType* LogicalNode::GetOutput(uint32_t index) {
        if (index >= m_outputs.size()) {
            m_status |= LogicalNodeStatus::OutputRangeError;
            return nullptr;
        }

        return m_outputs.at(index).pData;
    }

    void LogicalNode::MarkDirty() {
        if (GetType() == LogicalNodeType::Compute || GetType() == LogicalNodeType::Connector) {
            for (auto&& pin : m_outputs) {
                for (auto&& connection : pin.connections) {
                    if (!connection.pNode) {
                        continue;
                    }
                    connection.pNode->MarkDirty();
                }
            }
        }
    }

    LogicalNode::NodePin& LogicalNode::AddInputData(DataType* pData, uint64_t hashName) {
        auto&& pin = NodePin();
        pin.pData = pData;
        pin.hashName = hashName;
        return m_inputs.emplace_back(pin);
    }

    LogicalNode::NodePin& LogicalNode::AddOutputData(DataType* pData, uint64_t hashName) {
        auto&& pin = NodePin();
        pin.pData = pData;
        pin.hashName = hashName;
        return m_outputs.emplace_back(pin);
    }

    const DataType* LogicalNode::CalcInput(uint32_t index) {
        auto&& pNode = m_inputs[index].GetFirstNode();
        SRAssert(m_inputs[index].GetFirstNodePin() <= 255);
        return pNode->GetOutput(m_inputs[index].GetFirstNodePin());
    }

    bool LogicalNode::IsSuccessfullyCompleted() const noexcept {
        return (m_status & LogicalNodeStatus::Success) && !HasErrors();
    }

    void LogicalNode::RemoveOutputConnection(LogicalNode* pNode, uint32_t pinIndex) {
        auto&& connections = m_outputs[pinIndex].connections;

        for (auto pIt = connections.begin(); pIt != connections.end(); ) {
            if (pIt->pNode == pNode) {
                connections.erase(pIt);
                return;
            }
            ++pIt;
        }
    }

    void LogicalNode::RemoveInputConnection(LogicalNode* pNode, uint32_t pinIndex) {
        auto&& connections = m_inputs[pinIndex].connections;

        for (auto pIt = connections.begin(); pIt != connections.end(); ) {
            if (pIt->pNode == pNode) {
                connections.erase(pIt);
                return;
            }
            ++pIt;
        }
    }

    void LogicalNode::AddInputConnection(LogicalNode* pNode, uint32_t nodePinIndex, uint32_t pinIndex) {

    }

    void LogicalNode::AddOutputConnection(LogicalNode* pNode, uint32_t nodePinIndex, uint32_t pinIndex) {

    }

    void LogicalNode::ClearLogicalNode() {

    }

    void LogicalNode::RemoveInput(uint32_t index) {

    }

    void LogicalNode::RemoveOutput(uint32_t index) {

    }

    void LogicalNode::ResetInputFlows() {

    }

    void LogicalNode::ResetOutputFlows() {

    }

    void LogicalNode::ResetStatus() {
        m_status = LogicalNodeStatus::None;
    }

    void LogicalNode::Execute(float_t dt) {
        m_status |= LogicalNodeStatus::Success;
    }

    /// ----------------------------------------------------------------------------------------------------------------

    const DataType* IComputeNode::GetOutput(uint32_t index) {
        return LogicalNode::GetOutput(index);
    }

    void IComputeNode::MarkDirty() {
        m_dirty = true;
        LogicalNode::MarkDirty();
    }
}