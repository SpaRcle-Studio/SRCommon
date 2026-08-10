//
// Created by Monika on 09.08.2026.
//

#include <Utils/Flux/Runtime/FluxExecution.h>

namespace SR_FLUX_NS {
    FluxExecution::FluxExecution(FluxExecution&& other) noexcept
        : registers(std::move(other.registers))
        , valueStack(std::move(other.valueStack))
        , callStack(std::move(other.callStack))
        , instructionPointer(other.instructionPointer)
        , state(other.state)
    {
        other.instructionPointer = 0;
    }

    FluxExecution& FluxExecution::operator=(FluxExecution&& other) noexcept {
        if (this != &other) {
            registers = std::move(other.registers);
            valueStack = std::move(other.valueStack);
            callStack = std::move(other.callStack);

            instructionPointer = other.instructionPointer;
            other.instructionPointer = 0;

            state = other.state;
            other.state = FluxExecutionState::None;
        }
        return *this;
    }

    bool FluxExecution::IsFinished() const {
        return state == FluxExecutionState::Finished || state == FluxExecutionState::Error;
    }

    bool FluxExecution::CanBeExecuted() const {
        return !IsFinished();
    }
}