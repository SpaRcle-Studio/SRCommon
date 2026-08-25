//
// Created by Monika on 21.08.2026.
//

#include <Utils/Flux/Graph/FluxGraphCompileContext.h>

namespace SR_FLUX_NS {
    uint32_t FluxGraphCompileContext::AllocateRegister() {
        /// нулевой регистр зарезервирован, поиск начинается с первого
        for (uint32_t i = 1; i < availableRegisters.size(); ++i) {
            if (availableRegisters[i]) {
                availableRegisters[i] = false;
                return i;
            }
        }

        availableRegisters.emplace_back(false);

        const auto index = static_cast<uint32_t>(availableRegisters.size() - 1);
        requiredRegisters = SR_MAX(requiredRegisters, index + 1);

        return index;
    }

    void FluxGraphCompileContext::FreeRegister(const uint32_t index) {
        if (index == 0 || index >= availableRegisters.size()) {
            return;
        }
        availableRegisters[index] = true;
    }

    void FluxGraphCompileContext::EnterFlowSplit() {
        flowSplitStarts.emplace_back(static_cast<uint32_t>(flowSplitKeys.size()));
        for (auto&& [key, value] : materialized) {
            flowSplitKeys.emplace_back(key);
        }
    }

    void FluxGraphCompileContext::LeaveFlowSplit() {
        if (flowSplitStarts.empty()) {
            return;
        }
        flowSplitKeys.resize(flowSplitStarts.back());
        flowSplitStarts.pop_back();
    }

    void FluxGraphCompileContext::PruneToFlowSplitScope() {
        if (flowSplitStarts.empty()) {
            return;
        }

        const uint32_t start = flowSplitStarts.back();

        Vector<uint64_t> expired;
        for (auto&& [key, value] : materialized) {
            bool isAlive = false;
            for (uint32_t i = start; i < flowSplitKeys.size(); ++i) {
                if (flowSplitKeys[i] == key) {
                    isAlive = true;
                    break;
                }
            }
            if (!isAlive) {
                expired.emplace_back(key);
            }
        }

        for (auto&& key : expired) {
            if (auto&& pIt = materialized.find(key); pIt != materialized.end()) {
                if (pIt->second.isRegister) {
                    FreeRegister(pIt->second.registerIndex);
                }
                materialized.erase(key);
            }
            pendingUses.erase(key);
        }
    }

    FluxRegisterSnapshot FluxGraphCompileContext::SaveState() const {
        FluxRegisterSnapshot snapshot;
        snapshot.availableRegisters = availableRegisters;
        snapshot.materialized = materialized;
        snapshot.pendingUses = pendingUses;
        snapshot.deferredReleaseCount = static_cast<uint32_t>(deferredReleases.size());
        return snapshot;
    }

    void FluxGraphCompileContext::RestoreState(const FluxRegisterSnapshot& snapshot) {
        /// requiredRegisters намеренно не откатывается - это отметка максимума за всю компиляцию
        availableRegisters = snapshot.availableRegisters;
        materialized = snapshot.materialized;
        pendingUses = snapshot.pendingUses;

        /// вместе со счётчиками использований откатываются и отложенные освобождения ветви,
        /// иначе они были бы применены к восстановленным счётчикам повторно
        if (deferredReleases.size() > snapshot.deferredReleaseCount) {
            deferredReleases.resize(snapshot.deferredReleaseCount);
        }
    }

    void FluxGraphCompileContext::ResetExecutionState() {
        availableRegisters.clear();
        availableRegisters.emplace_back(false); /// нулевой регистр всегда занят
        materialized.clear();
        pendingUses.clear();
        emittedLabels.clear();
        deferredReleases.clear();
        loopScopeStarts.clear();
        evaluationStack.clear();
        flowSplitKeys.clear();
        flowSplitStarts.clear();
        loopDepth = 0;
        terminatorLabel = FluxInvalidLabel;
        flowTerminated = false;
    }
}