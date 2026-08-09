//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/Runtime/FluxRuntime.h>

namespace SR_FLUX_NS {
    FluxRuntime::FluxRuntime(FluxProgram* pProgram)
        : m_program(pProgram)
    { }

    void FluxRuntime::Emit(StringView functionName, const Vector<Reflection::Value>& args) {

    }

    void FluxRuntime::Update(float_t dt) {
        m_timeAccumulator += dt;

        const float_t floatingTicks = m_timeAccumulator / m_tickDuration;
        const auto ticks = static_cast<uint32_t>(floatingTicks);
        int64_t budget = m_budgetPerTick * ticks;
        const auto budgetPerExecution = static_cast<float_t>(m_executions.size()) / static_cast<float_t>(budget);
        if (budgetPerExecution < 1.f) {
            return; /// accumulating
        }

        m_timeAccumulator -= static_cast<float_t>(ticks) * m_tickDuration;

        while (budget > 0) {
            budget--;
        }
    }
}
