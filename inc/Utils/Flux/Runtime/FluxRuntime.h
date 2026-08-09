//
// Created by Monika on 08.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_RUNTIME_H
#define SR_ENGINE_COMMON_FLUX_RUNTIME_H

#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Reflection/Value.h>

namespace SR_FLUX_NS {
    struct FluxExecution {
        Vector<Reflection::Value> registers;
        Vector<Reflection::Value> stack;
        uint32_t instructionPointer = 0;
    };

    class FluxRuntime : public NonCopyable {
    public:
        FluxRuntime() = default;
        explicit FluxRuntime(FluxProgram* pProgram);

        void Emit(StringView functionName, const Vector<Reflection::Value>& args);
        void Update(float_t dt);

    public:
        uint32_t m_budgetPerTick = 1024;
        float_t m_tickDuration = 0.016f; // 60 FPS

        float_t m_timeAccumulator = 0.0f;

        Vector<FluxExecution> m_executions;

        FluxProgram* m_program = nullptr;

        Vector<Reflection::Value> constants;
        Vector<Reflection::Value> storage;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_RUNTIME_H
