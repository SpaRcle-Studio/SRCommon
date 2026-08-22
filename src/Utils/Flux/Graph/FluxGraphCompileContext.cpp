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
        loopDepth = 0;
        terminatorLabel = FluxInvalidLabel;
        flowTerminated = false;
    }
}