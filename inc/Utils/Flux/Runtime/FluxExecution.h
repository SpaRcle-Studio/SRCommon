//
// Created by Monika on 09.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_EXECUTION_H
#define SR_ENGINE_COMMON_FLUX_EXECUTION_H

#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Reflection/Value.h>

namespace SR_FLUX_NS {
    enum class FluxExecutionState : uint8_t {
        None,
        Running,
        Paused,
        Finished,
        Error
    };

    struct FluxExecution {
        FluxExecution() = default;
        FluxExecution(FluxExecution&& other) noexcept;
        FluxExecution(const FluxExecution& other) = delete;
        FluxExecution& operator=(FluxExecution&& other) noexcept;
        FluxExecution& operator=(const FluxExecution& other) = delete;

        Vector<Reflection::Value> registers;
        Vector<Reflection::Value> valueStack;
        Vector<FluxInstructionId> callStack;
        FluxInstructionId instructionPointer = 0;
        FluxExecutionState state = FluxExecutionState::None;
        StringView emittedLabel;

        SR_NODISCARD bool IsFinished() const;
        SR_NODISCARD bool CanBeExecuted() const;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_EXECUTION_H
