//
// Created by Monika on 15.08.2023.
//

#include <Utils/SRLM/LogicalMachine.h>
#include <Utils/SRLM/DataType.h>
#include <Utils/SRLM/LogicalNode.h>
#include <Utils/SRLM/LogicalNodeManager.h>
#include <Utils/Resources/Xml.h>

namespace SR_SRLM_NS {
    LogicalMachine::LogicalMachine() = default;

    LogicalMachine::~LogicalMachine() {
        Clear();
    }

    void LogicalMachine::UpdateMachine(float_t dt) {

    }

    bool LogicalMachine::Init() {
        return true;
    }

    void LogicalMachine::Optimize() {

    }

    void LogicalMachine::AddNode(LogicalNode* pNode) {

    }

    bool LogicalMachine::Load() {
        return Super::Load();
    }

    void LogicalMachine::Clear() {

    }

    bool LogicalMachine::Unload() {
        Clear();
        return Super::Unload();
    }

    bool LogicalMachine::Execute(float_t dt) {
        if (!GetCurrentNode()) {
            return false;
        }

        SRHalt("Unresolved behaviour!");

        return false;
    }

    LogicalNode* LogicalMachine::GetCurrentNode() const {
        return nullptr;
    }

    LogicalNode::NodePin* LogicalMachine::GetCurrentPin() const {
        return nullptr;
    }

    void LogicalMachine::SetCurrentNode(LogicalNode* pNode, LogicalNode::NodePin* pFromPin) {

    }

    bool LogicalMachine::ProcessExecutable(float_t dt) {
        return false;
    }

    bool LogicalMachine::ProcessReset(float_t dt) {

        return true;
    }
}